# cmake -RCMAKE_INSTALL_PREFIX=/ -DEXEC_INSTALL_PREFIX=/usr -DSHARE_INSTALL_PREFIX=/usr/share/ -DCMAKE_BUILD_TYPE=DebugFull ..
cmake_minimum_required(VERSION 2.6)

project(kopete_mrim)

set(KF5_VERSION "5.11.0")
set(REQUIRED_QT_VERSION "5.2.0")


#ECM modules configurations
find_package(ECM 1.7 REQUIRED NO_MODULE)

# find_package(KDE4 REQUIRED)
# find_package(Kopete REQUIRED)
# find_package(KdepimLibs REQUIRED)
# include (KDE4Defaults)
# include (MacroLibrary)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_MODULE_DIR} ${ECM_KDE_MODULE_DIR})

find_package(Phonon4Qt5 REQUIRED NO_MODULE)
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Gui Gui)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Xml Sql Test)
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS
	CoreAddons
	Config
	# I18n
	KDELibs4Support
)
set(CMAKE_AUTOMOC ON)

# find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
# find_package(KF5CoreAddons)
find_package(KF5 COMPONENTS IdentityManagement)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

include(ECMAddTests)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include_directories(${PHONON_INCLUDES})


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

include_directories("/usr/local/include/kopete")

if (Kopete_FOUND)
    message(STATUS "Kopete include dir is: ${KOPETE_INCLUDE_DIR}")
    include_directories(${KOPETE_INCLUDE_DIR})
    include_directories("${KOPETE_INCLUDE_DIR}/kopete")
else(Kopete_FOUND)
    message(ERROR "Kopete devel libraries do not found")
endif(Kopete_FOUND)

set(kopete_mrim_SRCS
    addcontacttask.cpp
    mrimprotocol.cpp
    mrimcontact.cpp
    mrimaccount.cpp
    mrimchatsession.cpp
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
    mra/mraconferencesettings.cpp

    ui/contactinfo.cpp
    ui/mrimeditaccountwidget.cpp
    ui/mrimaddcontactpage.cpp
    ui/createconferencedialog.cpp
)

ki18n_wrap_ui(kopete_mrim_SRCS
    mrimaddui.ui

    ui/mrimaccountpreferences.ui
    ui/contactinfo.ui
    ui/createconferencedialog.ui
)


add_library(kopete_mrim SHARED ${kopete_mrim_SRCS} ${libmra_files} )

add_subdirectory (icons)


target_link_libraries(kopete_mrim 
    kopeteidentity
    kopete
#    KD5::PIM
)

target_link_libraries(kopete_mrim 
    KF5::ConfigCore 
    KF5::ConfigGui 
    KF5::CoreAddons
    KF5::KDELibs4Support
    KF5::IdentityManagement
    )

# ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDE4_KDNSSD_LIBS} kopete)

install( TARGETS kopete_mrim  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kopete_mrim.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopetemrimchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_mrim)
