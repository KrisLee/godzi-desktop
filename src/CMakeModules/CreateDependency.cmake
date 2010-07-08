# Macro to setup an empty dependency; defines header only projects that do not build a target,
# but may need to perform generation of header files or other custom build steps.  Visual Studio
# projcets will be created to contain the files associated with the dependency
# Usage: create_dependency(DEPENDENCYNAME PROJECTNAME FILES <file> [<file> [...]]
#                         [DEPENDENCIES [<project-name> [<project-name> [...]]] [INTERNAL <project-name>] ... ]
#                         [LIBDEPENDENCIES [<library-name> [<library-name> [...]]] [INTERNAL <library-name>] ... ]
#                         [INCLUDE_PATH [<include-path> [<include-path> [...]]] [INTERNAL <include-path>] ... ]
#                         [DEFINITIONS [<definition> [<definition> [...]]] [INTERNAL <definition>] ... ]
#                         [LIBRARY_PATH [<library-path> [<library-path> [...]]] [INTERNAL <library-path>] ... ]
#                         [LIBRARIES [<library> [<library> [...]]] [INTERNAL <library>] ... ])
#
# Each dependency created exports include path, definition, library path, and libary variables to be used with projects which depend on it.
# The variables are named as ${DEPENDENCYNAME}_DEP_INCLUDE_PATH, ${DEPENDENCYNAME}_DEP_DEFINITIONS, ${DEPENDENCYNAME}_DEP_LIBRARY_PATH, and ${DEPENDENCYNAME}_DEP_LIBRARIES.
# Arguments prefixed with the INTERNAL modifier will be excluded from the exported variables.
#
# Only the DEPENDENCYNAME, PROJECTNAME, and FILES parameters are required.  All other parameters are optional.
# DEPENDENCYNAME is the name that will be used when creating the dependency.  PROJECTNAME specifies the name of the project which contains the
# dependency.  The project's dependency lists, both internal and external, will automatically be associated with the dependency.  This project must already
# exist.  The list of files specified for the FILES parameter should include all files to be associated with the project (and associated
# with IDE projects) which includes header files, source files, and resource files (for Windows icons, etc).
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

function(create_dependency DEPENDENCYNAME PROJECTNAME)

    # Make dependency list prefix
    set(LISTPREFIX ${DEPENDENCYNAME}_DEP)

    # Argument list names
    set(arglists "FILES;PROJECTLABEL;LIBDEPENDENCIES;DEPENDENCIES;INCLUDE_PATH;DEFINITIONS;LIBRARY_PATH;LIBRARIES")

    # Get the argument lists
    parse_arguments(ARG "${arglists}" "" ${ARGN})

    # Create the dependency lists.  Instead of just specifying ${ARGN}, separate lists so the ordering of list specification does not matter (basically extracting the SUBDIRECTORIES list)
    create_dependency_lists(${LISTPREFIX}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            DEPENDENCIES ${ARG_DEPENDENCIES}
                            LIBDEPENDENCIES ${ARG_LIBDEPENDENCIES}
                            INCLUDE_PATH ${ARG_INCLUDE_PATH}
                            DEFINITIONS ${ARG_DEFINITIONS}
                            LIBRARY_PATH ${ARG_LIBRARY_PATH}
                            LIBRARIES ${ARG_LIBRARIES})

    # Merge the parent's external dependencies with external dependencies
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

    # Add an empty custom build target
    add_custom_target(${DEPENDENCYNAME} SOURCES ${ARG_FILES})

    # Add non-link dependencies to ensure proper order for code generation, etc
    set(dependencies "")
    unique_append(dependencies ${${LISTPREFIX}_INTERNAL_DEPENDENCIES} ${${LISTPREFIX}_EXTERNAL_DEPENDENCIES})
    if(dependencies)
        add_dependencies(${DEPENDENCYNAME} ${dependencies})
    endif(dependencies)

    # If a project label has been specified, use it to override the default project label
    if(ARG_PROJECTLABEL)
        set_target_properties(${DEPENDENCYNAME} PROPERTIES PROJECT_LABEL "${ARG_PROJECTLABEL}")
    endif(ARG_PROJECTLABEL)

endfunction(create_dependency DEPENDENCYNAME PROJECTNAME)
