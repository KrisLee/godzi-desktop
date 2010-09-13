# Finds and imports sqlite3 library.

create_imported_library(
    KML_DOM
    kml/dom.h kmldom SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

create_imported_library(
    KML_BASE
    kml/base/zip_file.h kmlbase SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

create_imported_library(
    KML_ENGINE
    kml/engine.h kmlengine SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

create_imported_library(
    KML_XSD
    kml/xsd/xst_parser.h kmlxsd SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

create_imported_library(
    KML_CONVENIENCE
    kml/convenience/convenience.h kmlconvenience SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

create_imported_library(
    KML_REGIONATOR
    kml/regionator/regionator.h kmlregionator SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ${KML_DIR}/src ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ${KML_DIR}/msvc ENV{KML_DIR}/lib
)

set(KML_ALL_LIBDEPENDENCIES
    KML_DOM KML_BASE KML_ENGINE KML_XSD KML_CONVENIENCE KML_REGIONATOR)
