# - Try to find the SCIM
# Once done this will define
#
#  SCIM_FOUND - system has SCIM
#  SCIM_INCLUDE_DIR - The include directory to use for the SCIM headers
#  SCIM_LIBRARIES - Link these to use SCIM
#  SCIM_DEFINITIONS - Compiler switches required for using SCIM
#  SCIM_ICON_DIR - The directory to use for SCIM icons
#  SCIM_MODULE_DIR - The directory to use for SCIM modules
#  SCIM_LIBEXEC_DIR - The directory to use for SCIM executables

# Copyright (c) 2009 Wang Hoi, <zeahot.hoi@gmail.com>
# Copyright (c) 2011 Ni Hui, <shuizhuyuanluo@126.com>
# Based on Laurent Montel's FindFontConfig.cmake, <montel@kde.org> 
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)

   # in cache already
   set(SCIM_FOUND TRUE)

else (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)

   if (NOT WIN32)
      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      find_package(PkgConfig)
      pkg_check_modules(PC_SCIM QUIET scim)

      if (NOT SCIM_ICON_DIR)
         _pkgconfig_invoke(scim PC_SCIM ICON_DIR "" --variable=icondir)
         set(SCIM_ICON_DIR ${PC_SCIM_ICON_DIR} CACHE INTERNAL "")
      endif (NOT SCIM_ICON_DIR)

      if (NOT SCIM_MODULE_DIR)
         _pkgconfig_invoke(scim PC_SCIM MODULE_DIR "" --variable=moduledir)
         set(SCIM_MODULE_DIR ${PC_SCIM_MODULE_DIR} CACHE INTERNAL "")
      endif (NOT SCIM_MODULE_DIR)

      if (NOT SCIM_LIBEXEC_DIR)
         set(SCIM_LIBEXEC_DIR ${PC_SCIM_LIBDIR}/scim-1.0 CACHE INTERNAL "")
      endif (NOT SCIM_LIBEXEC_DIR)

      set(SCIM_DEFINITIONS ${PC_SCIM_CFLAGS_OTHER})
   endif (NOT WIN32)

   find_path(SCIM_INCLUDE_DIR scim.h
      PATHS
      ${PC_SCIM_INCLUDEDIR}
      ${PC_SCIM_INCLUDE_DIRS}
      )

   find_library(SCIM_LIBRARIES NAMES scim scim-1.0
      PATHS
      ${PC_SCIM_LIBDIR}
      ${PC_SCIM_LIBRARY_DIRS}
      )

   include(FindPackageHandleStandardArgs)
   FIND_PACKAGE_HANDLE_STANDARD_ARGS(SCIM DEFAULT_MSG SCIM_LIBRARIES SCIM_INCLUDE_DIR )

   mark_as_advanced(SCIM_LIBRARIES SCIM_INCLUDE_DIR)

endif (SCIM_LIBRARIES AND SCIM_INCLUDE_DIR)
