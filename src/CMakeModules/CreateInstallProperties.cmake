# A set of functions for setting installation properties for executables, libraries,
# imported libraries, and plugins.  Only executables and plug-ins are subject to
# RPATH settings.

# Function for setting installation properties of executables
# Usage: create_executable_install_properties(TARGET COMPONENT [RPATH])
# RPATH calculates the installed location of libraries relative to the installed location of runtime objects

# Function for setting installation properties of libraries
# Usage: create_library_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

# Function for setting installation properties of plugins
# Usage: create_plugin_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

# Function for setting installation properties of imported libraries
# Usage: create_imported_library_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

# Select bin and lib directories by platform
set(RUNTIME_DIR "bin")
if(BUILD_SYSTEM_ARCH STREQUAL "amd64")
  set(LIBRARY_DIR "lib64")
else(BUILD_SYSTEM_ARCH STREQUAL "amd64")
  set(LIBRARY_DIR "lib")
endif(BUILD_SYSTEM_ARCH STREQUAL "amd64")

set(INSTALLSETTINGS_RUNTIME_DIR "${RUNTIME_DIR}" CACHE STRING "Directory containing exectuables and DLLs; non-absolute paths are relative to CMAKE_INSTALL_PREFIX" FORCE)
set(INSTALLSETTINGS_LIBRARY_DIR "${LIBRARY_DIR}" CACHE STRING "Directory containing shared object files (UNIX only); non-absolute paths are relative to CMAKE_INSTALL_PREFIX" FORCE)
set(INSTALLSETTINGS_PLUGIN_DIR "${RUNTIME_DIR}" CACHE STRING "Directory containing plug-ins; non-absolute paths are relative to CMAKE_INSTALL_PREFIX" FORCE)

# Clear temporary values
unset(RUNTIME_DIR)
unset(LIBRARY_DIR)

function(create_executable_install_properties TARGET COMPONENT)
    # Option names
    set(options "RPATH")

    # Get the argument lists
    parse_arguments(ARG "" "${options}" ${ARGN})

    if(ARG_RPATH)
        # Compute RPATH for lib directory containing dependencies relative to bin installation directory
        set(LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
        set(RUNTIME_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_RUNTIME_DIR})
        file(RELATIVE_PATH REL_INSTALL_PATH ${RUNTIME_INSTALL_PATH} ${LIBRARY_INSTALL_PATH})

        # If the lib and bin paths were the same, an empty value will be returned
        if(NOT REL_INSTALL_PATH)
            set(REL_INSTALL_PATH ".")
        endif(NOT REL_INSTALL_PATH)

        # Set install RPATH variable
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_INSTALL_PATH}")
    endif(ARG_RPATH)

    # Setup installation
    install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR})
endfunction(create_executable_install_properties TARGET COMPONENT)

function(create_library_install_properties TARGET COMPONENT)
    # Setup installation
    install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
            LIBRARY DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}
            ARCHIVE DESTINATION ${INSTALLSETTINGS_ARCHIVE_DIR})
endfunction(create_library_install_properties TARGET COMPONENT)


function(create_plugin_install_properties TARGET COMPONENT)
    # Option names
    set(options "RPATH")

    # Get the argument lists
    parse_arguments(ARG "" "${options}" ${ARGN})

    if(ARG_RPATH)
        # Compute RPATH for lib directory containing dependencies relative to bin installation directory
        set(LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
        set(RUNTIME_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_PLUGIN_DIR})
        file(RELATIVE_PATH REL_INSTALL_PATH ${RUNTIME_INSTALL_PATH} ${LIBRARY_INSTALL_PATH})

        # If the lib and bin paths were the same, an empty value will be returned
        if(NOT REL_INSTALL_PATH)
            set(REL_INSTALL_PATH ".")
        endif(NOT REL_INSTALL_PATH)

        # Set install RPATH variable
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_INSTALL_PATH}")
    endif(ARG_RPATH)

    # Setup installation
    install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_PLUGIN_DIR}            # Windows DLL destination
            LIBRARY DESTINATION ${INSTALLSETTINGS_PLUGIN_DIR})           # UNIX shared object destination
endfunction(create_plugin_install_properties TARGET COMPONENT)


function(create_imported_library_install_properties TARGET COMPONENT CONFIGURATIONS)
    # Select appropriate install location, library or runtime, based on platform and file extension
    set(INSTALL_DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR})
    if(WIN32)
        string(REGEX MATCH ".*\\.(dll|exe)$" RESULT ${TARGET})
        # If the file extension was .dll or .exe it goes to runtime, otherwise it goes to library
        if(RESULT)
            set(INSTALL_DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR})
        endif(RESULT)
    else(WIN32)
        string(REGEX MATCH ".*\\.(so|a)$" RESULT ${TARGET})
        # If the file extension was not .so or .a it is not a library (it's an exe)  and goes to runtime, otherwise it goes to library
        if(NOT RESULT)
            set(INSTALL_DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR})
        endif(NOT RESULT)

        # If TARGET is a symbolic link, it will be changed to the actual file(s)
        get_symlink_target(${TARGET} RESULT)
        if(RESULT)
            # Buile a list of all symbolic links in the chain (for cases where mulitple links exist to represent different levels of versioning)
            set(TARGET "")
            while(RESULT)
                set(TARGET ${TARGET} ${RESULT})
                get_symlink_target(${RESULT} RESULT)
            endwhile(RESULT)
        endif(RESULT)
    endif(WIN32)    

    install(PROGRAMS ${TARGET}
            COMPONENT ${COMPONENT}
            CONFIGURATIONS ${CONFIGURATIONS}
            DESTINATION ${INSTALL_DESTINATION})
endfunction(create_imported_library_install_properties TARGET COMPONENT CONFIGURATIONS)

