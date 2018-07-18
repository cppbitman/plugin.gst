cmake_minimum_required(VERSION 2.8)
project(libgstplugin)


enable_testing()
include(${CMAKE_CURRENT_SOURCE_DIR}/conan.cmake)
conan_find_pkgconfig( 0.29.1 )
conan_compiler_flags()

pkg_check_modules(GST_MODULES REQUIRED
                  gstreamer-1.0>=1.14.0
                  gstreamer-base-1.0>=1.14.0
                  gstreamer-tag-1.0>=1.14.0
                  gstreamer-video-1.0>=1.14.0
                  gstreamer-pbutils-1.0>=1.14.0 )

include_directories(${GST_MODULES_INCLUDE_DIRS})
link_directories   (${GST_MODULES_LIBRARY_DIRS})

pkg_check_modules(OPENSSL_MODULES REQUIRED
                  libcrypto )

include_directories(${OPENSSL_MODULES_INCLUDE_DIRS})
link_directories   (${OPENSSL_MODULES_LIBRARY_DIRS})

include(${CMAKE_HOME_DIRECTORY}/build.cmake)

add_subdirectory( hls )
add_subdirectory( multifile )
add_subdirectory( memorysink )
add_subdirectory( filesink )