# Finds and imports sqlite3 library.

create_imported_library(
    SQLITE3
    sqlite3/sqlite3.h sqlite3 SHARED
    INCLUDE_SEARCH_PATH ${SQLITE3_DIR}/include ENV{SQLITE3_DIR}/include
    LIBRARY_SEARCH_PATH ${SQLITE3_DIR}/lib ENV{SQLITE3_DIR}/lib
)
