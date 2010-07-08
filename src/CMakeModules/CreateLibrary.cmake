# Function for library/archive setup
# Usage: create_library(LIBRARYNAME PROJECTNAME [STATIC|SHARED] FILES <file> [<file> [...]]
#                       [PROJECTLABEL <project-label>]
#                       [DEPENDENCIES [<project-name> [<project-name> [...]]] [INTERNAL <project-name>] ... ]
#                       [LIBDEPENDENCIES [<library-name> [<library-name> [...]]] [INTERNAL <library-name>] ... ]
#                       [INCLUDE_PATH [<include-path> [<include-path> [...]]] [INTERNAL <include-path>] ... ]
#                       [DEFINITIONS [<definition> [<definition> [...]]] [INTERNAL <definition>] ... ]
#                       [LIBRARY_PATH [<library-path> [<library-path> [...]]] [INTERNAL <library-path>] ... ]
#                       [LIBRARIES [<library> [<library> [...]]] [INTERNAL <library>] ... ]
#                       [INSTALLATION_COMPONENT <component-name>])
#
# Each library created exports include path, definition, library path, and libary variables to be used with projects which depend on it.  
# The variables are named as ${LIBRARYNAME}_LIB_INCLUDE_PATH, ${LIBRARYNAME}_LIB_DEFINITIONS, ${LIBRARYNAME}_LIB_LIBRARY_PATH, and ${LIBRARYNAME}_LIB_LIBRARIES.
# Arguments prefixed with the INTERNAL modifier will be excluded from the exported variables.
#
# Only the LIBRARYNAME, PROJECTNAME, and FILES parameters are required.  All other parameters are optional.  
# LIBRARYNAME is the name that will be used when creating the library, with the pattern ${LIBRARYNAME}.lib for Windows 
# and lib${LIBRARYNAME}.a for UNIX/Linux.  PROJECTNAME specifies the name of the project which contains the library.  The
# project's dependency lists, both internal and external, will automatically be associated with the library.  This project must already 
# exist.  The list of files specified for the FILES parameter should include all files to be associated with the project (and associated 
# with IDE projects) which includes header files, source files, and resource files (for Windows icons, etc).  
#
# The STATIC and SHARED options indicate the library type.  Only one option may be specified for a library.  If neither option is
# specified, the default is STATIC.  
#
# The DEPENDENCIES parameter specifies a list of names for other projects created by the create_project macro 
# on which the new project depends.  The names are the values specified for the PROJECTNAME parameter when the 
# dependecies were created.  Include paths, definitions, library paths, and libraries associated with each dependency
# will automatically be added for the current project.  Values from projects not marked INTERNAL will also be exported 
# as part of the include paths, definitions, library paths, and libraries variables defined for the new project.  
#
# The LIBDEPENDENCIES parameter is like the DEPENDENCIES parameter for library targets created with the create_library macro 
# instead of projects created by the create_project macro.  The <library-name> value is included as a link dependency of the library.  
#
# INCLUDEPATHS, DEFINITIONS, LIBRARYPATHS, and LIBRARIES define the dependencies for the project which are not included by any
# of the projects specified with the DEPENDENCIES parameter.  This could include system or external third-party dependencies.  
# Values not marked INTERNAL will be added to the exported variables defined for the new library.  

