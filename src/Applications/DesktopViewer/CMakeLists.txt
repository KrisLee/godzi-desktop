project( GODZI_DESKTOP_VIEWER )


set(PROJECT_FILES
    main.cpp
    DesktopMainWindow.cpp
    OpenFileDialog.cpp
    AboutDialog.cpp
    ServerManagementWidget.cpp
)

# Qt resrouce files
set(GODZI_DESKTOP_RCS
    AppImages.qrc
)

# Qt .ui files
set(GODZI_DESKTOP_UIS
    ui/OpenFileDialog.ui
    ui/AboutDialog.ui
)

# Header files that need to be moc'd
set (GODZI_DESKTOP_MOC_HDRS
    DesktopMainWindow
    OpenFileDialog
    AboutDialog
    ServerManagementWidget
)

FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

# add Qt resource files
QT4_ADD_RESOURCES( GODZI_DESKTOP_RC_SRCS ${GODZI_DESKTOP_RCS} )

# run uic on Qt .ui files
QT4_WRAP_UI( GODZI_DESKTOP_UI_HDRS ${GODZI_DESKTOP_UIS} )

# moc the files generated above
QT4_WRAP_CPP( GODZI_DESKTOP_UI_SRCS ${GODZI_DESKTOP_UI_HDRS} )

# moc necessary header files
QT4_WRAP_CPP( GODZI_DESKTOP_MOC_SRCS ${GODZI_DESKTOP_MOC_HDRS} )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

create_executable(
    godzi                          # executable name
    GODZI_DESKTOP_VIEWER           # project from which to build executable
    FILES
        ${PROJECT_FILES}
	${GODZI_DESKTOP_RCS}
	${GODZI_DESKTOP_RC_SRCS}
	${GODZI_DESKTOP_UIS}
	${GODZI_DESKTOP_UI_HDRS}
	${GODZI_DESKTOP_UI_SRCS}
	${GODZI_DESKTOP_MOC_HDRS}
	${GODZI_DESKTOP_MOC_SRCS}
    PROJECTLABEL
        "Application - Godzi Desktop"
    LIBDEPENDENCIES ${GODZI_SDK_LIB_LIBDEPENDENCIES} GODZI_SDK
    INCLUDE_PATH ${GODZI_SDK_LIB_INCLUDE_PATH}
    INSTALLATION_COMPONENT
        "Applications"
)
