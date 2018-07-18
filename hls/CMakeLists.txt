cmake_minimum_required(VERSION 2.8)

set(libbasename cushls)
get_project_name( ${libbasename} project_name  )
project(${project_name})

get_plugin_sources( sources )
make_library(
    PROJECT ${project_name}
    SOURCES ${sources}
    DEPENDENCIES  gstadaptivedemux-1.0
    )

get_install_dir ( install_dir )
install_library(${project_name} ${install_dir})                        