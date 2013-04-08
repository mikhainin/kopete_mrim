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
    addcontacttask.cpp
    mrimprotocol.cpp
    mrimcontact.cpp
    mrimaccount.cpp
    debug.cpp
    version.cpp
    filetransfertask.cpp

    mra/mraconnection.cpp
    mra/mradata.cpp
    mra/mraprotocol.cpp
    mra/mracontactlist.cpp
    mra/mracontactlistentry.cpp
    mra/mraofflinemessage.cpp
    mra/mraavatarloader.cpp
    mra/mracontactinfo.cpp
    mra/mraprotocolv123.cpp
    mra/transferrequestinfo.cpp
    mra/transfermanager.cpp

    ui/contactinfo.cpp
    ui/mrimeditaccountwidget.cpp
    ui/mrimaddcontactpage.cpp
)

kde4_add_ui_files(kopete_mrim_SRCS
    mrimaddui.ui

    ui/mrimaccountpreferences.ui
    ui/contactinfo.ui
)


add_definitions(-DKDE_DEFAULT_DEBUG_AREA=89326)
add_definitions(-DKOPETE_MRIM_VERSION=0.2.0)


kde4_add_plugin(kopete_mrim ${kopete_mrim_SRCS} ${libmra_files} )

add_subdirectory (icons)

target_link_libraries(kopete_mrim ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDE4_KDNSSD_LIBS} kopete)

install( TARGETS kopete_mrim  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kopete_mrim.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
