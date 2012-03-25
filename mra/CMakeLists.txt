
#set(libmra_SRCS
#    mraconnection.cpp
#    mradata.cpp
#    mramsg.cpp
#    mraprotocol.cpp
#    mracontactlist.cpp
#)

set(libmra_SRCS
    mraconnection.cpp
    mradata.cpp
    mraprotocol.cpp
    mracontactlist.cpp
    mracontactlistentry.cpp
    mraofflinemessage.cpp
)


qt4_automoc( QT_MOC_libmra_SRCS ${libmra_SRCS} )
# qt4_wrap_ui( UI_HEADERS ${FORMS} )
# qt4_wrap_cpp( MOC_SOURCES ${QT_HEADERS} )


add_library(mra ${libmra_SRCS} ${QT_MOC_libmra_SRCS} )
set_target_properties(mra PROPERTIES COMPILE_FLAGS -fPIC)
