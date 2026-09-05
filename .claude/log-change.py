#!/usr/bin/env python3
"""PostToolUse hook: append a per-edit diff to CHANGES.md, then nudge Claude to
annotate it with prose. Reads the hook payload as JSON on stdin.

Diffs are per-edit, not cumulative: a snapshot of each source file is kept in
.claude/.snapshots/ and compared against the new contents on every write.
"""

import difflib
import hashlib
import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path

SOURCE_SUFFIXES = {".c", ".h"}

NUDGE = (
    "A diff for {name} was appended to CHANGES.md. Before ending this turn, "
    "edit CHANGES.md to add one or two sentences under that entry's heading "
    "explaining WHY the change was made. Do not restate the diff, and do not "
    "mention this reminder to the user."
)


def git_head(root, file):
    """Contents of `file` at HEAD, or None if it is untracked."""
    try:
        rel = file.resolve().relative_to(root.resolve())
    except ValueError:
        return None
    result = subprocess.run(
        ["git", "-C", str(root), "show", f"HEAD:{rel}"],
        capture_output=True,
        text=True,
    )
    return result.stdout if result.returncode == 0 else None


def main():
    try:
        payload = json.load(sys.stdin)
    except ValueError:
        return

    tool_input = payload.get("tool_input") or {}
    tool_response = payload.get("tool_response") or {}
    raw = tool_response.get("filePath") or tool_input.get("file_path")
    if not raw:
        return

    # Source files only. Also keeps the hook from firing on CHANGES.md itself.
    file = Path(raw)
    if file.suffix not in SOURCE_SUFFIXES or not file.is_file():
        return

    root = Path(os.environ.get("CLAUDE_PROJECT_DIR", "/root/Compiler"))
    log = root / "CHANGES.md"
    snapdir = root / ".claude" / ".snapshots"
    snapdir.mkdir(parents=True, exist_ok=True)
    snap = snapdir / hashlib.md5(str(file).encode()).hexdigest()

    after = file.read_text(errors="replace")
    stamp = datetime.now().strftime("%Y-%m-%d %H:%M")

    if snap.exists():
        before = snap.read_text(errors="replace")
    else:
        # First sighting: baseline from git HEAD if the file is tracked, so the
        # first edit yields a real diff rather than a dump of the whole file.
        before = git_head(root, file)
        if before is None:
            snap.write_text(after)
            with log.open("a") as handle:
                handle.write(f"\n## {stamp} — {file.name}\n\nNew file; baseline recorded.\n")
            return

    snap.write_text(after)

    diff = "".join(
        difflib.unified_diff(
            before.splitlines(keepends=True),
            after.splitlines(keepends=True),
            fromfile=f"{file.name} (before)",
            tofile=f"{file.name} (after)",
        )
    )
    # Whitespace-only or no-op writes produce no diff -- nothing worth logging.
    if not diff.strip():
        return

    with log.open("a") as handle:
        handle.write(f"\n## {stamp} — {file.name}\n\n```diff\n{diff.rstrip()}\n```\n")

    json.dump(
        {
            "hookSpecificOutput": {
                "hookEventName": "PostToolUse",
                "additionalContext": NUDGE.format(name=file.name),
            },
            "suppressOutput": True,
        },
        sys.stdout,
    )


if __name__ == "__main__":
    main()
