# Function for plugin setup
# Usage: create_plugin(LIBRARYNAME PROJECTNAME FILES <file> [<file> [...]]
#                      [PROJECTLABEL <project-label>]
#                      [DEPENDENCIES [<project-name> [<project-name> [...]]] [INTERNAL <project-name>] ... ]
#                      [LIBDEPENDENCIES [<library-name> [<library-name> [...]]] [INTERNAL <library-name>] ... ]
#                      [INCLUDE_PATH [<include-path> [<include-path> [...]]] [INTERNAL <include-path>] ... ]
#                      [DEFINITIONS [<definition> [<definition> [...]]] [INTERNAL <definition>] ... ]
#                      [LIBRARY_PATH [<library-path> [<library-path> [...]]] [INTERNAL <library-path>] ... ]
#                      [LIBRARIES [<library> [<library> [...]]] [INTERNAL <library>] ... ]
#                      [INSTALLATION_COMPONENT <component-name>])
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
# Plug-ins are always created as shared objects/Dlls
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

function(create_plugin LIBRARYNAME PROJECTNAME)

    # Separate ARGN into specific lists so that the INSTALLATION_COMPONENT can be removed from create_library arg list
    set(arglists "FILES;PROJECTLABEL;LIBDEPENDENCIES;DEPENDENCIES;INCLUDE_PATH;DEFINITIONS;LIBRARY_PATH;LIBRARIES;INSTALLATION_COMPONENT")
    
    # Get the argument lists
    parse_arguments(ARG "${arglists}" "" ${ARGN})

    # Recreate ARGN without INSTALLATION_COMPONENT
    if(ARG_INSTALLATION_COMPONENT)
        set(ARGN FILES ${ARG_FILES} 
                 PROJECTLABEL ${ARG_PROJECTLABEL}
                 LIBDEPENDENCIES ${ARG_LIBDEPENDENCIES}
                 DEPENDENCIES ${ARG_DEPENDENCIES}
                 INCLUDE_PATH ${ARG_INCLUDE_PATH}
                 DEFINITIONS ${ARG_DEFINITIONS}
                 LIBRARY_PATH ${ARG_LIBRARY_PATH}
                 LIBRARIES ${ARG_LIBRARIES})
    endif(ARG_INSTALLATION_COMPONENT)


    # Create the shared object
    create_library(${LIBRARYNAME} ${PROJECTNAME} SHARED ${ARGN})

    # Create installation properties
    if(ARG_INSTALLATION_COMPONENT)
        create_plugin_install_properties(${LIBRARYNAME} ${ARG_INSTALLATION_COMPONENT} RPATH)
    endif(ARG_INSTALLATION_COMPONENT)

    # Set target prefix and postfix properties
    set(PLUGIN_PREFIX "pi")
    if(LIBRARYNAME MATCHES "^pi")
        set(PLUGIN_PREFIX "")
    endif(LIBRARYNAME MATCHES "^pi")
    set_target_properties(${LIBRARYNAME} PROPERTIES PREFIX "${PLUGIN_PREFIX}"
                                                    RELEASE_POSTFIX "_${BUILD_SYSTEM_LIB_SUFFIX}${CMAKE_RELEASE_POSTFIX}"
                                                    DEBUG_POSTFIX "_${BUILD_SYSTEM_LIB_SUFFIX}${CMAKE_DEBUG_POSTFIX}")

endfunction(create_plugin LIBRARYNAME PROJECTNAME)
