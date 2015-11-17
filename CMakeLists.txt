project(kimtoy)

cmake_minimum_required(VERSION 2.8.12)

set(KDE_INSTALL_USE_QT_SYS_PATHS ON)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckIncludeFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Qt5 5.3.0 CONFIG REQUIRED
    Core
    DBus
    Widgets
    X11Extras
)

find_package(KF5 5.2.0 REQUIRED
    Archive
    ConfigWidgets
    CoreAddons
    DBusAddons
    I18n
    IconThemes
    KIO
    NewStuff
    Notifications
    Plasma
    WidgetsAddons
    WindowSystem
)

include_directories(${X11_X11_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra)

set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

find_package(GLIB2)
set_package_properties(GLIB2 PROPERTIES DESCRIPTION "glib2" URL "http://www.gtk.org" PURPOSE "ibus support" TYPE OPTIONAL)

find_package(GIO)
set_package_properties(GIO PROPERTIES DESCRIPTION "gio" URL "http://www.gtk.org" PURPOSE "ibus support" TYPE OPTIONAL)

find_package(GObject)
set_package_properties(GObject PROPERTIES DESCRIPTION "gobject" URL "http://www.gtk.org" PURPOSE "ibus support" TYPE OPTIONAL)

find_package(IBus)
set_package_properties(IBus PROPERTIES DESCRIPTION "ibus" URL "http://code.google.com/p/ibus" PURPOSE "ibus support" TYPE OPTIONAL)

find_package(Pthread)
set_package_properties(Pthread PROPERTIES DESCRIPTION "pthread" PURPOSE "scim support" TYPE OPTIONAL)

find_package(DBusCXX)
set_package_properties(DBusCXX PROPERTIES DESCRIPTION "dbus-c++" URL "http://dbus-cplusplus.sourceforge.net" PURPOSE "scim support" TYPE OPTIONAL)

find_package(SCIM)
set_package_properties(SCIM PROPERTIES DESCRIPTION "scim" URL "http://www.scim-im.org" PURPOSE "scim support" TYPE OPTIONAL)

find_package(Strigi)
set_package_properties(Strigi PROPERTIES DESCRIPTION "strigi" URL "http://www.vandenoever.info/software/strigi" PURPOSE "strigi analyzer" TYPE OPTIONAL)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

############# check if libpng has apng support #############

# macro_optional_find_package(PNG)
find_package(PNG)
set_package_properties(PNG PROPERTIES DESCRIPTION "libpng-apng" URL "http://libpng-apng.sf.net" PURPOSE "animated png support from system libpng" TYPE OPTIONAL)

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

# macro_log_feature(PNG_HAS_APNG_SUPPORT "libpng-apng" "animated png support from system libpng" "http://libpng-apng.sf.net")

###################### ibus arguments ######################
### NOTE TODO remove following definition when ibus is able to run registered component
if(IBUS_FOUND)
    add_definitions(-DIBUS_LIBEXEC_DIR="\\\"${IBUS_LIBEXEC_DIR}\\\"")
else(IBUS_FOUND)
    add_definitions(-DIBUS_LIBEXEC_DIR="\\\"${CMAKE_INSTALL_PREFIX}/libexec\\\"")
endif(IBUS_FOUND)

########################## kimtoy ##########################

add_definitions(
#     -DQT_USE_QSTRINGBUILDER
#     -DQT_NO_CAST_TO_ASCII
#     -DQT_STRICT_ITERATORS
#     -DQT_NO_URL_CAST_FROM_STRING
#     -DQT_NO_CAST_FROM_BYTEARRAY
#     -DQT_NO_SIGNALS_SLOTS_KEYWORDS
#     -DQT_USE_FAST_OPERATOR_PLUS
)

set(kimtoy_SRCS
    animator.cpp
    envsettings.cpp
    filtermenu.cpp
    impanel.cpp
    impanelagent.cpp
    impanelagent_p.cpp
    inputmethods.cpp
    kimtoy.cpp
    kssf.cpp
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

ki18n_wrap_ui(kimtoy_SRCS
    appearance.ui
    inputmethod.ui
    theme.ui
    performance.ui
)

qt5_add_dbus_adaptor(kimtoy_SRCS
    org.kde.impanel.xml
    impanel.h
    IMPanel
    impaneladaptor
    IMPanelAdaptor
)

kconfig_add_kcfg_files(kimtoy_SRCS kimtoysettings.kcfgc)

add_executable(kimtoy ${kimtoy_SRCS})

target_link_libraries(kimtoy
    Qt5::Widgets
    Qt5::X11Extras
    KF5::Archive
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::KIONTLM
    KF5::NewStuff
    KF5::Notifications
    KF5::Plasma
    KF5::WidgetsAddons
    KF5::WindowSystem

    ${OPENSSL_LIBRARIES}
    ${X11_X11_LIB}
    X11
)

install(TARGETS kimtoy ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kimtoy.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(PROGRAMS kimtoy.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES kimtoy.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
# install(FILES org.kde.impanel.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_subdirectory(thumbnailer)
# add_subdirectory(strigi-analyzer)
# add_subdirectory(po)

######################### apng plugin #########################
add_subdirectory(kimgio-apng)
########################## ibus glue ##########################
# add_subdirectory(ibus-kconfig)
# add_subdirectory(ibus-panel)
########################## scim glue ##########################
# add_subdirectory(scim-kconfig)
# add_subdirectory(scim-panel)

# macro_display_feature_log()
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
