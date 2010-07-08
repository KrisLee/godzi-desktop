# Function for executable setup
# Usage: create_executable(EXENAME PROJECTNAME FILES <file> [<file> [...]]
#                       [WIN32]
#                       [PROJECTLABEL <project-label>]
#                       [DEPENDENCIES <project-name> [<project-name> [...]]]
#                       [LIBDEPENDENCIES <library-name> [<library-name> [...]]]
#                       [INCLUDE_PATH <include-path> [<include-path> [...]]]
#                       [DEFINITIONS <definition> [<definition> [...]]]
#                       [LIBRARY_PATH <library-path> [<library-path> [...]]]
#                       [LIBRARIES <library> [<library> [...]]]
#                       [INSTALLATION_COMPONENT <component-name>])
#
# Only the EXENAME, PROJECTNAME, and FILES parameters are required.  All other parameters are optional.  
# EXENAME is the name that will be used when creating the executable, with the pattern ${EXENAME}.exe for Windows 
# and ${EXENAME} for UNIX/Linux.  PROJECTNAME specifies the name of the project which contains the executable.  The
# project's dependency lists, both internal and external, will automatically be associated with the executable.  This project must already 
# exist.  The list of files specified for the FILES parameter should include all files to be associated with the project (and associated 
# with IDE projects) which includes header files, source files, and resource files (for Windows icons, etc).  
#
# The WIN32 option specifies that an executable should be a Windows program, not
# a console program.  This option is igonred by non-Microsoft Windows systems.
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

function(create_executable EXENAME PROJECTNAME)

    # Make dependency list prefix
    set(LISTPREFIX ${EXENAME}_EXE)

    # Argument list names
    set(arglists "FILES;PROJECTLABEL;LIBDEPENDENCIES;DEPENDENCIES;INCLUDE_PATH;DEFINITIONS;LIBRARY_PATH;LIBRARIES;INSTALLATION_COMPONENT")
    
    # Get the argument lists
    parse_arguments(ARG "${arglists}" "WIN32" ${ARGN})
    
    # Create the dependency lists.  Instead of jsut specifying ${ARGN}, separate lists so the ordering of list specification does not matter (basically extracting the SUBDIRECTORIES list)
    create_dependency_lists(${LISTPREFIX}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            DEPENDENCIES ${ARG_DEPENDENCIES}
                            LIBDEPENDENCIES ${ARG_LIBDEPENDENCIES}
                            INCLUDE_PATH ${ARG_INCLUDE_PATH}
                            DEFINITIONS ${ARG_DEFINITIONS}
                            LIBRARY_PATH ${ARG_LIBRARY_PATH}
                            LIBRARIES ${ARG_LIBRARIES})

    # Merge the parent's internal and external dependencies
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

    # Setup paths and pre-processor definitions
    include_directories(${${LISTPREFIX}_INTERNAL_INCLUDE_PATH} ${${LISTPREFIX}_EXTERNAL_INCLUDE_PATH})
    add_definitions(${${LISTPREFIX}_INTERNAL_DEFINITIONS} ${${LISTPREFIX}_EXTERNAL_DEFINITIONS})
    link_directories(${${LISTPREFIX}_INTERNAL_LIBRARY_PATH} ${${LISTPREFIX}_EXTERNAL_LIBRARY_PATH})

    # Add the executable target
    # Check for non-console mode application for Windows systems
    if(MSVC AND ARG_WIN32)
        add_executable(${EXENAME} WIN32 ${ARG_FILES})
        set_target_properties(${EXENAME} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
    else(MSVC AND ARG_WIN32)
        add_executable(${EXENAME} ${ARG_FILES})
    endif(MSVC AND ARG_WIN32)
    
    # Add the proper POSTFIX; CMake does not add it executables by default
    set_target_properties(${EXENAME}
        PROPERTIES
            DEBUG_OUTPUT_NAME ${EXENAME}${CMAKE_DEBUG_POSTFIX}
    )
    
    # Add non-link dependencies to ensure proper order for code generation, etc
    set(dependencies "")
    unique_append(dependencies ${${LISTPREFIX}_INTERNAL_DEPENDENCIES} ${${LISTPREFIX}_EXTERNAL_DEPENDENCIES})
    if(dependencies)
        add_dependencies(${EXENAME} ${dependencies})
    endif(dependencies)

    # Add target's dependencies
    set(libdependencies "")
    unique_append(libdependencies ${${LISTPREFIX}_INTERNAL_LIBDEPENDENCIES} ${${LISTPREFIX}_EXTERNAL_LIBDEPENDENCIES})
    if(libdependencies)
        target_link_libraries(${EXENAME} ${libdependencies})
    endif(libdependencies)

    # Add the link libraries for the target
    set(libraries "")
    unique_append(libraries ${${LISTPREFIX}_INTERNAL_LIBRARIES} ${${LISTPREFIX}_EXTERNAL_LIBRARIES})
    if(libraries)
        target_link_libraries(${EXENAME} ${libraries})
    endif(libraries)

    # If a project label has been specified, use it to override the default project label
    if(ARG_PROJECTLABEL)
        set_target_properties(${EXENAME} PROPERTIES PROJECT_LABEL "${ARG_PROJECTLABEL}")
    endif(ARG_PROJECTLABEL)

    # If an installation component was specified, create installation settings
    if(ARG_INSTALLATION_COMPONENT)
        create_executable_install_properties(${EXENAME} ${ARG_INSTALLATION_COMPONENT} RPATH)
    endif(ARG_INSTALLATION_COMPONENT)

endfunction(create_executable EXENAME PROJECTNAME)
