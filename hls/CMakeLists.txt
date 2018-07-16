cmake_minimum_required(VERSION 2.8)
project(libcushls)

file(GLOB_RECURSE LIBCUSHLS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
conan_project_group(LIBCUSHLS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR})

foreach( filename ${LIBCUSHLS_SOURCES} )
    message(${filename})
endforeach()

set(libname libcushls)

add_definitions( -DGST_USE_UNSTABLE_API -DHAVE_OPENSSL  )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gst-libs)
add_library(${libname} SHARED ${LIBCUSHLS_SOURCES})
target_link_libraries(${libname} ${GST_MODULES_LIBRARIES} )
target_link_libraries(${libname} ${OPENSSL_MODULES_LIBRARIES} )


set(instal_prefix ${CMAKE_HOME_DIRECTORY}/bin)
if( MSVC )
	set(plat win )
else()
	set(plat linux )
endif()
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL AMD64 OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64 )
	set(arch x64)
else()
	set(arch x86)
endif()
set(instal_dir ${instal_prefix}/${plat}/${arch})
install(TARGETS ${libname} RUNTIME DESTINATION ${instal_dir}
                           LIBRARY DESTINATION ${instal_dir})

                           