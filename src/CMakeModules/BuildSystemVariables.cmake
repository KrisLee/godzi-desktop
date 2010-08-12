# Define system specific parameters:
# BUILD_SYSTEM_NAME {win|solaris|linux}
# BUILD_SYSTEM_ARCH {x86|amd64|sparc}
# BUILD_TARGET_ARCH {x86|amd64|sparc}
# BUILD_SYSTEM_OS {nt|solaris|linux}
# BUILD_COMPILER {vc-8.0|vc-9.0|gcc-4.1.2|gcc-3.4.6}
# BUILD_COMPILER_NAME {vc|${CMAKE_C_COMPILER}}
# BUILD_COMPILER_VERSION 
# BUILD_COMPILER_MAJOR_VERSION
# BUILD_COMPILER_MINOR_VERSION
# BUILD_SYSTEM_LIB_SUFFIX
# BUILD_TYPE {32|64}
# BUILD_HWOS {x86-nt|sparc-solaris|x86-linux|amd64-nt|amd64-linux}
# BUILD_PLATFORM {win32|win64|linux32|linux64|solaris32}

# CMAKE_SYSTEM_PROCESSOR is set to the PROCESSOR_ARCHITECTURE value (eg x86 or AMD64) for Windows and uname -p result or UNIX/Linux
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} BUILD_SYSTEM_ARCH)
if(BUILD_SYSTEM_ARCH STREQUAL "x86_64")
    set(BUILD_SYSTEM_ARCH "amd64")
elseif(BUILD_SYSTEM_ARCH MATCHES "i?86")
    set(BUILD_SYSTEM_ARCH "x86")
elseif(BUILD_SYSTEM_ARCH STREQUAL "sparc")
    set(BUILD_SYSTEM_ARCH "sparc")
endif(BUILD_SYSTEM_ARCH STREQUAL "x86_64")


# Assign 32 or 64
if(WIN32)
  if(CMAKE_CL_64)
    set(BUILD_TYPE "64")
    set(BUILD_SYSTEM_ARCH "amd64")
    set(COMPILE_FLAG_BUILD_IN_64_BIT_MODE ON)
  else(CMAKE_CL_64)
    set(BUILD_TYPE "32")
    set(BUILD_SYSTEM_ARCH "x86")
    set(COMPILE_FLAG_BUILD_IN_64_BIT_MODE OFF)
  endif(CMAKE_CL_64)
elseif(UNIX)
  if(BUILD_SYSTEM_ARCH STREQUAL "amd64")
    set(BUILD_TYPE "64")
    if (COMPILE_32_ON_64)
      set(BUILD_TYPE "32")
    endif(COMPILE_32_ON_64)
  else(BUILD_SYSTEM_ARCH STREQUAL "amd64")
    set(BUILD_TYPE "32")
  endif(BUILD_SYSTEM_ARCH STREQUAL "amd64")
endif(WIN32)


# Get system name
if(WIN32)
    set(BUILD_SYSTEM_NAME win)
    set(BUILD_SYSTEM_OS nt)
elseif(UNIX)
    # CMAKE_SYSTEM_NAME contains value provided by 'uname -s'
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(BUILD_SYSTEM_NAME linux)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        set(BUILD_SYSTEM_NAME solaris)
    endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BUILD_SYSTEM_OS ${BUILD_SYSTEM_NAME})
endif(WIN32)

set(BUILD_PLATFORM "${BUILD_SYSTEM_NAME}${BUILD_TYPE}")
if(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND NOT BUILD_TYPE STREQUAL "64")
    set(BUILD_HWOS "x86-${BUILD_SYSTEM_OS}")
else(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND NOT BUILD_TYPE STREQUAL "64")
    set(BUILD_HWOS "${BUILD_SYSTEM_ARCH}-${BUILD_SYSTEM_OS}")
endif(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND NOT BUILD_TYPE STREQUAL "64")

