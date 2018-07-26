cmake_minimum_required(VERSION 2.8)

set(libbasename memorysink)
get_project_name( ${libbasename} project_name  )
project(${project_name})

get_plugin_sources( sources )
make_library(
    PROJECT ${project_name}
    SOURCES ${sources}
    DEFINITIONS PACKAGE_NAME
    )

get_install_dir ( install_dir )
install_library(${project_name} ${install_dir})                        
