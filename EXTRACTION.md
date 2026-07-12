# retarget core extraction plan (from the V-Sekai-fire/cloth-fit PolyFEM fork)

Goal: a **PolyFEM-free garment core** — the garment domain builds against a *base
PolyFEM* that contains zero garment code, talking to the world only through the
`mesh_source` / `mesh_sink` ports.

## Coupling map (verified against the fork)

cloth-fit is a true long-lived PolyFEM fork, drastically *reduced* to "just enough
base PolyFEM to run garment retargeting" + the garment code. The garment domain is
well isolated; base→garment coupling is tiny.

**Garment-domain (moves to `retarget/core`):**
- `src/polyfem/garment/` — `optimize.{cpp,hpp}` (the `GarmentSolver` god-object + free fns), `coordinate_system.hpp`
- `src/polyfem/solver/forms/garment_forms/` — 5 form pairs, all `: public Form` (Similarity, curve/torsion/size/symmetry, CurveTarget, FitForm<4>, Point{Penalty,Lagrangian}Form)
- `src/polyfem/solver/GarmentNLProblem.{cpp,hpp}` — `: public FullNLProblem` (base never calls into it)
- `src/polyfem/main.cpp` — the garment CLI entrypoint (constructs GarmentSolver, the forms, GarmentNLProblem, ALSolver<garment triple>)

**The ONLY hard base→garment dependency — `src/polyfem/solver/ALSolver.{hpp,cpp}`:**
- `ALSolver.hpp:3` includes `GarmentNLProblem.hpp` (unnecessary — the template body is generic on `Problem`)
- `ALSolver.cpp:2` includes `garment_forms/GarmentALForm.hpp`
- `ALSolver.cpp:135,141` — explicit specializations `compute_error` / `update_lagrangian` for `<GarmentNLProblem, PointLagrangianForm, PointPenaltyForm>`
- `ALSolver.cpp:168` — the sole explicit instantiation, on the garment triple
The template *body* (`solve_al`/`solve_reduced`/`set_al_weight`) is fully garment-agnostic.

No other base file (io, mesh, utils, autogen, FullNLProblem, ContactForm, Form) references garment.

## Phase 1 — sever in the fork, keep cloth-fit building (validatable with ON/OFF builds)

1a. **Make `ALSolver` a pure generic template.** Move its method definitions into
   `ALSolver.hpp` (header-only template), and move the garment-specific explicit
   specializations + the explicit instantiation + the garment includes out of
   `ALSolver.{hpp,cpp}` into a new garment TU `garment/GarmentALSolver.cpp`. Result:
   base `ALSolver` has zero garment references.
1b. **Relocate `GarmentNLProblem.{cpp,hpp}`** `solver/` → `garment/`; drop the two
   lines from `solver/CMakeLists.txt:4-5`, add them under `garment/`.
1c. **Gate the garment layer**: `option(POLYFEM_WITH_GARMENT ON)` guarding
   `add_subdirectory(garment)` (CMakeLists src/polyfem:17), `add_subdirectory(garment_forms)`
   (solver/forms:10), and the `_bin` target (top:342, `main.cpp`). ON = cloth-fit
   unchanged; OFF = base PolyFEM with no garment symbols.
   **Validate:** build with garment ON (no regression, full fit still runs) and OFF
   (links clean, no garment symbols in libpolyfem).

## Phase 2 — extract to weftfit/retarget

- Vendor base PolyFEM: `retarget` pulls the fork at `-DPOLYFEM_WITH_GARMENT=OFF` as a
  submodule / FetchContent (it is a bespoke minimal subset, NOT upstream PolyFEM, so
  it must be vendored, not fetched from polyfem/polyfem).
- Build `core/` = garment/ + garment_forms/ + GarmentNLProblem + GarmentALSolver +
  main, linking base PolyFEM + `weftfit_ports` (`wf_mesh`).

## Phase 3 — port refactor (remove I/O from the core)

- Replace `optimize.cpp`'s inline `read_/write_mesh_with_groups` (which call the OBJ
  and `cfusd_*`/USD code) with `wf_mesh_source` / `wf_mesh_sink` port calls. The core
  then links **no** OBJ/USD code — those live in `weftfit/obj` and `weftfit/stage`.
- `core/spec/` runs the solve against baked mesh fixtures through a fixture adapter.

## Validation (end state)

`retarget` builds standalone → a lib/binary that reads garment/avatar/skeletons via a
`mesh_source`, runs the solve, and fans per-step output to `mesh_sink`s — with base
PolyFEM carrying no garment code. Parity-check a FoxGirl fit against the cloth-fit
baseline.

Until Phase 2 lands green, `V-Sekai-fire/cloth-fit` remains the authoritative build.
