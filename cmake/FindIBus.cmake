# - Try to find IBUS
# Once done this will define
#
#  IBUS_FOUND - system has IBUS
#  IBUS_INCLUDE_DIR - The include directory to use for the fontconfig headers
#  IBUS_LIBRARIES - Link these to use IBUS
#  IBUS_DEFINITIONS - Compiler switches required for using IBUS
#  IBUS_COMPONENT_DIR - The directory to use for ibus components
#  IBUS_LIBEXEC_DIR - The directory to use for ibus executables

# Copyright (c) 2011 Ni Hui, <shuizhuyuanluo@126.com>
# Based on Laurent Montel's FindFontConfig.cmake, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)

   # in cache already
   set(IBUS_FOUND TRUE)

else (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)

   if (NOT WIN32)
      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      find_package(PkgConfig)
      pkg_check_modules(PC_IBUS QUIET ibus-1.0)

      if (NOT IBUS_COMPONENT_DIR)
         _pkgconfig_invoke(ibus-1.0 PC_IBUS PKGDATA_DIR "" --variable=pkgdatadir)
         set(IBUS_COMPONENT_DIR ${PC_IBUS_PKGDATA_DIR}/component CACHE INTERNAL "")
      endif (NOT IBUS_COMPONENT_DIR)

      if (NOT IBUS_LIBEXEC_DIR)
         _pkgconfig_invoke(ibus-1.0 PC_IBUS EXEC_PREFIX "" --variable=exec_prefix)
         set(IBUS_LIBEXEC_DIR ${PC_IBUS_EXEC_PREFIX}/libexec CACHE INTERNAL "")
      endif (NOT IBUS_LIBEXEC_DIR)

      set(IBUS_DEFINITIONS ${PC_IBUS_CFLAGS_OTHER})
   endif (NOT WIN32)

   find_path(IBUS_INCLUDE_DIR ibus.h
      PREFIX ibus-1.0
      PATHS
      ${PC_IBUS_INCLUDEDIR}
      ${PC_IBUS_INCLUDE_DIRS}
      )

   find_library(IBUS_LIBRARIES NAMES ibus-1.0 ibus
      PATHS
      ${PC_IBUS_LIBDIR}
      ${PC_IBUS_LIBRARY_DIRS}
      )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(IBUS DEFAULT_MSG IBUS_LIBRARIES IBUS_INCLUDE_DIR)

   mark_as_advanced(IBUS_LIBRARIES IBUS_INCLUDE_DIR)
endif (IBUS_LIBRARIES AND IBUS_INCLUDE_DIR)
