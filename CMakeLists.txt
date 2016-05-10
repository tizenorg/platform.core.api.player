
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(fw_name "capi-media-player")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX /usr)
SET(PREFIX ${CMAKE_INSTALL_PREFIX})

IF (TIZEN_WEARABLE)
SET(INC_DIR include/wearable)
ELSE (TIZEN_WEARABLE)
SET(INC_DIR include/common)
ENDIF (TIZEN_WEARABLE)

SET(INC_COM_DIR include)

INCLUDE_DIRECTORIES(${INC_DIR}, ${INC_COM_DIR})
IF (TIZEN_MOBILE)
SET(dependents "dlog glib-2.0 libtbm capi-media-tool capi-media-sound-manager capi-base-common mused mm-common tizen-extension-client evas ecore elementary mm-evas-renderer")
ELSE (TIZEN_MOBILE)
SET(dependents "dlog glib-2.0 libtbm capi-media-tool capi-media-sound-manager capi-base-common mused mm-common tizen-extension-client evas ecore elementary")
ENDIF (TIZEN_MOBILE)
SET(pc_dependents "libtbm capi-media-tool capi-base-common capi-media-sound-manager")

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${dependents})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

pkg_check_modules(MMSVC_PLAYER mmsvc-player)
FOREACH(mmsvcflag ${MMSVC_PLAYER_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${mmsvcflag}")
ENDFOREACH(mmsvcflag)

SET(CMAKE_C_FLAGS "-I./${INC_DIR} ${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Werror -Wno-deprecated -Wno-deprecated-declarations")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

IF(WAYLAND_SUPPORT)
 ADD_DEFINITIONS("-DHAVE_WAYLAND")
ENDIF(WAYLAND_SUPPORT)
IF(X11_SUPPORT)
 ADD_DEFINITIONS("-DHAVE_X11")
ENDIF(X11_SUPPORT)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DTIZEN_DEBUG")

IF (TIZEN_MOBILE)
ADD_DEFINITIONS("-DTIZEN_MOBILE")
ENDIF(TIZEN_MOBILE)

IF (TIZEN_TV)
ADD_DEFINITIONS("-DTIZEN_TV")
ENDIF(TIZEN_TV)

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=${LIB_INSTALL_DIR}")

aux_source_directory(src SOURCES)
ADD_LIBRARY(${fw_name} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${fw_name} ${${fw_name}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_name}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_name} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(
        DIRECTORY ${INC_DIR}/ DESTINATION include/media
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "${INC_DIR}/*.h"
        )

SET(PC_NAME ${fw_name})
SET(PC_REQUIRED ${pc_dependents})
SET(PC_LDFLAGS -l${fw_name})
SET(PC_CFLAGS -I\${includedir}/media)

CONFIGURE_FILE(
    ${fw_name}.pc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

ADD_SUBDIRECTORY(test)

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean
        COMMENT "distribution clean"
        COMMAND find
        ARGS    .
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)

