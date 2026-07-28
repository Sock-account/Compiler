# Lexer bug fixes

Changes to `main.c` and `Makefile` fixing the garbage characters that appeared in the
lexer's output.

## The symptom

Running `./Compiler` on `test.txt` (`exit(12);`) printed three lines of junk:

```
TYPE EXIT
EXIT
TOKEN TYPE: KEYWORD
M-8@[7M-k}        <- garbage
 M-rM-CM-^PM-i]   <- garbage
 M-rM-CM-^PM-i]   <- garbage
FOUND OPEN PAREN
TEST TOKEN VALUE: 12
12
TOKEN TYPE: INT
FOUND CLOSE PAREN
FOUND SEMICOLON
```

Two independent bugs caused it, plus a third that made the first one fire every time.

---

## Bug 1 — buffer was never null-terminated

`lexer` allocated exactly `length` bytes, leaving no room for the terminator, then
wrote the terminator two positions past the end of the allocation.

```c
buffer = malloc(sizeof(char) * length);   // indices 0..length-1
fread(buffer, 1, length, file);
buffer[length + 1] = '\0';                // writes at index length+1 -- out of bounds
```

`buffer[length]` was never set, so `while (current[current_index] != '\0')` read past
the end of the heap block. It only appeared to work because the next byte in the heap
happened to be zero.

AddressSanitizer confirmed it:

```
==2213==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 1 at 0x72d3237e001a
    #0 in lexer /root/Compiler/main.c:85
0x72d3237e001a is located 1 bytes after 9-byte region
```

**Fix:** allocate `length + 1`, terminate at `buffer[length]`.

```c
char *buffer = malloc(length + 1);
fread(buffer, 1, length, file);
fclose(file);
buffer[length] = '\0';
```

## Bug 2 — uninitialized token, the actual source of the garbage

`generate_keyword` only assigned `type` and `value` inside the `strcmp` match. Any word
that was not `"exit"` returned a `malloc`'d struct full of uninitialized memory, and
`print_token` then dereferenced that junk pointer.

```c
if (strcmp(keyword, "exit") == 0) {
    token->type = KEYWORD;
    token->value = "EXIT";
}
return token;              // every other word: type and value are garbage
```

**Fix:** assign on every path, with a new `IDENTIFIER` token type as the default.
The token also stores the real lexeme now instead of the string literal `"EXIT"` —
a literal must never be passed to `free()`, which would have become a problem as soon
as cleanup was added.

```c
token.value = /* the scanned word */;
token.type  = IDENTIFIER;
if (strcmp(token.value, "exit") == 0) {
    token.type = KEYWORD;
}
```

## Bug 3 — the lexer never advanced past a token

This is what guaranteed Bug 2 fired. After consuming a token, the main loop only did
`current_index++`, so `exit` was scanned at `e`, then re-scanned as `xit`, `it`, `t` —
three non-matches, three garbage lines.

`generate_number` worked around this with a division loop that counted digits:

```c
int token_value = atoi(test_token.value);
while (token_value >= 10) {
    token_value = token_value / 10;
    current_index++;
}
```

That mis-advances on input like `007` (`atoi` gives `7`, so it skips one digit instead
of three).

**Fix:** both generators take `int *current_index` and advance the caller's index past
the token. The loop `continue`s afterwards so the trailing `current_index++` cannot
skip a character. The division hack is deleted.

```c
}else if (isdigit((unsigned char)current[current_index])) {
    Token token = generate_number(current, &current_index);
    print_token(token);
    free(token.value);
    continue;  // generate_number already advanced current_index
}
```

---

## Other corrections in the same pass

