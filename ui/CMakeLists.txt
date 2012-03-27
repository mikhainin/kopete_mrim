

set(mrimui_SRCS
    contactinfo.cpp
)

kde4_add_ui_files(mrimui_SRCS
    contactinfo.ui
)

include_directories(..)


qt4_automoc( QT_MOC_mrimui_SRCS ${mrimui_SRCS} )
# qt4_wrap_ui( UI_HEADERS ${FORMS} )
# qt4_wrap_cpp( MOC_SOURCES ${QT_HEADERS} )


add_library(ui ${mrimui_SRCS} ${QT_MOC_mrimui_SRCS} )
set_target_properties(ui PROPERTIES COMPILE_FLAGS -fPIC)
