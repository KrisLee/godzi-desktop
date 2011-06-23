# Finds and imports sqlite3 library.

find_package(Qt4)
include(${QT_USE_FILE})

set(QT_ALL_LIBRARIES ${QT_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTWEBKIT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY})
