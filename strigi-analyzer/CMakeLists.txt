find_package(Strigi REQUIRED)
include_directories(${STRIGI_INCLUDE_DIR})

set(ssfendanalyzer_SRCS ssfendanalyzer.cpp)
add_library(strigi-ssf MODULE ${ssfendanalyzer_SRCS})
target_link_libraries(strigi-ssf ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})
set_target_properties(strigi-ssf PROPERTIES OUTPUT_NAME ssf PREFIX strigiea_)
install(TARGETS strigi-ssf LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

set(fskinendanalyzer_SRCS fskinendanalyzer.cpp)
add_library(strigi-fskin MODULE ${fskinendanalyzer_SRCS})
target_link_libraries(strigi-fskin ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})
set_target_properties(strigi-fskin PROPERTIES OUTPUT_NAME fskin PREFIX strigiea_)
install(TARGETS strigi-fskin LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
