---
description: "Scan a Kōtuku module or sub-project for bugs using parallel agents and compile a prioritised report"
argument-hint: "<sub-project-name>"
allowed-tools: ["Agent", "Bash", "Glob", "Grep", "Read", "TaskCreate", "TaskUpdate"]
---

# Bug Sweeper

Scan the **$ARGUMENTS** module or sub-project for bugs using parallel specialised agents, then compile a prioritised report.

## Step 1 — Verify the module exists

Confirm the targeted project in $ARGUMENTS exists. If it does not, tell the user and stop. List the source files briefly so you know the scope.

## Step 2 — Launch parallel scanning agents

Launch all five agents **simultaneously in a single message** (run_in_background: true for each). Each agent has a budget of **25 minutes** — instruct them to stop and summarise whatever they have found if they are approaching that limit.

Give every agent this shared context in its prompt:
- Project path, e.g. `src/$ARGUMENTS/`
- Project rules: no `static_cast` (use C-style casts), no `&&`/`||` (use `and`/`or`), no `==` (use `IS` macro), no C++ exceptions, 3-space indentation, upper camelCase function args, lower snake_case locals, British English, 120-char columns.
- Return findings as a structured list: `[SEVERITY] file:line — description`. Severity levels: CRITICAL, HIGH, MEDIUM, LOW.
- Do NOT fix anything — report only.

### Agent 1 — Memory & Resource Management
Scan all C++ and Tiri source files in the targeted sub-project for:
- Memory leaks (heap allocations without matching frees, smart pointer misuse)
- Use-after-free and dangling pointer patterns
- Resource handles (file descriptors, sockets, graphics objects) that may not be released on every exit path
- Double-free or redundant release calls

### Agent 2 — Logic & Control Flow
Scan all C++ and Tiri source files in the targeted sub-project for:
- Null pointer dereferences (pointers used before null checks)
- Array and buffer out-of-bounds accesses
- Off-by-one errors in loops and index calculations
- Unreachable code and dead branches
- Incorrect operator precedence or accidental truncation in integer arithmetic
- Potential integer overflow or underflow

### Agent 3 — Thread Safety
Scan all C++ source files in the targeted sub-project for:
- Global variables (prefixed `gl`) accessed without locks or atomic operations
- Functions that read/write shared state without synchronisation
- Race conditions in callbacks or signal handlers
- Incorrect use of mutexes (recursive lock, lock inversion, forgetting to unlock)

### Agent 4 — Error Handling
Scan all C++ and Tiri source files in the targeted sub-project for:
- Return values from functions that return an error code but whose result is ignored at the call site
- Missing error propagation — an error is detected but not returned or logged
- Functions that silently succeed when they should fail
- Missing input validation at module API entry points

## Step 3 — Collect results

Wait for all background agents to complete before proceeding.

## Step 4 — Compile the prioritised report

Aggregate all agent findings and deduplicate overlapping reports for the same location. Produce the final report in this format:

```
# Bug Scan Report — <module> module

## Summary
| Severity | Count |
|----------|-------|
| CRITICAL |   N   |
| HIGH     |   N   |
| MEDIUM   |   N   |
| LOW      |   N   |

---

## CRITICAL Issues
1. `file:line` — Description [Agent: <name>]
   Suggested fix: …

## HIGH Issues
…

## MEDIUM Issues
…

## LOW Issues
…

---

## Coverage Notes
List any subdirectories or file types that were not scanned, or any areas that require manual review beyond static analysis.
```

Order items within each severity group by the file they appear in, for easy navigation. If no issues are found at a severity level, omit that section.
