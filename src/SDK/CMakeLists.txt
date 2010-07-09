create_project(
    GODZI_SDK             # project name    
    INCLUDE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${QT_INCLUDES}
    LIBDEPENDENCIES
        ${OSG_ALL_LIBDEPENDENCIES}
        ${OSGEARTH_ALL_LIBDEPENDENCIES}
        ${QT_ALL_LIBRARIES}
)

# ----- common --------------------------------------------------------

set(COMMON_HEADERS
	Godzi/Common
)
source_group( Headers FILES ${COMMON_HEADERS} )

# ----- graphics -------------------------------------------------------

set(HEADERS
	Godzi/ViewerWidgets
)   
source_group( Headers FILES ${HEADERS} )

set(SOURCES
	Godzi/ViewerWidgets.cpp
)
source_group( Sources FILES ${SOURCES} )

# ----------------------------------------------------------------------

set(PROJECT_FILES
    ${COMMON_HEADERS}
    ${HEADERS} ${SOURCES}
)

create_library(
    GODZI_SDK                  # library name
    GODZI_SDK                  # project from which to create the library
    SHARED                     # STATIC or SHARED library
    FILES
        ${PROJECT_FILES}
    PROJECTLABEL
        "Godzi SDK"
    INSTALLATION_COMPONENT
        "SDK"
)
