project(q-xdg)
cmake_minimum_required(VERSION 2.6)
set(XDG_LIB_VERSION "0.1")

message("-- Checking for Qt 4...")
find_package(Qt4 REQUIRED)

option(WITH_DOXYGEN "" ON)

if(WITH_DOXYGEN)
    message("-- Checking for Doxygen...")
    find_package(Doxygen)
endif()

set(QT_USE_QTCORE TRUE)
set(QT_USE_QTGUI TRUE)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${QT_DEFINITIONS})

set(QXDG_SOURCES
    src/xdgenvironment.cpp
    src/xdgicontheme.cpp
    src/xdgiconmanager.cpp
    src/xdgthemechooser.cpp
    src/xdgicon.cpp
    src/xdgiconengine.cpp
)

set(QXDG_HEADERS
    src/xdg.h
    src/xdgenvironment.h
    src/xdgicontheme.h
    src/xdgiconmanager.h
    src/xdgthemechooser.h
    src/xdgicon.h
)

set(QXDG_PRIVATE_HEADERS
    src/xdgicontheme_p.h
    src/xdgiconmanager_p.h
    src/xdgiconengine_p.h
)

set(TEST_SOURCES test/main.cpp)

qt4_automoc(${QXDG_SOURCES} ${TEST_SOURCES})
add_library(q-xdg SHARED ${QXDG_SOURCES} ${QXDG_HEADERS} ${QXDG_PRIVATE_HEADERS})
target_link_libraries(q-xdg ${QT_QTCORE_LIBRARY})

add_executable(qxdgtest ${TEST_SOURCES})
target_link_libraries(qxdgtest ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} q-xdg)
add_dependencies(qxdgtest q-xdg)

set_target_properties(q-xdg PROPERTIES VERSION ${XDG_LIB_VERSION} SOVERSION "0")
install(TARGETS q-xdg DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${QXDG_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/q-xdg)

if(DOXYGEN_FOUND)
    set(DOC_TARGET "doc")
    configure_file(doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)
    add_custom_target(${DOC_TARGET} ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc DESTINATION ${CMAKE_INSTALL_PREFIX}/share/q-xdg-${XDG_LIB_VERSION})
    add_dependencies(q-xdg ${DOC_TARGET})
endif()
