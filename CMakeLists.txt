cmake_minimum_required(VERSION 3.0.0)

# add_subdirectory("assimp")

project(my_opengl_project VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)

# find_package(glm CONFIG REQUIRED)
# target_link_libraries(main PRIVATE glm::glm-header-only)

add_executable(my_opengl_project 
    main.cpp
    glad.c
    stb_image.cpp
    game.cpp
    resource_manager.cpp
    shader.cpp
    texture.cpp
    sprite_renderer.cpp
    game_object.cpp
    game_level.cpp
    ball_object.cpp
    particle_generator.cpp
    post_processor.cpp)

# assimp/code/Common/material.cpp assimp/code/Common/Importer.h assimp/code/Common/Importer.cpp assimp/code/Common/scene.cpp

target_link_libraries(my_opengl_project glfw OpenGL::GL /home/stevica/openGL_projects/breakout/irrKlang/bin/linux-gcc-64/libIrrKlang.so)
