project(kimtoy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(KDE4 REQUIRED 4.5)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroLogFeature)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
include_directories(${X11_X11_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra)

set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "glib2" "ibus support" "http://www.gtk.org")

macro_optional_find_package(GIO)
macro_log_feature(GIO_FOUND "gio" "ibus support" "http://www.gtk.org")

macro_optional_find_package(GObject)
macro_log_feature(GOBJECT_FOUND "gobject" "ibus support" "http://www.gtk.org")

macro_optional_find_package(IBus)
macro_log_feature(IBUS_FOUND "ibus" "ibus support" "http://code.google.com/p/ibus")

macro_optional_find_package(DBusCXX)
macro_log_feature(DBUSCXX_FOUND "dbus-c++" "scim support" "http://dbus-cplusplus.sourceforge.net")

macro_optional_find_package(SCIM)
macro_log_feature(SCIM_FOUND "scim" "scim support" "http://www.scim-im.org")

macro_optional_find_package(Strigi)
macro_log_feature(STRIGI_FOUND "strigi" "strigi analyzer" "http://www.vandenoever.info/software/strigi")

############# check if libpng has apng support #############

macro_optional_find_package(PNG)

if(PNG_FOUND)
    include(CheckCSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES ${PNG_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${PNG_LIBRARY})
    check_c_source_compiles(
"
#include <png.h>
int main()
{
    (void)png_get_num_frames((png_structp)0, (png_infop)0);
    return 0;
}
"
        PNG_HAS_APNG_SUPPORT
    )
else(PNG_FOUND)
    set(PNG_HAS_APNG_SUPPORT 0)
endif(PNG_FOUND)

if(${PNG_HAS_APNG_SUPPORT})
    message("-- Using system PNG")
else(${PNG_HAS_APNG_SUPPORT})
    message("-- Using internal PNG")
endif(${PNG_HAS_APNG_SUPPORT})

macro_log_feature(PNG_HAS_APNG_SUPPORT "libpng-apng" "animated png support from system libpng" "http://libpng-apng.sf.net")

###################### ibus arguments ######################
### NOTE TODO remove following definition when ibus is able to run registered component
if(IBUS_FOUND)
    add_definitions(-DIBUS_LIBEXEC_DIR="\\\"${IBUS_LIBEXEC_DIR}\\\"")
else(IBUS_FOUND)
    add_definitions(-DIBUS_LIBEXEC_DIR="\\\"${CMAKE_INSTALL_PREFIX}/libexec\\\"")
endif(IBUS_FOUND)

########################## kimtoy ##########################

set(kimtoy_SRCS
    animator.cpp
    envsettings.cpp
    filtermenu.cpp
    impanel.cpp
    impanelagent.cpp
    impanelagent_p.cpp
    inputmethods.cpp
    kimtoy.cpp
    main.cpp
    preeditbar.cpp
    propertywidget.cpp
    skinpixmap.cpp
    statusbar.cpp
    statusbarlayout.cpp
    theme.cpp
    themelistview.cpp
    themelistmodel.cpp
    themer.cpp
    themer_fcitx.cpp
    themer_none.cpp
    themer_plasma.cpp
    themer_sogou.cpp
    themeragent.cpp
)

kde4_add_ui_files(kimtoy_SRCS
    appearance.ui
    inputmethod.ui
    theme.ui
    performance.ui
)

qt4_add_dbus_adaptor(kimtoy_SRCS
    org.kde.impanel.xml
    impanel.h
    IMPanel
    impaneladaptor
    IMPanelAdaptor
)

kde4_add_kcfg_files(kimtoy_SRCS kimtoysettings.kcfgc)

kde4_add_executable(kimtoy ${kimtoy_SRCS})

target_link_libraries(kimtoy
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KNEWSTUFF3_LIBS}
    ${X11_X11_LIB}
)

install(TARGETS kimtoy ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kimtoy.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(PROGRAMS kimtoy.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES kimtoy.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
# install(FILES org.kde.impanel.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_subdirectory(thumbnailer)
add_subdirectory(strigi-analyzer)
# add_subdirectory(po)

######################### apng plugin #########################
add_subdirectory(kimgio-apng)
########################## ibus glue ##########################
add_subdirectory(ibus-kconfig)
add_subdirectory(ibus-panel)
########################## scim glue ##########################
add_subdirectory(scim-kconfig)
add_subdirectory(scim-panel)

macro_display_feature_log()
