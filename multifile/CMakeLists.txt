cmake_minimum_required(VERSION 2.8)

set(instal_prefix ${CMAKE_HOME_DIRECTORY}/bin)
set(project_basename cusmultifile)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL AMD64 OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64 )
	set(arch x64)
else()
	set(arch x86)
endif()
if( MSVC )
    set(libname lib${project_basename})
    set(plat win )
    set(instal_dir ${instal_prefix}/${plat}/${arch})
else()
    set(libname ${project_basename})
    set(plat linux )
    set(instal_dir $ENV{GST_PLUGIN_PATH_1_0_CUSTOM})
endif()

project(${libname})
string(TOUPPER ${libname} libname_upper)
file(GLOB_RECURSE ${libname_upper}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.[ch]
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cc)
conan_project_group(${libname_upper}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR})

foreach( filename ${${libname_upper}_SOURCES} )
    message(${filename})
endforeach()

add_definitions( -DGETTEXT_PACKAGE )
add_definitions( -DGST_USE_UNSTABLE_API -DHAVE_OPENSSL  )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gst-libs)

add_library(${libname} SHARED ${${libname_upper}_SOURCES})
target_link_libraries(${libname} ${GST_MODULES_LIBRARIES} )
target_link_libraries(${libname} ${OPENSSL_MODULES_LIBRARIES} )
target_link_libraries(${libname} gio-2.0 )

install(TARGETS ${libname} RUNTIME DESTINATION ${instal_dir}
                           LIBRARY DESTINATION ${instal_dir})

                           