| Issue | Before | After |
|---|---|---|
| `file == NULL` checked after use | check sat *below* the `fseek`/`fread` calls it was meant to guard | moved to the top of `lexer` |
| Leaked allocation | `current = malloc(...)` then immediately `current = buffer` | `char *current = buffer;` |
| Leaked `Token` | both generators `malloc`'d a `Token`; `generate_number` returned `*token`, leaking the original | both return `Token` by value, no `malloc` |
| Fixed 8-byte value buffer | `malloc(sizeof(char) * 8)` with no bounds check — an 8-character identifier overflowed it | buffer sized to the actual token length |
| `buffer` never freed | — | `free(buffer)` at end of `lexer`; `free(token.value)` after each `print_token` |
| `isdigit`/`isalpha` on plain `char` | undefined behaviour for negative values | argument cast to `unsigned char` |
| Debug prints | `TYPE EXIT` and `TEST TOKEN VALUE: %s` | removed; `print_token` reports the type |

`print_token` gained an `IDENTIFIER` branch alongside the existing ones.

## Makefile

Added a sanitized build. It writes to a **separate** binary — pointing it at `Compiler`
would leave a slow instrumented build in place that plain `make` then reports as
"up to date".

```make
BINS := Compiler T_test Compiler_debug

Compiler_debug: main.c
	$(CC) $(CFLAGS) -fsanitize=address,undefined -o $@ $<

debug: Compiler_debug
	./Compiler_debug
```

`make debug` builds and runs with AddressSanitizer + UndefinedBehaviorSanitizer. Roughly
2x slower, and it finds both classes of bug above automatically.

---

## Verification

`main.c` compiles with zero warnings under `-Wall -Wextra`, and runs clean under
ASan/UBSan with no leak reports.

Original input `exit(12);`:

```
exit
TOKEN TYPE: KEYWORD
FOUND OPEN PAREN
12
TOKEN TYPE: INT
FOUND CLOSE PAREN
FOUND SEMICOLON
```

Cases that previously broke — `exit(007); someVeryLongIdentifierName(42);`:

```
exit
TOKEN TYPE: KEYWORD
FOUND OPEN PAREN
007                          <- leading zeros survive (was mis-advanced)
TOKEN TYPE: INT
FOUND CLOSE PAREN
FOUND SEMICOLON
someVeryLongIdentifierName   <- 26 chars (was an 8-byte overflow)
TOKEN TYPE: IDENTIFIER       <- non-keyword word (was garbage)
FOUND OPEN PAREN
42
TOKEN TYPE: INT
FOUND CLOSE PAREN
FOUND SEMICOLON
```

## Known remaining issues

Not addressed, outside the scope of this fix:

- **Unrecognized characters are silently dropped.** Anything that is not a digit,
  letter, `;`, `(`, or `)` falls through every branch and disappears — `1 + 2` lexes as
  two integers with no operator. Needs a final `else` that reports the character.
- **`test.c` has pre-existing warnings.** A `%d` used against a `long int`, and
  `if (!fread)` which tests the address of the function (always true) rather than its
  result. Unrelated to this bug, but they clutter the quickfix list on every `make`.
- **`ftell` returns `long`, assigned to `int`.** Fine for small files, wrong past 2 GB.
- **Return values unchecked** on `malloc` and `fread`.
- **`SEPARATOR` is never produced.** The separator cases print directly instead of
  building a token, so the enum member is unused.

---

# Appendix: changes from earlier Claude Code sessions

Reconstructed from session transcripts in `~/.claude/projects/-root-Compiler/`. Three
earlier sessions edited the source: 8 edits to `main.c`, 3 to `test.c`.

**Only one of those edits is still in the code.** The rest were reverted before this
session began, or superseded by the rewrite above. None of them were ever committed —
`HEAD` (`d525739`, 2026-07-26) predates the 2026-07-28 sessions and still contains the
original hardcoded Windows path.

