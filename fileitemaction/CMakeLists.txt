
kcoreaddons_add_plugin(installskinfileitemaction
    SOURCES installskinfileitemaction.cpp
    JSON installskinfileitemaction.json
    INSTALL_NAMESPACE "kf5/kfileitemaction"
)

target_link_libraries(installskinfileitemaction
    KF5::I18n
    KF5::KIOWidgets
)
