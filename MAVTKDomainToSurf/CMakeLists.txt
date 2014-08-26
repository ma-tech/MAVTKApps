# define the project
PROJECT(MAVTKDomainToSurf)

cmake_minimum_required(VERSION 2.8)

# OpenMP
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Woolz
SET(WOOLZ_DIR / CACHE PATH "Base 	Woolz install path.")
include_directories(BEFORE ${WOOLZ_DIR}/include)
link_directories(${WOOLZ_DIR}/lib)

# VTK
FIND_PACKAGE(VTK 6.0 REQUIRED NO_MODULE)
IF(VTK_FOUND)
	INCLUDE(${VTK_USE_FILE})

ELSE(VTK_FOUND)
	MESSAGE(FATAL_ERROR
		"Cannot build the executable without VTK. Please set the
		VTK_DIR")
ENDIF(VTK_FOUND)

# Source file
set(SRCS MAVTKDomainToSurf.cpp)

# Executable
ADD_EXECUTABLE(MAVTKDomainToSurf ${SRCS})
TARGET_LINK_LIBRARIES(MAVTKDomainToSurf ${VTK_LIBRARIES} Wlz Alg Alc)

