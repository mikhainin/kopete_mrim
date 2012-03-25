# cmake -RCMAKE_INSTALL_PREFIX=/ -DEXEC_INSTALL_PREFIX=/usr -DSHARE_INSTALL_PREFIX=/usr/share/ -DCMAKE_BUILD_TYPE=DebugFull ..

project(kopete_mrim)

cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
find_package(Kopete REQUIRED)
find_package(KdepimLibs REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

if (Kopete_FOUND)
    message(STATUS "Kopete include dir is: ${KOPETE_INCLUDE_DIR}")
    include_directories(${KOPETE_INCLUDE_DIR})
    include_directories("${KOPETE_INCLUDE_DIR}/kopete")
else(Kopete_FOUND)
    message(ERROR "Kopete devel libraries do not found")
endif(Kopete_FOUND)

# include_directories(${KDEPIMLIBS_INCLUDE_DIRS})

include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(kopete_mrim_SRCS
    mrimprotocol.cpp
    mrimcontact.cpp
    mrimaddcontactpage.cpp
    mrimeditaccountwidget.cpp
    mrimaccount.cpp
)

kde4_add_ui_files(kopete_mrim_SRCS
    mrimaddui.ui
    mrimaccountpreferences.ui
)

add_subdirectory (mra)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=89326)
kde4_add_plugin(kopete_mrim ${kopete_mrim_SRCS} )

target_link_libraries (kopete_mrim mra)

target_link_libraries(kopete_mrim ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDE4_KDNSSD_LIBS} kopete)

install( TARGETS kopete_mrim  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kopete_mrim.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