| When | Session | File | Change | Status |
|---|---|---|---|---|
| 07-14 15:23 | `772bb0e7` | main.c | `main()` took `argc`/`argv`, path from `argv[1]`, NULL check, `fclose`, `return 0` | reverted |
| 07-14 15:24 | `772bb0e7` | main.c | Hardcoded Windows path → `"test.txt"` | reverted |
| 07-28 16:39 | `a1661ece` | main.c | `file == NULL` check added at top of `lexer` | reverted, redone above |
| 07-28 16:39 | `a1661ece` | main.c | Old misplaced NULL check removed | reverted, redone above |
| 07-28 16:39 | `a1661ece` | main.c | Hardcoded Windows path → `"test.txt"` | **in the code** |
| 07-28 16:40 | `a1661ece` | test.c | Hardcoded Windows path → `"test.txt"` | reverted |
| 07-28 16:40 | `a1661ece` | test.c | `printf("Length: %d")` → `%ld` for `long` | reverted |
| 07-28 16:40 | `a1661ece` | test.c | `if (!fread)` → capture `fread` result, compare to length | reverted |
| 07-28 16:52 | `2998ad5f` | main.c | `generate_keyword` gained an `int *consumed` out-param | superseded |
| 07-28 16:52 | `2998ad5f` | main.c | Caller advanced `current_index += consumed - 1` | superseded |

Resume any of these with `claude --resume` and the session ID.

## What the reverted work was

**The `argv` version of `main`** (07-14) let you lex an arbitrary file rather than
always `test.txt`, and closed the file and returned a status code:

```c
int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : "test.txt";
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error: could not open file '%s'\n", path);
        return 1;
    }
    lexer(file);
    fclose(file);
    return 0;
}
```

Current `main` is back to the no-argument form with the filename hardcoded. Worth
redoing — it is how you would test the lexer against more than one input without
editing `test.txt` each time. Note that `lexer` already calls `fclose` internally, so
reinstating the `fclose` in `main` as written above would double-close.

**The `consumed` out-param** (07-28) solved the same advance bug as Bug 3 above, by a
different route — `generate_keyword` reported how many characters it had scanned and
the caller did `current_index += consumed - 1`, the `-1` compensating for the loop's
own increment. The version above instead passes `int *current_index` so the generator
advances the index directly and the caller `continue`s. Same fix; the current one
avoids the off-by-one adjustment and works for numbers too.

**The `test.c` fixes** (07-28) are still worth reapplying — those two warnings are
exactly the ones `make` still prints today, listed under "Known remaining issues".

## 2026-07-28 13:32 — main.c

Investigating a report that the lexer was re-iterating over input it had already consumed. The first step was just getting a build: the working tree had a one-character typo in `generate_number` that made the file fail to compile, so nothing could be reproduced or instrumented until it was corrected.

