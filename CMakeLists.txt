cmake_minimum_required(VERSION 3.22)
project(SDFs LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dépendances vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)

# Exécutable
add_executable(${PROJECT_NAME}
    source/main.cpp
    third_party/glad/src/glad.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
    glm::glm
    OpenGL::GL
)

# Copie automatique des assets dans le dossier du binaire
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)