
if(SCIM_FOUND)
    include_directories(${SCIM_INCLUDE_DIR})
    set(scim_kconfig_SRCS scim_kconfig_config.cpp)
    add_library(scim-kconfig MODULE ${scim_kconfig_SRCS})
    target_link_libraries(scim-kconfig ${KDE4_KDECORE_LIBS} ${SCIM_LIBRARIES})
    set_target_properties(scim-kconfig PROPERTIES OUTPUT_NAME kconfig4 PREFIX "")
    install(TARGETS scim-kconfig DESTINATION ${SCIM_MODULE_DIR}/Config)
endif(SCIM_FOUND)
