cmake_minimum_required(VERSION 3.22)
project(SDFs LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# Vérifie la présence des bibliothèques critiques avant de lancer vcpkg
pkg_check_modules(SYSTEM_DEPS REQUIRED x11 wayland-client egl)

# Dépendances vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

list(REMOVE_ITEM COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/ray_marching.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/sphere_tracing.cpp"
)
list(APPEND COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/src/glad.c"
)

function(add_sdf_app target entrypoint)
    add_executable(${target} ${entrypoint} ${COMMON_SOURCES})

    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
    )

    target_link_libraries(${target} PRIVATE
        SDL2::SDL2
        glm::glm
        OpenGL::GL
    )

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${target}>/assets
    )
endfunction()

add_sdf_app(marching-cube source/ray_marching.cpp)
add_sdf_app(sphere-tracing source/sphere_tracing.cpp)