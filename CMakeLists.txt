cmake_minimum_required(VERSION 3.20)
project(weftfit_retarget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- ports layer -----------------------------------------------------------
# The C-ABI mesh payload (ports/mesh.h) shared by the core and every adapter.
# Builds standalone with no dependencies — the boundary the cluster agrees on.
add_library(weftfit_ports STATIC core/mesh.cpp)
target_include_directories(weftfit_ports PUBLIC ports)

# --- core layer ------------------------------------------------------------
# The garment-retargeting solve uses PolyFEM as its computational substrate.
# Behind WEFTFIT_BUILD_CORE (default OFF, so the ports lib builds fast on its own),
# vendor *base PolyFEM*: the cloth-fit fork with the garment domain gated OFF.
# Phase 1 (cloth-fit) made garment optional via POLYFEM_WITH_GARMENT, so OFF yields
# a garment-free `polyfem` target — the core links it as a math engine, and no
# garment code lives in the substrate (the purist cut).
#
# Remaining (tracked in EXTRACTION.md):
#   step 2  extract_core     — add core/garment sources, link polyfem + weftfit_ports
#   step 3  build_core        — compile the core standalone against base PolyFEM
#   step 4  refactor_io       — optimize.cpp inline I/O -> wf_mesh_source/_sink ports
option(WEFTFIT_BUILD_CORE "Build the PolyFEM-backed retarget core (fetches base PolyFEM)" OFF)

if(WEFTFIT_BUILD_CORE)
  include(FetchContent)

  # base PolyFEM = cloth-fit @ the commit where POLYFEM_WITH_GARMENT landed, garment OFF.
  set(POLYFEM_WITH_GARMENT OFF CACHE BOOL "base PolyFEM carries no garment code" FORCE)
  FetchContent_Declare(base_polyfem
    GIT_REPOSITORY https://github.com/V-Sekai-fire/cloth-fit.git
    GIT_TAG        d4ad4283f58700310eaa22aede33c3de06420cdd  # Phase 1: garment gated
  )
  # Provides the garment-free `polyfem` target (its own bin/tests stay off as a
  # subproject via POLYFEM_TOPLEVEL_PROJECT). Heavy: pulls PolyFEM's dep stack.
  FetchContent_MakeAvailable(base_polyfem)

  # core/garment sources + `add_library(weftfit_core ... )` linking `polyfem` +
  # `weftfit_ports` are added in step 2/3.
endif()
