# Finds and imports sqlite3 library.

create_imported_library(
    KML_DOM
    kml/dom.h libkmldom SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

create_imported_library(
    KML_BASE
    kml/base/zip_file.h libkmlbase SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

create_imported_library(
    KML_ENGINE
    kml/engine.h libkmlengine SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

create_imported_library(
    KML_XSD
    kml/xsd/xst_parser.h libkmlxsd SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

create_imported_library(
    KML_CONVENIENCE
    kml/convenience/convenience.h libkmlconvenience SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

create_imported_library(
    KML_REGIONATOR
    kml/regionator/regionator.h libkmlregionator SHARED
    INCLUDE_SEARCH_PATH ${KML_DIR}/include ENV{KML_DIR}/include
    LIBRARY_SEARCH_PATH ${KML_DIR}/lib ENV{KML_DIR}/lib
)

set(KML_ALL_LIBDEPENDENCIES
    KML_DOM KML_BASE KML_ENGINE KML_XSD KML_CONVENIENCE KML_REGIONATOR)
