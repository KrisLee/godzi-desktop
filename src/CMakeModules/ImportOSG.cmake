# Setup OSG library
# Setting the OPENSCENEGRAPH_DIR environment variable will allow use of a custom built library
#
# Example:
#
#  create_executable(
#      my_executavle MyExecutable
#      FILES
#          ${PROJECT_FILES}
#      INCLUDE_PATH
#          ${OSG_INCLUDE_PATH}
#      LIBDEPENDENCIES
#          ${OSG_COMMON_LIBRARIES}
#      PROJECTLABEL
#          "My Project" )
#

set(OSG_DEFINES "")

# Setup search paths
set(INCLUDE_DIRS
	OSG_DIR/include
    ${OSG_DIR}/include
    $ENV{OSG_DIR}/include
)

set(LIB_DIRS 
    ${OSG_DIR}/lib
    $ENV{OSG_DIR}/lib
)

macro(import_osg_headers_library LIBRARYNAME HEADERFILE LIBRARYFILE)

    create_imported_library(
        ${LIBRARYNAME} ${HEADERFILE} ${LIBRARYFILE} SHARED
        INCLUDE_SEARCH_PATH ${INCLUDE_DIRS}
        LIBRARY_SEARCH_PATH ${LIB_DIRS}
        DEFINITIONS ${OSG_DEFINES}
    )

endmacro(import_osg_headers_library LIBRARYNAME HEADERFILE LIBRARYFILE)

macro(import_ot_headers_library LIBRARYNAME HEADERFILE LIBRARYFILE)

    create_imported_library(
        ${LIBRARYNAME} ${HEADERFILE} ${LIBRARYFILE} SHARED
        INCLUDE_SEARCH_PATH ${INCLUDE_DIRS}
        LIBRARY_SEARCH_PATH ${LIB_DIRS}
        DEFINITIONS ${OSG_DEFINES}
    )
    
endmacro(import_ot_headers_library LIBRARYNAME HEADERFILE LIBRARYFILE)

macro(import_osg_library LIBRARYNAME LIBRARYFILE)

    create_imported_library(
        ${LIBRARYNAME} "" ${LIBRARYFILE} SHARED
        INCLUDE_SEARCH_PATH ${INCLUDE_DIRS}
        LIBRARY_SEARCH_PATH ${LIB_DIRS}
        DEFINITIONS ${OSG_DEFINES}
    )

endmacro(import_osg_library LIBRARYNAME LIBRARYFILE)

import_osg_headers_library(OSG osg/Version osg)
import_osg_library(OSGANIMATION osgAnimation)
import_osg_library(OSGDB osgDB)
import_osg_library(OSGFX osgFX)
import_osg_library(OSGGA osgGA)
import_osg_library(OSGMANIPULATOR osgManipulator)
import_osg_library(OSGPARTICLE osgParticle)
import_osg_library(OSGPRESENTATION osgPresentation)
import_osg_library(OSGSHADOW osgShadow)
import_osg_library(OSGSIM osgSim)
import_osg_library(OSGTERRAIN osgTerrain)
import_osg_library(OSGTEXT osgText)
import_osg_library(OSGUTIL osgUtil)
import_osg_library(OSGVIEWER osgViewer)
import_osg_library(OSGVOLUME osgVolume)
import_osg_library(OSGWIDGET osgWidget)
import_osg_library(OSGQT osgQt)
import_ot_headers_library(OPENTHREADS OpenThreads/Version OpenThreads)

set(OSG_COMMON_LIBDEPENDENCIES
    OSG OSGDB OSGGA OSGUTIL OSGVIEWER OPENTHREADS
)
set(OSG_ALL_LIBDEPENDENCIES
    OSG OSGDB OSGGA OSGSIM OSGTERRAIN OSGTEXT OSGUTIL OSGVIEWER OSGWIDGET OSGQT OSGANIMATION OPENTHREADS
)
