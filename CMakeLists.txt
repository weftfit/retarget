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

  # Match cloth-fit's `cloth_fit.build_native` PolyFEM option set so the fetched
  # substrate builds everywhere: Eigen fallback solvers (no MKL/external), OpenVDB
  # serialization off, static TBB. NOTE: the pinned OpenVDB still needs a static
  # oneTBB 2021.9.0 — provide it with -DTBB_ROOT=<install> -DCMAKE_PREFIX_PATH=...
  # (cloth-fit builds it from source in build_native's build_tbb step).
  set(OPENVDB_USE_DELAYED_LOADING OFF CACHE BOOL "" FORCE)
  set(USE_BLOSC     OFF CACHE BOOL "" FORCE)
  set(USE_ZLIB      OFF CACHE BOOL "" FORCE)
  set(USE_IMATH_HALF OFF CACHE BOOL "" FORCE)
  set(TBB_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
  foreach(_solver MKL CHOLMOD UMFPACK SUPERLU HYPRE AMGCL SPECTRA)
    set(POLYSOLVE_WITH_${_solver} OFF CACHE BOOL "" FORCE)
  endforeach()

  FetchContent_Declare(base_polyfem
    GIT_REPOSITORY https://github.com/V-Sekai-fire/cloth-fit.git
    GIT_TAG        d4ad4283f58700310eaa22aede33c3de06420cdd  # Phase 1: garment gated
  )
  # Provides the garment-free `polyfem` target (its own bin/tests stay off as a
  # subproject via POLYFEM_TOPLEVEL_PROJECT). Heavy: pulls PolyFEM's dep stack.
  FetchContent_MakeAvailable(base_polyfem)

  # --- weftfit_core: the garment domain over garment-free base PolyFEM ---------
  # These are exactly the sources cloth-fit gates OUT of libpolyfem when
  # POLYFEM_WITH_GARMENT=OFF. Building them here, linked against that garment-free
  # `polyfem`, is the purist cut compiling: the garment core over a substrate that
  # carries no garment code. The sources reference <polyfem/...> throughout, which
  # resolve via polyfem's PUBLIC include; USD stays off so the inline I/O uses OBJ
  # (the port refactor in step 4 removes it entirely).
  set(_pf "${base_polyfem_SOURCE_DIR}/src/polyfem")
  add_library(weftfit_core STATIC
    "${_pf}/garment/optimize.cpp"
    "${_pf}/garment/GarmentNLProblem.cpp"
    "${_pf}/solver/forms/garment_forms/GarmentForm.cpp"
    "${_pf}/solver/forms/garment_forms/GarmentALForm.cpp"
    "${_pf}/solver/forms/garment_forms/CurveConstraintForm.cpp"
    "${_pf}/solver/forms/garment_forms/CurveCenterTargetForm.cpp"
    "${_pf}/solver/forms/garment_forms/FitForm.cpp"
  )
  target_link_libraries(weftfit_core PUBLIC polyfem weftfit_ports)
  target_compile_features(weftfit_core PUBLIC cxx_std_17)
endif()
