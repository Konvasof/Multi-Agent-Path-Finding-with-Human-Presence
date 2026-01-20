cmake_minimum_required(VERSION 3.17)
project(MyMapfSolver)

set(CMAKE_CXX_STANDARD 17)

# 1. SFML 3.0
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

# 2. OpenMP (Manuální nastavení pro Apple Silicon)
if(APPLE)
    set(OPENMP_ROOT "/opt/homebrew/opt/libomp")
    include_directories("${OPENMP_ROOT}/include")
    link_directories("${OPENMP_ROOT}/lib")
    
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_ROOT}/include")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OPENMP_ROOT}/include")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "${OPENMP_ROOT}/lib/libomp.dylib")
endif()

find_package(OpenMP REQUIRED)

# 3. Hlavičky
include_directories(inc)

# 4. Seznam souborů
add_executable(MyMapfSolver
    # Tvoje soubory
    src/Main.cpp
    src/Grid.cpp
    src/DataLoader.cpp
    src/PathFinder.cpp
    src/MapModifier.cpp
    src/LNS.cpp
    src/Replanner.cpp
    
    # Vizualizér a jeho závislosti (PŘIDÁNO Solver a SIPP)
    src/Visualizer.cpp
    src/Instance.cpp
    src/SharedData.cpp
    src/Computation.cpp
    src/IterInfo.cpp
    src/Map.cpp
    src/utils.cpp
    src/Solver.cpp    
    src/SIPP.cpp      
    
    # ImGui
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_widgets.cpp
    src/imgui_tables.cpp
    src/imgui_demo.cpp
    src/imgui-SFML.cpp
)

# 5. Linkování
if(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(MyMapfSolver 
        SFML::Graphics 
        SFML::Window 
        SFML::System 
        OpenGL::GL
        "${OPENMP_ROOT}/lib/libomp.dylib"
    )
else()
    target_link_libraries(MyMapfSolver 
        SFML::Graphics 
        SFML::Window 
        SFML::System 
        GL 
        OpenMP::OpenMP_CXX
    )
endif()
