#pragma once
#include <iostream>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

class Program{
    public : 
        uint ID; 
        
        Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::vector<std::string>& dependancies);
        Program(const std::string& shaderPath, const std::vector<std::string>& dependancies);

        void use(); 
    private : 
        static std::string read_shader(const std::string& shaderPath);
        GLuint compile_shader(GLenum type, const std::string& source, const std::string& dependancies_src);
        void link(GLuint vertex, GLuint frag);
        
};