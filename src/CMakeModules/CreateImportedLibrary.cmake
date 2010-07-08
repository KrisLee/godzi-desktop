# Function for static library/archive setup
# Usage: create_imported_library(LIBRARYNAME HEADERFILE LIBRARYFILE [STATIC|SHARED|UNKNOWN]
#                                [INCLUDE_SEARCH_PATH <search-path> [<search-path> [...]]]
#                                [LIBRARY_SEARCH_PATH <search-path> [<search-path> [...]]]
#                                [LIBDEPENDENCIES <library-name> [<library-name> [...]]]
#                                [INCLUDE_PATH <include-path> [<include-path> [...]]]
#                                [DEFINITIONS <definition> [<definition> [...]]]
#                                [LIBRARY_PATH <library-path> [<library-path> [...]]]
#                                [LIBRARIES <library> [<library> [...]]]
#                                [INSTALLATION_COMPONENT <component-name>]
#                                [DLL_PREFIX <prefix>])
#
# Each imported library created exports include path, definition, library path, and libary variables to be used with projects which depend on it.  
# The variables are named as ${LIBRARYNAME}_LIB_INCLUDE_PATH, ${LIBRARYNAME}_LIB_DEFINITIONS, ${LIBRARYNAME}_LIB_LIBRARY_PATH, and ${LIBRARYNAME}_LIB_LIBRARIES, 
# where ${LIBRARYNAME}_${PROJECTNAME} represents the value specified for the macro's NAME arguments.  Arguments prefixed with the INTERNAL modifier will be
# excluded from the exported variables.  
#
# Only the LIBRARYNAME, HEADERFILE, and LIBRARYFILE parameters are required.  All other parameters are optional.  
# LIBRARYNAME is the name that will be used when creating the libraries variables and reference name for other projects.  
# HEADERFILE is the name of a header associated with the library (jpeg.h, etc) and will be used to determine the include path for the library.  
# Alternatively, HEADERFILE may be set to "" and an include path may be explicitly specified as part of the INCLUDE_PATH argument lise.  LIBRARYFILE
# is the name of the actual library file to be imported without a prefix or extension (libjpeg.so is specified as jpeg).  
#
# The STATIC, SHARED, and UNKOWN options indicate the library type.  Only one option may be specified for a library.  If neither option is
# specified, the default is UNKNOWN.  
#
# INCLUDE_SEARCH_PATH and LIBRARY_SEARCH_PATH define the directories to be searched when looking for the header files and library file associated
# with the imported library's profile.  
#
# The LIBDEPENDENCIES parameter specifies a list of names of other library projects, either created with the create_library macro or imported
# with the create_imported_library macro, on which the newly imported library depends.  The names are the values specified for the PROJECTNAME parameter when the 
# dependecies were created.  Include paths, definitions, library paths, and libraries associated with each dependency
# will automatically be added for the current project.  Values from projects not marked INTERNAL will also be exported 
# as part of the include paths, definitions, library paths, and libraries variables defined for the new project. 
#
# INCLUDEPATHS, DEFINITIONS, LIBRARYPATHS, and LIBRARIES define the dependencies for the imported library which are not included by any
# of the libraries specified with the LIBDEPENDENCIES parameter.  This could include system or external third-party dependencies.  
# Values not marked INTERNAL will be added to the exported variables defined for the new library.  

if(NOT DEFINED LIBRARY_LIST)
    message("Defining library list")
    set(LIBRARY_LIST "" CACHE INTERNAL "Library List" FORCE)
endif(NOT DEFINED LIBRARY_LIST)


