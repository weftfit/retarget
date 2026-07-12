# weftfit/retarget

The **core** of weftfit: intersection-free garment retargeting. Refits a garment
authored for one body onto a different avatar without the cloth intersecting the
body — a PolyFEM-based optimization that respects skeleton correspondence and
stays collision-free. Canonical frame is Godot's (+Y up, +Z front, right-handed).

Hexagonal component (`core` / `ports` / `adapters`):

- **`core/`** — the domain solve. Given canonical-frame meshes (garment, avatar,
  source/target skeletons, skin weights) + fit config, produces the retargeted
  per-step meshes. No file/format/transport code reaches this layer; mesh I/O is
  obtained through the ports below. `core/spec/` runs the solve against baked
  fixtures in isolation.
- **`ports/`** — the C-ABI contracts adapters implement (lowest common
  denominator across languages):
  - `mesh_source.h` — driving/source port: *read* a mesh (`wf_mesh_source`)
  - `mesh_sink.h` — driven/sink port: *write* a mesh (`wf_mesh_sink`)
  - `mesh.h` — the mesh payload crossing the boundary
  - `sibling-repos.txt` — cluster wiring
- **`adapters/`** — in-repo adapters (e.g. a fixture source/sink for `core/spec`).
  Heavy real-I/O adapters live in sibling repos: **weftfit/stage** (OpenUSD),
  **weftfit/obj**, **weftfit/cli** (driving), **weftfit/viewer**.

One port admits many adapters, so a single solve pass fans out to OBJ + OpenUSD +
viewer sinks at once, and a new output is a new sink adapter with no core change.

## Migration status (from the `cloth-fit` monolith)

The garment domain code (`optimize.{cpp,hpp}`, `coordinate_system.hpp`, the
`garment_forms`) is lifted here from `V-Sekai-fire/cloth-fit`. Remaining to make
the core build standalone:

- [ ] Replace `optimize.cpp`'s inline `read_/write_mesh_with_groups` calls with
      `wf_mesh_source` / `wf_mesh_sink` port calls (de-entangle I/O from the solve)
- [ ] Depend on PolyFEM as the computational substrate (submodule or fetched dep)
      rather than vendoring the whole fork — the core links PolyFEM as a math
      engine, not a transport/I/O framework
- [ ] Provide `wf_mesh` support (mesh.h impl) shared with adapters
- [ ] `core/spec/` fixtures + CI

The authoritative solver still lives in `cloth-fit` until this repo builds green;
this is the structural cut, not yet a drop-in replacement.
