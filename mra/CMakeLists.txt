
#set(libmra_SRCS
#    mraconnection.cpp
#    mradata.cpp
#    mramsg.cpp
#    mraprotocol.cpp
#    mracontactlist.cpp
#)

set(kopete_mrim_SRCS
    ${kopete_mrim_SRCS}
    mra/mraconnection.cpp
    mra/mradata.cpp
    mra/mraprotocol.cpp
    mra/mracontactlist.cpp
    mra/mracontactlistentry.cpp
    mra/mraofflinemessage.cpp
    PARENT_SCOPE
)

#add_library(mra ${libmra_SRCS})
