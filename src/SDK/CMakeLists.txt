create_project(
    GODZI_SDK             # project name    
    INCLUDE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${QT_INCLUDES}
        ${KML_INCLUDES}
        ${EXPAT_INCLUDES}
        ${BOOST_INCLUDE_DIR}
    LIBDEPENDENCIES
        ${OSG_ALL_LIBDEPENDENCIES}
        ${OSGEARTH_ALL_LIBDEPENDENCIES}
        ${QT_ALL_LIBRARIES}
        ${KML_ALL_LIBDEPENDENCIES}
        EXPAT
)

# ----- CORE namespace -------------------------------------------------

set(CORE_INCLUDE
    include/Godzi/Common
	include/Godzi/Actions
	include/Godzi/Application
	include/Godzi/Project
	include/Godzi/DataSources
	include/Godzi/Earth
	include/Godzi/Features/Placemark
)
set(CORE_SOURCE
	src/Godzi/Actions.cpp
	src/Godzi/Application.cpp
	src/Godzi/Project.cpp
	src/Godzi/DataSources.cpp
	src/Godzi/Earth.cpp
	src/Godzi/Features/Placemark.cpp
)   
source_group( Core FILES ${CORE_INCLUDE} ${CORE_SOURCE} )

# ----- KML namespace ---------------------------------------------------

set(KML_INCLUDE
    include/Godzi/KML/KMLActions
	include/Godzi/KML/KMLFeatureSourceOptions
	include/Godzi/KML/KMLFeatureSource
	include/Godzi/KML/KMLDataSource
	include/Godzi/KML/KMLParser
	include/Godzi/KML/KMLSymbol
)
set(KML_SOURCE
    src/Godzi/KML/KMLActions.cpp
    src/Godzi/KML/KMLDataSource.cpp
	src/Godzi/KML/KMLFeatureSource.cpp
	src/Godzi/KML/KMLParser.cpp
)   
source_group( KML FILES ${KML_INCLUDE} ${KML_SOURCE} )

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
	include/Godzi/Project
	include/Godzi/DataSources
)

QT4_WRAP_CPP( GODZI_SDK_MOC_SRCS ${GODZI_SDK_MOC_HDRS} )

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# ----------------------------------------------------------------------

set(PROJECT_FILES
    ${CORE_INCLUDE} ${CORE_SOURCE}
    ${UI_INCLUDE} ${UI_SOURCE}
    ${KML_INCLUDE} ${KML_SOURCE}
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