```diff
--- main.c (before)
+++ main.c (after)
@@ -6,7 +6,6 @@
 typedef enum {
     INT,
     KEYWORD,
-    IDENTIFIER,
     SEPARATOR,
 
 } TokenType;
@@ -27,90 +26,97 @@
     if (token.type == KEYWORD) {
         printf("TOKEN TYPE: KEYWORD\n");
     }
-    if (token.type == IDENTIFIER) {
-        printf("TOKEN TYPE: IDENTIFIER\n");
-    }
     if (token.type == SEPARATOR) {
         printf("TOKEN TYPE: SEPARATOR\n");
     }
 }
 
-// Advances *current_index past the digits and returns them as a new string.
-Token generate_number(char *current, int *current_index) {
-    int start = *current_index;
-    while (isdigit((unsigned char)current[*current_index])) {
-        (*current_index)++;
+Token generate_number(char *current, int current_index) {
+    Token *token = malloc(sizeof(Token));
+    token->type = INT;
+    char *value = malloc(sizeof(char) * 8);
+    int value_index = 0;
+    while (isdigit(current[current_index]) && current[current_index] != '\0') {
+        if (!isdigit(current[current_index])) {
+            break;
+        }
+        value[value_index] = current[current_index];
+        value_index++;
+        current_index++;
     }
-    int length = *current_index - start;
-
-    Token token;
-    token.type = INT;
-    token.value = malloc(length + 1);
-    memcpy(token.value, current + start, length);
-    token.value[length] = '\0';
-    return token;
+    value[value_index] = '\0';
+    //printf("%c", token->value);
+    token->value = value;
+    return *token;
 }
 
-// Advances *current_index past the letters and returns them as a new string.
-Token generate_keyword(char *current, int *current_index) {
-    int start = *current_index;
-    while (isalpha((unsigned char)current[*current_index])) {
-        (*current_index)++;
+Token *generate_keyword(char *current, int current_index) {
+    Token *token = malloc(sizeof(Token));
+    char *keyword = malloc(sizeof(char) * 8);
+    int keyword_index = 0;
+    while (current[current_index] != '\0' && isalpha(current[current_index])) {
+        keyword[keyword_index] = current[current_index];
+        //printf("%c", current[current_index]);
+        keyword_index++;
+        current_index++;
     }
-    int length = *current_index - start;
-
-    Token token;
-    token.value = malloc(length + 1);
-    memcpy(token.value, current + start, length);
-    token.value[length] = '\0';
-
-    token.type = IDENTIFIER;
-    if (strcmp(token.value, "exit") == 0) {
-        token.type = KEYWORD;
+    if (keyword_index > 0) {
+        keyword[keyword_index] = '\0';
+    } else {
+        keyword[0] = '\0';  // Handle empty string case
+    }
+    if (strcmp(keyword, "exit") == 0) {
+        printf("TYPE EXIT\n");
+        token->type = KEYWORD;
+        token->value = "EXIT";
     }
     return token;
 }
 
 void lexer(FILE *file) {
+    int length;
+    char *buffer = 0;
+    fseek(file, 0, SEEK_END);
+    length = ftell(file);
+    fseek(file, 0, SEEK_SET);
+    buffer = malloc(sizeof(char) * length);
+    fread(buffer, 1, length, file);
+    fclose(file);
+    buffer[length + 1] = '\0';
+    char *current = malloc(sizeof (char) * length + 1);
+    current = buffer;
+    int current_index = 0;
+
     if (file == NULL) {
         printf("Error reading file\n");
         return;
     }
-
-    int length;
-    fseek(file, 0, SEEK_END);
-    length = ftell(file);
-    fseek(file, 0, SEEK_SET);
-    char *buffer = malloc(length + 1);
-    fread(buffer, 1, length, file);
-    fclose(file);
-    buffer[length] = '\0';
-
-    char *current = buffer;
-    int current_index = 0;
-
     while (current[current_index] != '\0') {
+        //printf("curr: %c\n", current[current_index]);
         if (current[current_index] == ';') {
             printf("FOUND SEMICOLON\n");
         }else if (current[current_index] == '(') {
             printf("FOUND OPEN PAREN\n");
         }else if (current[current_index] == ')') {
             printf("FOUND CLOSE PAREN\n");
-        }else if (isdigit((unsigned char)current[current_index])) {
-            Token token = generate_number(current, &current_index);
-            print_token(token);
-            free(token.value);
-            continue;  // generate_number already advanced current_index
-        }else if (isalpha((unsigned char)current[current_index])) {
-            Token token = generate_keyword(current, &current_index);
-            print_token(token);
-            free(token.value);
-            continue;  // generate_keyword already advanced current_index
+        }else if (isdigit(current[current_index])) {
+        Token test_token = generate_number(current, current_index);
+            printf("TEST TOKEN VALUE: %s\n", test_token.value);
+            int token_value = atoi(test_token.value);
+            while (token_value >= 10) {
+                token_value = token_value / 10;
+                current_index++;
+            }
+            print_token(test_token);
+        }else if (isalpha(current[current_index])) {
+            Token *token_keyword = generate_keyword(current, current_index);
+            //printf("Alpha %c\n", test_keyword->type);
+            //printf("FOUND CHARACTER: %c\n", current[current_index]);
+        print_token(*token_keyword);
         }
         current_index++;
+        //current = fgetc(file);
     }
-
-    free(buffer);
 }
 int main() {
     FILE *file;
@@ -119,3 +125,4 @@
 
 
 }
+
```

## 2026-07-28 13:33 — main.c

`generate_keyword` only populated the token when the scanned text was exactly `exit`; on any other word it returned a heap allocation whose `type` and `value` fields had never been assigned, and `print_token` then dereferenced whatever pointer happened to be in that memory. Populating the token unconditionally also means the caller can ask how many characters were consumed, which the loop fix below depends on.

