
if(KF5FileMetaData_FOUND)
    add_library(kfilemetadata_ssfextractor MODULE ssfextractor.cpp ../kssf.cpp)
    target_link_libraries(kfilemetadata_ssfextractor KF5::Archive KF5::FileMetaData ${OPENSSL_LIBRARIES})
    install(TARGETS kfilemetadata_ssfextractor DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kfilemetadata)

    add_library(kfilemetadata_fskinextractor MODULE fskinextractor.cpp)
    target_link_libraries(kfilemetadata_fskinextractor KF5::Archive KF5::FileMetaData)
    install(TARGETS kfilemetadata_fskinextractor DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kfilemetadata)
endif(KF5FileMetaData_FOUND)
