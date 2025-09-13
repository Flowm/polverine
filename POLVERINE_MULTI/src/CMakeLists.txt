# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# Find all compressed HTML files
file(GLOB web_files "../web/*.html.gz")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "."
    EMBED_FILES ${web_files}
)
