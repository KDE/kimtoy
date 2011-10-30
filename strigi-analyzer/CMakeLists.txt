find_package(Strigi REQUIRED)
include_directories(${STRIGI_INCLUDE_DIR})

set(ssfendanalyzer_SRCS ssfendanalyzer.cpp)
add_library(ssf MODULE ${ssfendanalyzer_SRCS})
target_link_libraries(ssf ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})
set_target_properties(ssf PROPERTIES PREFIX strigiea_)
install(TARGETS ssf LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

set(fskinendanalyzer_SRCS fskinendanalyzer.cpp)
add_library(fskin MODULE ${fskinendanalyzer_SRCS})
target_link_libraries(fskin ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})
set_target_properties(fskin PROPERTIES PREFIX strigiea_)
install(TARGETS fskin LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
