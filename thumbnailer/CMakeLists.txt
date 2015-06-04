
########## ssf thumbnailer ##########
set(ssfthumbnail_SRCS ssfcreator.cpp ../kssf.cpp)

kde4_add_plugin(ssfthumbnail ${ssfthumbnail_SRCS})
target_link_libraries(ssfthumbnail
    ${KDE4_KIO_LIBS}
    ${OPENSSL_LIBRARIES}
)

install(TARGETS ssfthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ssfthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ssf.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

########## fskin thumbnailer ##########
set(fskinthumbnail_SRCS fskincreator.cpp)

kde4_add_plugin(fskinthumbnail ${fskinthumbnail_SRCS})
target_link_libraries(fskinthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS fskinthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES fskinthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES fskin.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
