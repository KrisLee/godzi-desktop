cmake_minimum_required(VERSION 2.8)
project(GODZI)

# Include base macros required by other macros
include(CMakeModules/BuildSystemVariables.cmake)
include(CMakeModules/HelperFunctions.cmake)
include(CMakeModules/CMakeModulesParseArgs.cmake)
include(CMakeModules/CreateDependencyLists.cmake)

# Include project setup macros
include(CMakeModules/CreateInstallProperties.cmake)
include(CMakeModules/CreateProject.cmake)
include(CMakeModules/CreateLibrary.cmake)
include(CMakeModules/CreateExecutable.cmake)

# Include third party dependency macros
include(CMakeModules/CreateImportedLibrary.cmake)

# Set architecture identifiers
string(TOUPPER ${BUILD_SYSTEM_ARCH} ARCH)
add_definitions(-D${ARCH})

# Set system identifiers
if(WIN32)
    if(BUILD_COMPILER_NAME STREQUAL "vc")
        set(SOCKET_LIBS ws2_32)
        add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE)

        option(ENABLE_STATIC_ANALYSIS "Enable Visual Studio's static analyzer" OFF)
        if(ENABLE_STATIC_ANALYSIS)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /analyze")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
        endif(ENABLE_STATIC_ANALYSIS)

    endif(BUILD_COMPILER_NAME STREQUAL "vc")
else(WIN32)
#    if(BUILD_SYSTEM_ARCH STREQUAL "amd64")
        # Build 32 bit binaries for 64-bit OS
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
#    endif(BUILD_SYSTEM_ARCH STREQUAL "amd64")

    set(RT_LIBS -lrt)
    set(PTHREAD_LIBS -lpthread)
    set(MATH_LIBS -lm)
    add_definitions(-DUNIX)

    if(BUILD_SYSTEM_NAME STREQUAL "linux")
        add_definitions(-DLinux)
    elseif(BUILD_SYSTEM_NAME STREQUAL "solaris")
        add_definitions(-DSolaris)
    endif(BUILD_SYSTEM_NAME STREQUAL "linux")
endif(WIN32)

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a debug suffix, usually d for Windows")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Projects

# Setup third party dependencies

# --- OpenSceneGraph ----------------------

# OSG: allow the user to set OSG_DIR in CMAKE GUI and auto-populate all the OSG libraries:
set(OSG_DIR "" CACHE STRING "OpenSceneGraph root directory")
if(NOT ENV{OSG_DIR} AND NOT ENV{OSGDIR} AND NOT ENV{OSG_ROOT})
  set( ENV{OSG_DIR} ${OSG_DIR} )
endif(NOT ENV{OSG_DIR} AND NOT ENV{OSGDIR} AND NOT ENV{OSG_ROOT})
set(OSG_INCLUDE_DIR ${OSG_DIR}/include)

# import the package:
include( CMakeModules/ImportOSG.cmake )


# --- osgEarth -----------------------------

# allow the user to set OSGEARTH_DIR in CMAKE GUI and auto-populate all OSGEARTH libraries:
set(OSGEARTH_DIR "" CACHE STRING "osgEarth root directory")
include( CMakeModules/ImportOSGEARTH.cmake )

# --- SQLITE3 -----------------------------
set(SQLITE3_DIR "" CACHE STRING "Sqlite3 root directory")
include( CMakeModules/ImportSQLITE3.cmake )


# --- Qt -----------------------------
include( CMakeModules/ImportQT.cmake )



# subprojects =================================================

add_subdirectory(Applications)
add_subdirectory(SDK)

