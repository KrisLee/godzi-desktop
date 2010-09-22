# Finds and imports boost library.

find_path(BOOST_INCLUDE_DIR boost/any.hpp
  PATHS
  ~/Library/Frameworks/boost/Headers
  /Library/Frameworks/boost/Headers
  /usr/local/include/boost
  /usr/local/include/Boost
  /usr/local/include
  /usr/include/boost
  /usr/include/Boost
  /usr/include
  /sw/include/boost 
  /sw/include/Boost 
  /sw/include # Fink
  /opt/local/include/boost
  /opt/local/include/Boost
  /opt/local/include # DarwinPorts
  /opt/csw/include/boost
  /opt/csw/include/Boost
  /opt/csw/include # Blastwave
  /opt/include/boost
  /opt/include/Boost
  /opt/include
  e:/devel/boost_1_43_0
  e:/devel/boost_1_43_0/boost_1_43_0
)

SET(BOOST_FOUND "NO")
IF(BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND "YES")
  SET(BOOST_INCLUDES ${BOOST_INCLUDE_DIR})
ENDIF(BOOST_INCLUDE_DIR)
