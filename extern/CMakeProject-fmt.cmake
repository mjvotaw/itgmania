set(FMT_TEST OFF CACHE INTERNAL "Don't build fmt tests")
set(FMT_DOC OFF CACHE INTERNAL "Don't build fmt docs")
set(FMT_INSTALL OFF CACHE INTERNAL "Don't install fmt")

add_subdirectory("fmt" EXCLUDE_FROM_ALL)

set_property(TARGET "fmt" PROPERTY FOLDER "External Libraries")

disable_project_warnings("fmt")
