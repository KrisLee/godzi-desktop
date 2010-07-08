# Finds and imports all the OSGEARTH libraries.

create_imported_library(
    OSGEARTH
    osgEarth/Common osgEarth SHARED
    INCLUDE_SEARCH_PATH ${OSGEARTH_DIR}/include ENV{OSGEARTH_DIR}/include
    LIBRARY_SEARCH_PATH ${OSGEARTH_DIR}/lib ENV{OSGEARTH_DIR}/lib
)

create_imported_library(
    OSGEARTH_UTIL
    osgEarthUtil/Common osgEarthUtil SHARED
    INCLUDE_SEARCH_PATH ${OSGEARTH_DIR}/include ENV{OSGEARTH_DIR}/include
    LIBRARY_SEARCH_PATH ${OSGEARTH_DIR}/lib ENV{OSGEARTH_DIR}/lib
)

create_imported_library(
    OSGEARTH_FEATURES
    osgEarthFeatures/Common osgEarthFeatures SHARED
    INCLUDE_SEARCH_PATH ${OSGEARTH_DIR}/include ENV{OSGEARTH_DIR}/include
    LIBRARY_SEARCH_PATH ${OSGEARTH_DIR}/lib ENV{OSGEARTH_DIR}/lib
)

create_imported_library(
    OSGEARTH_SYMBOLOGY
    osgEarthSymbology/Common osgEarthSymbology SHARED
    INCLUDE_SEARCH_PATH ${OSGEARTH_DIR}/include ENV{OSGEARTH_DIR}/include
    LIBRARY_SEARCH_PATH ${OSGEARTH_DIR}/lib ENV{OSGEARTH_DIR}/lib
)

set(OSGEARTH_ALL_LIBDEPENDENCIES
    OSGEARTH OSGEARTH_UTIL OSGEARTH_FEATURES OSGEARTH_SYMBOLOGY)
    