function(create_library LIBRARYNAME PROJECTNAME)

    # Make dependency list prefix
    set(LISTPREFIX ${LIBRARYNAME}_LIB)

    # Argument list names
    set(arglists "FILES;PROJECTLABEL;LIBDEPENDENCIES;DEPENDENCIES;INCLUDE_PATH;DEFINITIONS;LIBRARY_PATH;LIBRARIES;INSTALLATION_COMPONENT")
    
    # Option names
    set(options "STATIC;SHARED")

    # Get the argument lists
    parse_arguments(ARG "${arglists}" "${options}" ${ARGN})
    
    # Create the dependency lists.  Instead of just specifying ${ARGN}, separate lists so the ordering of list specification does not matter (basically extracting the SUBDIRECTORIES list)
    create_dependency_lists(${LISTPREFIX}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            DEPENDENCIES ${ARG_DEPENDENCIES}
                            LIBDEPENDENCIES ${ARG_LIBDEPENDENCIES}
                            INCLUDE_PATH ${ARG_INCLUDE_PATH}
                            DEFINITIONS ${ARG_DEFINITIONS}
                            LIBRARY_PATH ${ARG_LIBRARY_PATH}
                            LIBRARIES ${ARG_LIBRARIES})

    # Merge the parent's external and internal dependencies
    unique_append(${LISTPREFIX}_INTERNAL_LIBDEPENDENCIES ${${PROJECTNAME}_INTERNAL_LIBDEPENDENCIES})
    unique_append(${LISTPREFIX}_INTERNAL_INCLUDE_PATH ${${PROJECTNAME}_INTERNAL_INCLUDE_PATH})
    unique_append(${LISTPREFIX}_INTERNAL_DEFINITIONS ${${PROJECTNAME}_INTERNAL_DEFINITIONS})
    unique_append(${LISTPREFIX}_INTERNAL_LIBRARY_PATH ${${PROJECTNAME}_INTERNAL_LIBRARY_PATH})
    unique_append(${LISTPREFIX}_INTERNAL_LIBRARIES ${${PROJECTNAME}_INTERNAL_LIBRARIES})
    unique_append(${LISTPREFIX}_EXTERNAL_LIBDEPENDENCIES ${${PROJECTNAME}_EXTERNAL_LIBDEPENDENCIES})
    unique_append(${LISTPREFIX}_EXTERNAL_INCLUDE_PATH ${${PROJECTNAME}_EXTERNAL_INCLUDE_PATH})
    unique_append(${LISTPREFIX}_EXTERNAL_DEFINITIONS ${${PROJECTNAME}_EXTERNAL_DEFINITIONS})
    unique_append(${LISTPREFIX}_EXTERNAL_LIBRARY_PATH ${${PROJECTNAME}_EXTERNAL_LIBRARY_PATH})
    unique_append(${LISTPREFIX}_EXTERNAL_LIBRARIES ${${PROJECTNAME}_EXTERNAL_LIBRARIES})
    
    # Export the project's global (cached) variables
    set(${LISTPREFIX}_LIBDEPENDENCIES ${${LISTPREFIX}_EXTERNAL_LIBDEPENDENCIES} CACHE INTERNAL "${LISTPREFIX} project link dependencies on other project's libraries")
    set(${LISTPREFIX}_INCLUDE_PATH ${${LISTPREFIX}_EXTERNAL_INCLUDE_PATH} CACHE INTERNAL "${LISTPREFIX} project include path")
    set(${LISTPREFIX}_DEFINITIONS ${${LISTPREFIX}_EXTERNAL_DEFINITIONS} CACHE INTERNAL "${LISTPREFIX} project pre-processor definitions")
    set(${LISTPREFIX}_LIBRARY_PATH ${${LISTPREFIX}_EXTERNAL_LIBRARY_PATH} CACHE INTERNAL "${LISTPREFIX} project library path")
    set(${LISTPREFIX}_LIBRARIES ${${LISTPREFIX}_EXTERNAL_LIBRARIES} CACHE INTERNAL "${LISTPREFIX} project required libraries")
    
    # Determine library type as static or dynamic
    if(ARG_SHARED)
        set(type SHARED)
    else(ARG_SHARED)
        set(type STATIC)
    endif(ARG_SHARED)
    
    #Include pre-processor defs to support DLL linkage on windows:
    if(WIN32)
        if(ARG_SHARED)
            add_definitions( -D${LISTPREFIX}_EXPORT_SHARED )
        else(ARG_SHARED)
            add_definitions( -D${LISTPREFIX}_EXPORT_STATIC )
        endif(ARG_SHARED)
    endif(WIN32)

    # Setup paths and pre-processor definitions
    include_directories(${${LISTPREFIX}_INTERNAL_INCLUDE_PATH} ${${LISTPREFIX}_EXTERNAL_INCLUDE_PATH})
    add_definitions( ${${LISTPREFIX}_INTERNAL_DEFINITIONS} ${${LISTPREFIX}_EXTERNAL_DEFINITIONS} )
    link_directories(${${LISTPREFIX}_INTERNAL_LIBRARY_PATH} ${${LISTPREFIX}_EXTERNAL_LIBRARY_PATH})

    # Add the library target
    add_library(${LIBRARYNAME} ${type} ${ARG_FILES})

    # Add non-link dependencies to ensure proper order for code generation, etc
    set(dependencies "")
    unique_append(dependencies ${${LISTPREFIX}_INTERNAL_DEPENDENCIES} ${${LISTPREFIX}_EXTERNAL_DEPENDENCIES})
    if(dependencies)
        add_dependencies(${LIBRARYNAME} ${dependencies})
    endif(dependencies)

    # Add target's library dependencies
    set(libdependencies "")
    unique_append(libdependencies ${${LISTPREFIX}_INTERNAL_LIBDEPENDENCIES} ${${LISTPREFIX}_EXTERNAL_LIBDEPENDENCIES})
    if(libdependencies)
        target_link_libraries(${LIBRARYNAME} ${libdependencies})
    endif(libdependencies)

    # Add the link libraries for the target
    set(libraries "")
    unique_append(libraries ${${LISTPREFIX}_INTERNAL_LIBRARIES} ${${LISTPREFIX}_EXTERNAL_LIBRARIES})
    if(libraries)
        target_link_libraries(${LIBRARYNAME} ${libraries})
    endif(libraries)

    # If a project label has been specified, use it to override the default project label
    if(ARG_PROJECTLABEL)
        set_target_properties(${LIBRARYNAME} PROPERTIES PROJECT_LABEL "${ARG_PROJECTLABEL}")
    endif(ARG_PROJECTLABEL)

    # If an installation component was specified, create installation settings
    if(ARG_INSTALLATION_COMPONENT)
        create_library_install_properties(${LIBRARYNAME} ${ARG_INSTALLATION_COMPONENT})
    endif(ARG_INSTALLATION_COMPONENT)
    
endfunction(create_library LIBRARYNAME PROJECTNAME)