```diff
--- main.c (before)
+++ main.c (after)
@@ -60,15 +60,11 @@
         keyword_index++;
         current_index++;
     }
-    if (keyword_index > 0) {
-        keyword[keyword_index] = '\0';
-    } else {
-        keyword[0] = '\0';  // Handle empty string case
-    }
+    keyword[keyword_index] = '\0';
+    token->type = KEYWORD;
+    token->value = keyword;
     if (strcmp(keyword, "exit") == 0) {
         printf("TYPE EXIT\n");
-        token->type = KEYWORD;
-        token->value = "EXIT";
     }
     return token;
 }
```

## 2026-07-28 13:33 — main.c

The terminator was written one byte past the end of an allocation that had no room for it, so the byte the scan loop relies on to stop was left as uninitialized heap. AddressSanitizer flagged the write, and the loop's termination was pure luck — it depended on whatever the allocator happened to leave after the file contents. The `NULL` check was also moved ahead of the first `fseek`, since as written it could only run after the pointer had already been used.

```diff
--- main.c (before)
+++ main.c (after)
@@ -72,21 +72,21 @@
 void lexer(FILE *file) {
     int length;
     char *buffer = 0;
-    fseek(file, 0, SEEK_END);
-    length = ftell(file);
-    fseek(file, 0, SEEK_SET);
-    buffer = malloc(sizeof(char) * length);
-    fread(buffer, 1, length, file);
-    fclose(file);
-    buffer[length + 1] = '\0';
-    char *current = malloc(sizeof (char) * length + 1);
-    current = buffer;
-    int current_index = 0;
 
     if (file == NULL) {
         printf("Error reading file\n");
         return;
     }
+    fseek(file, 0, SEEK_END);
+    length = ftell(file);
+    fseek(file, 0, SEEK_SET);
+    buffer = malloc(sizeof(char) * length + 1);
+    length = fread(buffer, 1, length, file);
+    fclose(file);
+    buffer[length] = '\0';
+    char *current = buffer;
+    int current_index = 0;
+
     while (current[current_index] != '\0') {
         //printf("curr: %c\n", current[current_index]);
         if (current[current_index] == ';') {
```

## 2026-07-28 13:33 — main.c

This is the reported symptom. The scan functions take the index by value, so consuming a multi-character token left the caller's position one past where it started — `exit` was lexed, then `x`, `i`, and `t` were each lexed again as separate words. Advancing by the length of the token that was actually produced makes every branch consume what it matched; the digit branch had been approximating this by dividing the parsed integer down to count its digits, which silently miscounted anything with a leading zero.

```diff
--- main.c (before)
+++ main.c (after)
@@ -91,27 +91,25 @@
         //printf("curr: %c\n", current[current_index]);
         if (current[current_index] == ';') {
             printf("FOUND SEMICOLON\n");
+            current_index++;
         }else if (current[current_index] == '(') {
             printf("FOUND OPEN PAREN\n");
+            current_index++;
         }else if (current[current_index] == ')') {
             printf("FOUND CLOSE PAREN\n");
+            current_index++;
         }else if (isdigit(current[current_index])) {
-        Token test_token = generate_number(current, current_index);
+            Token test_token = generate_number(current, current_index);
             printf("TEST TOKEN VALUE: %s\n", test_token.value);
-            int token_value = atoi(test_token.value);
-            while (token_value >= 10) {
-                token_value = token_value / 10;
-                current_index++;
-            }
             print_token(test_token);
+            current_index += strlen(test_token.value);
         }else if (isalpha(current[current_index])) {
             Token *token_keyword = generate_keyword(current, current_index);
-            //printf("Alpha %c\n", test_keyword->type);
-            //printf("FOUND CHARACTER: %c\n", current[current_index]);
-        print_token(*token_keyword);
+            print_token(*token_keyword);
+            current_index += strlen(token_keyword->value);
+        }else {
+            current_index++;
         }
-        current_index++;
-        //current = fgetc(file);
     }
 }
 int main() {
```
