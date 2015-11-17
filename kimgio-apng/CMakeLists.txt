
set(kimg_apng_LIB_SRCS apng.cpp)

if(NOT PNG_HAS_APNG_SUPPORT)
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIR})

    set(libpng_sources
        libpng-apng/png.c
        libpng-apng/pngerror.c
        libpng-apng/pngget.c
        libpng-apng/pngmem.c
        libpng-apng/pngpread.c
        libpng-apng/pngread.c
        libpng-apng/pngrio.c
        libpng-apng/pngrtran.c
        libpng-apng/pngrutil.c
        libpng-apng/pngset.c
        libpng-apng/pngtrans.c
        libpng-apng/pngwio.c
        libpng-apng/pngwrite.c
        libpng-apng/pngwtran.c
        libpng-apng/pngwutil.c
    )

    add_definitions(-DPNG_CONFIGURE_LIBPNG)
    add_definitions(-DPNG_NO_CONSOLE_IO)
    add_definitions(-DPNG_NO_STDIO)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng-apng/pnglibconf.h.prebuilt
                ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    add_definitions(-DUSE_INTERNAL_PNG=1)

    set(kimg_apng_LIB_SRCS ${kimg_apng_LIB_SRCS} ${libpng_sources})
endif(NOT PNG_HAS_APNG_SUPPORT)

add_library(kimg_apng MODULE ${kimg_apng_LIB_SRCS})
target_link_libraries(kimg_apng Qt5::Gui)

if(PNG_HAS_APNG_SUPPORT)
    target_link_libraries(kimg_apng ${PNG_LIBRARY})
else(PNG_HAS_APNG_SUPPORT)
    target_link_libraries(kimg_apng ${ZLIB_LIBRARY})
endif(PNG_HAS_APNG_SUPPORT)

message("KDE_INSTALL_QTPLUGINDIR = ${KDE_INSTALL_QTPLUGINDIR}")
install(TARGETS kimg_apng DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats/)

install(FILES apng.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

