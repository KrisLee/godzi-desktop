create_project(
    GODZI_SDK             # project name    
    INCLUDE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${QT_INCLUDES}
        ${KML_INCLUDES}
    LIBDEPENDENCIES
        ${OSG_ALL_LIBDEPENDENCIES}
        ${OSGEARTH_ALL_LIBDEPENDENCIES}
        ${QT_ALL_LIBRARIES}
        ${KML_ALL_LIBDEPENDENCIES}
)

# ----- CORE namespace -------------------------------------------------

set(CORE_INCLUDE
    include/Godzi/Common
	include/Godzi/Actions
	include/Godzi/Application
	include/Godzi/Project
	include/Godzi/Earth
	include/Godzi/KML
	include/Godzi/Features/Placemark
	include/Godzi/Features/Feature
        )
set(CORE_SOURCE
	src/Godzi/Actions.cpp
	src/Godzi/Application.cpp
	src/Godzi/Project.cpp
	src/Godzi/Earth.cpp
	src/Godzi/KML.cpp
	src/Godzi/Features/Placemark.cpp
	src/Godzi/Features/Feature.cpp
)   
source_group( Core FILES ${CORE_INCLUDE} ${CORE_SOURCE} )

# ----- UI namespace ---------------------------------------------------

set(UI_INCLUDE
	include/Godzi/UI/ViewerWidgets
)
set(UI_SOURCE
	src/Godzi/UI/ViewerWidgets.cpp
)
source_group( UI FILES ${UI_INCLUDE} ${UI_SOURCE} )

# ----------------------------------------------------------------------

set(PROJECT_FILES
    ${CORE_INCLUDE} ${CORE_SOURCE}
    ${UI_INCLUDE} ${UI_SOURCE}
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
