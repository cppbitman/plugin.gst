function ( get_host_arch result )
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL AMD64 
      OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64 )
        set(${result} x64 PARENT_SCOPE)
    else()
        set(${result} x86 PARENT_SCOPE)
    endif()
endfunction()

function ( get_host_platform result )
    if( MSVC )
        set(${result} win PARENT_SCOPE)
    else()
        set(${result} linux PARENT_SCOPE)
    endif()
endfunction()

function ( get_project_name basename result )
    if( MSVC )
        set(${result} lib${basename} PARENT_SCOPE)
    else()
        set(${result} ${basename} PARENT_SCOPE)
    endif()
endfunction()

function ( get_install_dir result )
    get_host_arch( _arch )
    get_host_platform( _platform )
    set(_prefix ${CMAKE_HOME_DIRECTORY}/bin)
    if( MSVC )
        set(${result} ${_prefix}/${_platform}/${_arch} PARENT_SCOPE)
    else()
        set(${result} $ENV{GST_PLUGIN_PATH_1_0_CUSTOM} PARENT_SCOPE)
    endif()
endfunction()

function ( get_plugin_sources result )
    file(GLOB_RECURSE _sources ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.[ch]
      ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cc)
    conan_project_group(_sources ${CMAKE_CURRENT_SOURCE_DIR})
    
    foreach( f ${_sources} )
      message( ${f} )
    endforeach()

    set(${result} ${_sources} PARENT_SCOPE)
endfunction()

include(CMakeParseArguments)
function(add_plugin_library)
    cmake_parse_arguments(LIB "" "PROJECT" "DEFINITIONS;DEPENDENCIES;INCLUDE;SOURCES" ${ARGN})
    
    foreach( d ${LIB_DEFINITIONS} )
        message("definition: -D${d}")
        add_definitions( -D${d} )
    endforeach()

    foreach( i ${LIB_INCLUDE})
        message("include: ${i}")
        include_directories( ${i} )
    endforeach()
    
    message("project: ${LIB_PROJECT}")
    message("sources: ${LIB_SOURCES}")
    add_library(${LIB_PROJECT} SHARED ${LIB_SOURCES})

    foreach( d ${LIB_DEPENDENCIES})
        target_link_libraries( ${LIB_PROJECT}  ${d})
    endforeach()
endfunction()
    
function(make_library)
    cmake_parse_arguments(LIB "" "PROJECT" "DEFINITIONS;DEPENDENCIES;INCLUDE;SOURCES" ${ARGN})
    
    foreach( d ${LIB_DEFINITIONS} )
        message("definition: -D${d}")
        add_definitions( -D${d} )
    endforeach()
    add_definitions( -DGST_USE_UNSTABLE_API -DHAVE_OPENSSL )

    foreach( i ${LIB_INCLUDE})
        message("include: ${i}")
        include_directories( ${i} )
    endforeach()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gst-libs)
    
    message("project: ${LIB_PROJECT}")
    message("sources: ${LIB_SOURCES}")
    add_library(${LIB_PROJECT} SHARED ${LIB_SOURCES})
    
    foreach( d ${LIB_DEPENDENCIES})
        message("dependency: ${d}")
        target_link_libraries( ${LIB_PROJECT}  ${d})
    endforeach()
    target_link_libraries(${project_name} ${GST_MODULES_LIBRARIES} )
    target_link_libraries(${project_name} ${OPENSSL_MODULES_LIBRARIES} )
endfunction()



function( install_library project_name install_dir)
    install(TARGETS ${project_name} RUNTIME DESTINATION ${install_dir}
                                    LIBRARY DESTINATION ${install_dir})
endfunction()
