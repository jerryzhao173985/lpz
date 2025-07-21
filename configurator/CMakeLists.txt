# Configurator - Runtime parameter adjustment library

# Optional Java bridge feature (disabled by default)
option(ENABLE_JAVA_BRIDGE "Enable JNI-based Java bridge in configurator" OFF)

# This is a Qt-based library
# Require Qt - Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Core Widgets Xml QUIET)
if(Qt6_FOUND)
    # qt6_standard_project_setup() requires Qt 6.3+
    # For older Qt6 versions, set the required properties manually
    if(Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
    else()
        qt6_standard_project_setup()
    endif()
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Xml)
else()
    find_package(Qt5 COMPONENTS Core Widgets Xml)
    if(NOT Qt5_FOUND)
        message(WARNING "Qt5 not found. Skipping configurator.")
        return()
    endif()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Xml)
endif()

# Collect Qt widget sources
file(GLOB QCONFIGURABLE_SOURCES src/qconfigurable/*.cpp)

# Main sources (explicit list for Qt MOC)
set(CONFIGURATOR_MAIN_SOURCES
    src/ConfiguratorProxy.cpp
    src/QConfigurator.cpp
    src/QExtAction.cpp
    src/QLogViewWidget.cpp
)

# Use the unified library creation function
lpzrobots_add_component_library(configurator
    SOURCES
        ${CONFIGURATOR_MAIN_SOURCES}
        ${QCONFIGURABLE_SOURCES}
    PUBLIC_DEPS
        ${QT_LIBRARIES}
        lpzrobots::selforg
)

# Apply Java bridge compile definitions
if(ENABLE_JAVA_BRIDGE)
    target_compile_definitions(configurator PRIVATE USE_JAVA_BRIDGE)
    message(STATUS "Configurator: Java bridge enabled (compile-time macro only)")
else()
    target_compile_definitions(configurator PRIVATE NO_JAVA_BRIDGE)
endif()

# Additional include directories for Qt widgets
target_include_directories(configurator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/qconfigurable
)

# Compiler options - be lenient with configurator due to Qt code
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(configurator PRIVATE
        -Wno-deprecated-declarations
        -Wno-unused-parameter
        -Wno-old-style-cast
        -Wno-zero-as-null-pointer-constant
    )
    if(TARGET configurator_opt)
        target_compile_options(configurator_opt PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-old-style-cast
            -Wno-zero-as-null-pointer-constant
        )
    endif()
    if(TARGET configurator_dbg)
        target_compile_options(configurator_dbg PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-old-style-cast
            -Wno-zero-as-null-pointer-constant
        )
    endif()
endif()

# Create header symlinks for development
lpzrobots_create_header_symlinks(configurator)

# Install widget headers for users
file(GLOB CONFIGURATOR_MOC_HEADERS src/*.h)
file(GLOB QCONFIGURABLE_HEADERS src/qconfigurable/*.h)
install(FILES ${CONFIGURATOR_MOC_HEADERS} ${QCONFIGURABLE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/configurator/widgets
)

# Test application
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/testapp)
    add_subdirectory(testapp)
endif()