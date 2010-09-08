# Finds and imports all the EXPAT libraries.

create_imported_library(
    EXPAT
    expat.h libexpat SHARED
    INCLUDE_SEARCH_PATH
        ${EXPAT_DIR}/include ENV{EXPAT_DIR}/include E:/devel/expat-2.0.1
    LIBRARY_SEARCH_PATH 
        ${EXPAT_DIR}/lib ENV{EXPAT_DIR}/lib E:/devel/expat-2.0.1
)
