# How AI Indexing Works With Small 10-Row Datasets

This note explains exactly how your project identifies each AI system and calculates scores, even when datasets are small.

## 1) What is the primary AI index key?

The primary key is:

- `ai_system_id`

This key appears in both:

- `ai_systems.csv` (master list of AI systems)
- `ai_behavior.csv` (behavior events)

The project does not use row number as identity. It uses `ai_system_id` value matching.

## 2) Which file decides whether an AI is "available"?

`ai_systems.csv` decides availability.

At runtime:

1. The program first loads all systems from `ai_systems.csv` into memory.
2. It loops reports only over these loaded systems.
3. For each loaded system, it searches behavior records where `behavior.ai_system_id == system.ai_system_id`.

So, if an ID is not in `ai_systems.csv`, that AI will not appear in final tables, even if behavior rows exist for that ID.

## 3) What happens with only 10 behavior rows?

That is valid.

The engine does not require large volume. It computes per-AI metrics from whatever matching rows exist.

- Fewer rows means less statistical depth.
- But indexing and scoring still work correctly.

## 4) Matching logic in simple words

For each AI in `ai_systems.csv`:

1. Find all behavior rows with same `ai_system_id`.
2. Attach behavior to relevant ethical principles.
3. Compute principle alignment, cultural compatibility, and risk.
4. Build final index from weighted formula.

## 5) Edge cases (important)

### Case A: AI exists in systems, but no behavior rows

Result:

- Ethical alignment becomes `0.0`
- Cultural compatibility becomes `0.0`
- Risk defaults to Low categories (scores 0)
- Final score still gets base contribution from low-risk penalty mapping

Current overall formula:

- `overall = 0.45*ethical + 0.35*cultural + 0.20*(1-risk_penalty)`
- Low risk penalty is `0.10`

So if ethical and cultural are zero:

- `overall = 0 + 0 + 0.20*(0.90) = 0.18` (18%)

### Case B: Behavior rows exist for AI ID not present in systems

Result:

- Those behavior rows are loaded internally
- But they are not shown in output tables, because reporting loops over loaded systems only

So practically, orphan behavior IDs are ignored in final reporting.

### Case C: Duplicate AI IDs in systems file

Result:

- Program treats both rows as separate entries in array
- This can duplicate output lines for same ID

Recommendation: keep `ai_system_id` unique in `ai_systems.csv`.

## 6) Why this design is okay for your sponsor demo

- Clear master list (`ai_systems.csv`) controls what is evaluated
- Behavior file provides evidence events
- IDs link evidence to each AI system
- Small datasets are acceptable for demonstration and pipeline validation

## 7) Quick checklist before demo

1. Every AI in `ai_systems.csv` has at least one behavior row in `ai_behavior.csv`.
2. `ai_system_id` is unique in `ai_systems.csv`.
3. No typo mismatch in IDs between the two CSVs.
4. Behavior CSV follows 6-column format.

## 8) One-line answer to your question

Yes, AI indexing is available and working: the system uses `ai_system_id` join logic, not row count, so 10-line datasets still work as long as IDs are consistent.
