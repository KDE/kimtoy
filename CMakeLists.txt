project(kimtoy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(KDE4 REQUIRED 4.5)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra)

########################## kimtoy ##########################

set(kimtoy_SRCS
    envsettings.cpp
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
)

install(TARGETS kimtoy ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kimtoy.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(PROGRAMS kimtoy.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES kimtoy.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
# install(FILES org.kde.impanel.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_subdirectory(thumbnailer)
# add_subdirectory(strigi-analyzer)
# add_subdirectory(po)

########################## ibus glue ##########################
add_subdirectory(ibus-kconfig)
add_subdirectory(ibus-panel)
########################## scim glue ##########################
add_subdirectory(scim-panel)
