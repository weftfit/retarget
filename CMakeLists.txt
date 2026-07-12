cmake_minimum_required(VERSION 3.20)
project(weftfit_retarget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- ports layer -----------------------------------------------------------
# The C-ABI mesh payload (ports/mesh.h) shared by the core and every adapter.
# Builds standalone with no dependencies — the boundary the cluster agrees on.
add_library(weftfit_ports STATIC core/mesh.cpp)
target_include_directories(weftfit_ports PUBLIC ports)

# --- core layer (TODO) -----------------------------------------------------
# The garment-retargeting solve (core/garment/optimize.cpp) uses PolyFEM as its
# computational substrate. To build the core standalone:
#   1. Replace optimize.cpp's inline read_/write_mesh_with_groups + cfusd_* calls
#      with wf_mesh_source / wf_mesh_sink port calls (de-entangle I/O).
#   2. FetchContent PolyFEM (the V-Sekai fork) and link the garment sources.
#   3. Add core/spec fixtures + tests.
# Until then the authoritative solve lives in V-Sekai-fire/cloth-fit.
option(WEFTFIT_BUILD_CORE "Build the PolyFEM-backed retarget core (WIP)" OFF)
