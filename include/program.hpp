#pragma once
#include <iostream>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

class Program{
    public : 
        uint ID; 
        
        Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        Program(const std::string& shaderPath);

        void use(); 
    private : 
        static std::string read_shader(const std::string& shaderPath);
        GLuint compile_shader(GLenum type, const std::string& source);
        void link(GLuint vertex, GLuint frag);
        
};