# Get compiler info
if(MSVC)
    set(BUILD_COMPILER_NAME vc)
    if(MSVC70)
        set(BUILD_COMPILER_VERSION "7.0")
    elseif(MSVC71)
        set(BUILD_COMPILER_VERSION "7.1")
    elseif(MSVC80)
        set(BUILD_COMPILER_VERSION "8.0")
    elseif(MSVC90)
        set(BUILD_COMPILER_VERSION "9.0")
    elseif(MSVC100)
        set(BUILD_COMPILER_VERSION "10.0")
    endif(MSVC70)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    # Intel compiler will use latest gcc build version for third party libraries
    set(BUILD_COMPILER_NAME gcc)
    set(BUILD_COMPILER_VERSION "4.1.2")
else(MSVC)
    # Get compiler name and version (gcc and gcc-compatible compilers)
    exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE BUILD_COMPILER_VERSION)
    string(REGEX REPLACE "([A-Za-z0-9])[ ].*" "\\1" BUILD_COMPILER_NAME ${BUILD_COMPILER_VERSION})
    string(REGEX REPLACE ".*([0-9]\\.[0-9]\\.[0-9]).*" "\\1" BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
endif(MSVC)

# Extract major and minor version numbers
string(REGEX REPLACE "([0-9])\\.[0-9](\\.[0-9])?" "\\1" BUILD_COMPILER_MAJOR_VERSION ${BUILD_COMPILER_VERSION})
string(REGEX REPLACE "[0-9]\\.([0-9])(\\.[0-9])?" "\\1" BUILD_COMPILER_MINOR_VERSION ${BUILD_COMPILER_VERSION})


# Set the suffix for libraries.  Currently 32 bit only expected.  
if(WIN32)
    set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
    set(BUILD_SYSTEM_LIB_SUFFIX "${BUILD_PLATFORM}_${BUILD_COMPILER}")
else(WIN32)
    # Solaris uses 3.4.6
    if(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
        set(BUILD_COMPILER_VERSION "3.4.6")
    elseif(BUILD_COMPILER_MAJOR_VERSION EQUAL 4)
        set(BUILD_COMPILER_VERSION "4.1.2")
    else(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
        message(FATAL_ERROR "Unsupported compiler version ${BUILD_COMPILER_NAME} ${BUILD_COMPILER_VERSION}")
    endif(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
    set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
    set(BUILD_SYSTEM_LIB_SUFFIX "${BUILD_PLATFORM}_${BUILD_COMPILER}")
endif(WIN32)

# Trying library search based on compiler major and minor version...
set(BUILD_SYSTEM_LIB_SUFFIX_MAJOR_MINOR "${BUILD_PLATFORM}_${BUILD_COMPILER_NAME}-${BUILD_COMPILER_MAJOR_VERSION}\\.${BUILD_COMPILER_MINOR_VERSION}.*")

# Trying library search based on compiler major and version...
set(BUILD_SYSTEM_LIB_SUFFIX_MAJOR "${BUILD_PLATFORM}_${BUILD_COMPILER_NAME}-${BUILD_COMPILER_MAJOR_VERSION}.*")

message("Configuration Details:")
message("  BUILD_SYSTEM_NAME\t\t${BUILD_SYSTEM_NAME}")
message("  BUILD_SYSTEM_ARCH\t\t${BUILD_SYSTEM_ARCH}")
message("  BUILD_SYSTEM_OS\t\t${BUILD_SYSTEM_OS}")
message("  BUILD_COMPILER\t\t${BUILD_COMPILER}")
message("  BUILD_COMPILER_NAME\t\t${BUILD_COMPILER_NAME}")
message("  BUILD_COMPILER_VERSION\t${BUILD_COMPILER_VERSION}")
message("  BUILD_SYSTEM_LIB_SUFFIX\t${BUILD_SYSTEM_LIB_SUFFIX}")
message("  BUILD_TYPE\t\t${BUILD_TYPE}")
message("  BUILD_HWOS\t\t${BUILD_HWOS}")
message("  BUILD_PLATFORM\t\t${BUILD_PLATFORM}")
if(COMPILE_32_ON_64)
    message("  COMPILE_32_ON_64\tTRUE")
else(COMPILE_32_ON_64)
    message("  COMPILE_32_ON_64\tFALSE")
endif(COMPILE_32_ON_64)
