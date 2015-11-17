
########## ssf thumbnailer ##########
set(ssfthumbnail_SRCS ssfcreator.cpp ../kssf.cpp)

add_library(ssfthumbnail MODULE ${ssfthumbnail_SRCS})
target_link_libraries(ssfthumbnail
    Qt5::Gui
    KF5::Archive
    KF5::KIOWidgets
    ${OPENSSL_LIBRARIES}
)

install(TARGETS ssfthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ssfthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ssf.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

########## fskin thumbnailer ##########
set(fskinthumbnail_SRCS fskincreator.cpp)

add_library(fskinthumbnail MODULE ${fskinthumbnail_SRCS})
target_link_libraries(fskinthumbnail
    Qt5::Gui
    KF5::Archive
    KF5::KIOWidgets
)

install(TARGETS fskinthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES fskinthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES fskin.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
