project( GODZI_DESKTOP_VIEWER )

set(PROJECT_FILES
    main.cpp
)

create_executable(
    godzi                          # executable name
    GODZI_DESKTOP_VIEWER           # project from which to build executable
    FILES
        ${PROJECT_FILES}
    PROJECTLABEL
        "Application - Godzi Desktop"
    LIBDEPENDENCIES
        GODZI_SDK
    INSTALLATION_COMPONENT
        "Applications"
)
