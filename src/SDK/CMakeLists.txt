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
	include/Godzi/Features/ApplyFeature
	include/Godzi/Features/Feature
	include/Godzi/Features/FeatureSource
	include/Godzi/Features/Geometry
	include/Godzi/Features/Placemark
	include/Godzi/Features/SymbolicNode
#	include/Godzi/Features/AGGLiteRasterizerTileSource
        )
set(CORE_SOURCE
	src/Godzi/Actions.cpp
	src/Godzi/Application.cpp
	src/Godzi/Project.cpp
	src/Godzi/Earth.cpp
	src/Godzi/KML.cpp
#	src/Godzi/Features/ApplyFeature
	src/Godzi/Features/Feature.cpp
	src/Godzi/Features/FeatureSource.cpp
	src/Godzi/Features/Geometry.cpp
	src/Godzi/Features/Placemark.cpp
	src/Godzi/Features/SymbolicNode.cpp
#	src/Godzi/Features/AGGLiteRasterizerTileSource.cpp
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

# ----- Qt moc headers -------------------------------------------------

FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

set(GODZI_SDK_MOC_HDRS
	include/Godzi/Application
)

QT4_WRAP_CPP( GODZI_SDK_MOC_SRCS ${GODZI_SDK_MOC_HDRS} )

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# ----------------------------------------------------------------------

set(PROJECT_FILES
    ${CORE_INCLUDE} ${CORE_SOURCE}
    ${UI_INCLUDE} ${UI_SOURCE}
    ${GODZI_SDK_MOC_SRCS}
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