function(create_imported_library LIBRARYNAME HEADERFILE LIBRARYFILE)

    # Make dependency list prefix
    set(LISTPREFIX ${LIBRARYNAME}_LIB)

    # Argument list names
    set(arglists "INCLUDE_SEARCH_PATH;LIBRARY_SEARCH_PATH;LIBDEPENDENCIES;INCLUDE_PATH;DEFINITIONS;LIBRARY_PATH;LIBRARIES;INSTALLATION_COMPONENT;DLL_PREFIX")
    
    # Option names
    set(options "STATIC;SHARED;UNKNOWN")

    # Get the argument lists
    parse_arguments(ARG "${arglists}" "${options}" ${ARGN})
    
    # Now find the include path
    if(NOT HEADERFILE STREQUAL "")
        find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${HEADERFILE} ${ARG_INCLUDE_SEARCH_PATH} NO_DEFAULT_PATH)
        list(APPEND ARG_INCLUDE_PATH ${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH})
    endif(NOT HEADERFILE STREQUAL "")

    
    # Create the dependency lists.  Instead of just specifying ${ARGN}, separate lists so the ordering of list specification does not matter (basically extracting the SUBDIRECTORIES list)
    create_dependency_lists(${LISTPREFIX}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            DEPENDENCIES ${ARG_DEPENDENCIES}
                            LIBDEPENDENCIES ${ARG_LIBDEPENDENCIES}
                            INCLUDE_PATH ${ARG_INCLUDE_PATH}
                            DEFINITIONS ${ARG_DEFINITIONS}
                            LIBRARY_PATH ${ARG_LIBRARY_PATH}
                            LIBRARIES ${ARG_LIBRARIES})


    # Export the libraries's global (cached) variables
    set(${LISTPREFIX}_LIBDEPENDENCIES ${${LISTPREFIX}_EXTERNAL_LIBDEPENDENCIES} CACHE INTERNAL "${LISTPREFIX} project link dependencies on other project's libraries")
    set(${LISTPREFIX}_INCLUDE_PATH ${${LISTPREFIX}_EXTERNAL_INCLUDE_PATH} CACHE INTERNAL "${LISTPREFIX} project include path")
    set(${LISTPREFIX}_DEFINITIONS ${${LISTPREFIX}_EXTERNAL_DEFINITIONS} CACHE INTERNAL "${LISTPREFIX} project pre-processor definitions")
    set(${LISTPREFIX}_LIBRARY_PATH ${${LISTPREFIX}_EXTERNAL_LIBRARY_PATH} CACHE INTERNAL "${LISTPREFIX} project library path")
    set(${LISTPREFIX}_LIBRARIES ${${LISTPREFIX}_EXTERNAL_LIBRARIES} CACHE INTERNAL "${LISTPREFIX} project required libraries")

    # Determine library type as static or dynamic
    if(ARG_SHARED)
        set(type SHARED)
    elseif(ARG_STATIC)
        set(type STATIC)
    else(ARG_SHARED)
        set(type UNKNOWN)
    endif(ARG_SHARED)

    # Add the library target
    add_library(${LIBRARYNAME} ${type} IMPORTED)

    # Look for debug and optimized libraries
    find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES ${LIBRARYFILE} PATHS ${ARG_LIBRARY_SEARCH_PATH} NO_DEFAULT_PATH)
    find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES ${LIBRARYFILE}${CMAKE_DEBUG_POSTFIX} PATHS ${ARG_LIBRARY_SEARCH_PATH} NO_DEFAULT_PATH)

    # Add the libraries to the global library list    
    list(APPEND LIBRARY_LIST "${LIBRARYNAME}_LIBRARY_RELEASE_NAME")
    list(APPEND LIBRARY_LIST "${LIBRARYNAME}_LIBRARY_DEBUG_NAME")
    set(LIBRARY_LIST ${LIBRARY_LIST} PARENT_SCOPE)

    # Setup the library locations - only specify a debug library if one is found
    if(type STREQUAL "SHARED")
        # Setup DLL linking  (IMPORTED_LOCATION and IMPORTED_IMPLIB; requires DLL and LIB files)
        string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" ${LIBRARYNAME}_DLL_RELEASE_NAME ${${LIBRARYNAME}_LIBRARY_RELEASE_NAME})
        # Add the DLL prefix
        if(WIN32 AND ${LIBRARYNAME}_DLL_RELEASE_NAME AND ARG_DLL_PREFIX)
            # Ensure that the file separators are UNIX style
            file(TO_CMAKE_PATH ${${LIBRARYNAME}_DLL_RELEASE_NAME} BASE_DLL_RELEASE_NAME)
            # Get the file part
            string(REGEX MATCH ".*/" DLLPATH ${BASE_DLL_RELEASE_NAME})
            string(REGEX MATCH "[^/]+\\.dll" DLLNAME ${BASE_DLL_RELEASE_NAME})
            set(${LIBRARYNAME}_DLL_RELEASE_NAME "${DLLPATH}${ARG_DLL_PREFIX}${DLLNAME}")
        endif(WIN32 AND ${LIBRARYNAME}_DLL_RELEASE_NAME AND ARG_DLL_PREFIX)
        set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION ${${LIBRARYNAME}_DLL_RELEASE_NAME} IMPORTED_IMPLIB ${${LIBRARYNAME}_LIBRARY_RELEASE_NAME})
        
        # Setup the debug library
        if(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
            string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" ${LIBRARYNAME}_DLL_DEBUG_NAME ${${LIBRARYNAME}_LIBRARY_DEBUG_NAME})
            # Add the DLL prefix
            if(WIN32 AND ${LIBRARYNAME}_DLL_DEBUG_NAME AND ARG_DLL_PREFIX)
                # Ensure that the file separators are UNIX style
                file(TO_CMAKE_PATH ${${LIBRARYNAME}_DLL_DEBUG_NAME} BASE_DLL_DEBUG_NAME)
                # Get the file part
                string(REGEX MATCH ".*/" DLLPATH ${BASE_DLL_DEBUG_NAME})
                string(REGEX MATCH "[^/]+\\.dll" DLLNAME ${BASE_DLL_DEBUG_NAME})
                set(${LIBRARYNAME}_DLL_DEBUG_NAME "${DLLPATH}${ARG_DLL_PREFIX}${DLLNAME}")
            endif(WIN32 AND ${LIBRARYNAME}_DLL_DEBUG_NAME AND ARG_DLL_PREFIX)
            set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION_DEBUG ${${LIBRARYNAME}_DLL_DEBUG_NAME} IMPORTED_IMPLIB_DEBUG ${${LIBRARYNAME}_LIBRARY_DEBUG_NAME})
        endif(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
        
    else(type STREQUAL "SHARED")
        # Setup static lib linking (IMPORTED_LOCATION only)
        set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION ${${LIBRARYNAME}_LIBRARY_RELEASE_NAME})
        
        # Setup the debug library
        if(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
            set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION_DEBUG ${${LIBRARYNAME}_LIBRARY_DEBUG_NAME})
        endif(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
    endif(type STREQUAL "SHARED")

    # If an installation component was specified, create installation settings
    if(ARG_INSTALLATION_COMPONENT)
        if(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
          create_imported_library_install_properties(${${LIBRARYNAME}_DLL_RELEASE_NAME} ${ARG_INSTALLATION_COMPONENT} "Release")
          create_imported_library_install_properties(${${LIBRARYNAME}_DLL_DEBUG_NAME} ${ARG_INSTALLATION_COMPONENT} "Debug")
        else(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
          create_imported_library_install_properties(${${LIBRARYNAME}_DLL_RELEASE_NAME} ${ARG_INSTALLATION_COMPONENT} "Release;Debug")
        endif(NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME STREQUAL "${LIBRARYNAME}_LIBRARY_DEBUG_NAME-NOTFOUND")
    endif(ARG_INSTALLATION_COMPONENT)

endfunction(create_imported_library LIBRARYNAME HEADERFILE LIBRARYFILE)
