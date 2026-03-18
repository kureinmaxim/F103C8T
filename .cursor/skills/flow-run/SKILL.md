---
name: flow-run
description: Execute the repository task workflow defined in `SPECS/COMMANDS/FLOW.md` from branch creation through review, archive, pull request, and CI follow-up. Use when the user asks to do the next task end-to-end, complete the full workflow, or carry a task from start to finish without pausing between phases.
---

# Flow Run

Execute `SPECS/COMMANDS/FLOW.md` as the source of truth. Do not skip required phases unless the workflow explicitly allows it.

## Quick Start

1. Read `SPECS/COMMANDS/FLOW.md` before doing any work.
2. Identify the task and branch context.
3. Execute phases in order:
   `BRANCH -> SELECT -> PLAN -> EXECUTE -> ARCHIVE -> REVIEW -> FOLLOW-UP -> ARCHIVE-REVIEW -> PR -> CI-REVIEW`
4. Stop only after the pull request exists and CI results have been reviewed.

## Required Inputs

Collect these before starting:
- Task identifier and short description.
- Current branch and whether the correct feature branch already exists.
- Review subject name for `REVIEW_{subject}.md`.

If the task identifier is unknown, determine it during `SELECT` from:
- `SPECS/Workplan.md`
- `SPECS/INPROGRESS/next.md`

## Execution Rules

Apply these rules throughout the run:
- Treat `SPECS/COMMANDS/FLOW.md` as authoritative if any instruction conflicts with this file.
- Execute every phase in sequence.
- Use the `flow-primitive-commit` skill for every required commit checkpoint.
- Stage only task-relevant files for each commit.
- Use present-tense FLOW commit message patterns.
- During `EXECUTE`, run required quality gates from FLOW, including `pytest`, `ruff check src/`, `mypy src/` if configured, and `pytest --cov` with coverage at or above the FLOW threshold.
- Save artifacts in the expected locations under `SPECS/INPROGRESS/` and `SPECS/ARCHIVE/`.
- If `REVIEW` has no actionable findings, skip `FOLLOW-UP` only if FLOW explicitly allows it.
- After `ARCHIVE-REVIEW`, use the `gh-create-pr` skill to open the pull request.
- During `CI-REVIEW`, inspect results with the `gh-pr-results-review` skill, fix failures, push, and repeat until checks pass or the workflow says otherwise.

## Phase Checklist

### 1. BRANCH

- Ensure `main` is current.
- Create `feature/{TASK_ID}-{short-description}` if needed.
- Commit with `flow-primitive-commit`.
- Message pattern: `Branch for {TASK_ID}: {short description}`.

### 2. SELECT

- Pick the next task from `SPECS/Workplan.md`.
- Optionally use `python scripts/pick_next_task.py` if the repo workflow expects it.
- Update `SPECS/INPROGRESS/next.md`.
- Commit with message pattern: `Select task {TASK_ID}: {TASK_NAME}`.

### 3. PLAN

- Create `SPECS/INPROGRESS/{TASK_ID}_{TASK_NAME}.md`.
- Capture deliverables, acceptance criteria, and dependencies.
- Commit with message pattern: `Plan task {TASK_ID}: {TASK_NAME}`.

### 4. EXECUTE

- Implement according to the task PRD.
- Run required quality gates from FLOW.
- Create `SPECS/INPROGRESS/{TASK_ID}_Validation_Report.md`.
- Commit with message pattern: `Implement {TASK_ID}: {brief description of changes}`.
- For large tasks, make additional logical checkpoint commits with `flow-primitive-commit`.

### 5. ARCHIVE

- Run the `SPECS/COMMANDS/ARCHIVE.md` workflow.
- Confirm the task archive exists under `SPECS/ARCHIVE/{TASK_ID}_{TASK_NAME}/`.
- Verify `SPECS/INPROGRESS/next.md` and `SPECS/Workplan.md` are updated.
- Commit with message pattern: `Archive task {TASK_ID}: {TASK_NAME} ({VERDICT})`.

### 6. REVIEW

- Run `SPECS/COMMANDS/REVIEW.md`.
- Save the review report as `SPECS/INPROGRESS/REVIEW_{subject}.md`.
- Commit with message pattern: `Review {TASK_ID}: {short subject}`.

### 7. FOLLOW-UP

- If the review contains actionable findings, run `SPECS/COMMANDS/PRIMITIVES/FOLLOW_UP.md`.
- Add any follow-up work to `SPECS/Workplan.md`.
- Commit with message pattern: `Follow-up {TASK_ID}: {short subject}`.
- If there are no actionable findings, record that `FOLLOW-UP` was skipped.

### 8. ARCHIVE-REVIEW

- Move `REVIEW_{subject}.md` to `SPECS/ARCHIVE/_Historical/` or the relevant task archive.
- Update `SPECS/ARCHIVE/INDEX.md`.
- Commit with message pattern: `Archive REVIEW_{subject} report`.

### 9. PR

- Use the `gh-create-pr` skill.
- Open a pull request from the feature branch into `main`.
- Title format: `{TASK_ID}: {TASK_NAME}`.
- Body should summarize changes, list quality gate results, and reference the validation report.

### 10. CI-REVIEW

- Wait long enough for CI to start.
- Use the `gh-pr-results-review` skill to inspect results.
- If checks fail, surface the actionable cause, fix it, push a new commit, and repeat `CI-REVIEW`.
- Finish only when CI has been reviewed and failures are resolved or explicitly accepted by the workflow.

## Completion Criteria

Treat the run as complete only when all are true:
- The required FLOW sequence has been completed, with optional `FOLLOW-UP` skipped only when permitted.
- Required artifacts exist in `SPECS/INPROGRESS/` and `SPECS/ARCHIVE/`.
- Required quality gates were run and their outcomes were captured.
- Required commit checkpoints were created with `flow-primitive-commit`.
- A pull request was created with `gh-create-pr`.
- CI results were reviewed with `gh-pr-results-review`.

## Trigger Phrases

Apply this skill when requests look like:
- "Do the next task from start to finish."
- "Run the full FLOW workflow."
- "Take the next task end-to-end."
- "Complete the whole task without pausing between phases."
- "Strictly follow `SPECS/COMMANDS/FLOW.md` for the next task."
