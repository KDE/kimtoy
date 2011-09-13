########## ssf thumbnailer ##########
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

set(ssfthumbnail_SRCS ssfcreator.cpp)

kde4_add_plugin(ssfthumbnail ${ssfthumbnail_SRCS})
target_link_libraries(ssfthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS ssfthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ssfthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ssf.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
