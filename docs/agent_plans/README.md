# Agent plans

Where the plans for larger efforts live, and the rules that keep them honest.

| File | What it is |
|------|-----------|
| `ROADMAP.md` | The one place for open work. Its `## Active plans` table lists every plan that is in progress |
| `FINDINGS.md` | Durable findings from the consolidation sweeps; not a plan |
| `<effort>.md` or `<effort>/PLAN.md` | One plan per effort, with a status line (below) |
| `archive/<effort>/` | Finished efforts, frozen, indexed in `archive/README.md` |
| `check_plans.py` | The checker; CI runs it on every push and pull request |

## The status line

Every plan carries, within its first ten lines:

```
**Status:** <state> — <YYYY-MM-DD> — <one line on where it stands>
```

| State | Meaning |
|-------|---------|
| `active` | Work is happening or next up. Listed in the roadmap's active-plans table |
| `blocked` | Waiting on a decision or on something outside the plan. Also listed there, say on what |
| `done` | Finished. The status line names the closing commit(s). A done plan does not stay here: move it to `archive/<effort>/`, add a row to `archive/README.md`, and the checker stops complaining |
| `reference` | A companion document (an audit, a survey) that is not itself a plan |

The date is the date of the last status change, not the last edit.

## Why a checker

Two archived plans once described finished work as unstarted for weeks, because nothing
closed them out. The checker makes the cheap thing mandatory: a missing or stale status
line, a done plan that has not been archived, an archive directory without an index row,
or a roadmap table that disagrees with the files all fail CI.

```
python3 docs/agent_plans/check_plans.py
```
