#!/usr/bin/env python3
"""Check that every plan under docs/agent_plans declares its status and that the indexes agree.

Rules (also in docs/agent_plans/README.md):

* Every Markdown file directly under docs/agent_plans or in a non-archive subdirectory, except
  ROADMAP.md, FINDINGS.md and README.md, carries a status line within its first ten lines:

      **Status:** <state> — <YYYY-MM-DD>[ — <note>]

  where <state> is one of active, blocked, done, reference.
* A plan in state "done" must name at least one commit (7 to 40 hex digits) on its status line,
  and must not stay outside archive/: move it to archive/<effort>/ and add a row to
  archive/README.md. "done" outside the archive is therefore an error on purpose.
* Every subdirectory of archive/ has a row in archive/README.md whose first cell names it.
* ROADMAP.md has a "## Active plans" table listing exactly the plans in state active or blocked,
  by their path relative to docs/agent_plans.

Exit code 0 when everything agrees, 1 otherwise. Run from anywhere:

    python3 docs/agent_plans/check_plans.py
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
STATES = ("active", "blocked", "done", "reference")
STATUS_RE = re.compile(r"^\*\*Status:\*\*\s+(\w+)\s+[—-]+\s+(\d{4}-\d{2}-\d{2})(?:\s+[—-]+\s+(.*))?\s*$")
COMMIT_RE = re.compile(r"\b[0-9a-f]{7,40}\b")
SKIP = {"ROADMAP.md", "FINDINGS.md", "README.md"}


def plan_files():
    for path in sorted(ROOT.rglob("*.md")):
        rel = path.relative_to(ROOT)
        if rel.parts[0] == "archive" or path.name in SKIP:
            continue
        yield path


def read_status(path):
    with path.open(encoding="utf-8") as handle:
        for _ in range(10):
            line = handle.readline()
            if not line:
                break
            match = STATUS_RE.match(line.rstrip("\n"))
            if match:
                return match.group(1), match.group(2), match.group(3) or ""
    return None


def main():
    errors = []
    statuses = {}

    for path in plan_files():
        rel = path.relative_to(ROOT).as_posix()
        status = read_status(path)
        if status is None:
            errors.append(f"{rel}: no status line in the first ten lines "
                          f"(expected '**Status:** <state> — <YYYY-MM-DD>')")
            continue
        state, date, note = status
        if state not in STATES:
            errors.append(f"{rel}: state '{state}' is not one of {', '.join(STATES)}")
            continue
        if state == "done":
            if not COMMIT_RE.search(note):
                errors.append(f"{rel}: a done plan names the commit(s) that closed it on its status line")
            errors.append(f"{rel}: done plans do not stay here; move it to archive/<effort>/ "
                          f"and add a row to archive/README.md")
        statuses[rel] = (state, date, note)

    archive_readme = (ROOT / "archive" / "README.md").read_text(encoding="utf-8")
    for entry in sorted(p for p in (ROOT / "archive").iterdir() if p.is_dir()):
        if f"| `{entry.name}/`" not in archive_readme:
            errors.append(f"archive/{entry.name}/: no row in archive/README.md")

    roadmap = (ROOT / "ROADMAP.md").read_text(encoding="utf-8")
    listed = set()
    section = re.search(r"^## Active plans\n(.*?)(?=^## |\Z)", roadmap, re.S | re.M)
    if section is None:
        errors.append("ROADMAP.md: no '## Active plans' section")
    else:
        for match in re.finditer(r"^\|\s*`([^`]+)`", section.group(1), re.M):
            listed.add(match.group(1))
    expected = {rel for rel, (state, _, _) in statuses.items() if state in ("active", "blocked")}
    for rel in sorted(expected - listed):
        errors.append(f"ROADMAP.md: active plan `{rel}` is missing from the '## Active plans' table")
    for rel in sorted(listed - expected):
        errors.append(f"ROADMAP.md: '## Active plans' lists `{rel}`, which is not an active or blocked plan")

    width = max((len(rel) for rel in statuses), default=10)
    for rel, (state, date, note) in sorted(statuses.items()):
        print(f"{rel:<{width}}  {state:<9}  {date}  {note}")
    if errors:
        print()
        for error in errors:
            print("ERROR:", error)
        return 1
    print(f"\n{len(statuses)} plans checked, indexes agree.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
