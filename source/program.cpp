#include "program.hpp"

std::string Program::read_shader(const std::string& shaderPath){
    std::ifstream file(shaderPath);
    std::stringbuf buffer;
    if(!file.good()){
        throw std::runtime_error("error loading program : " + shaderPath);
    }
    std::cout << "program loaded successfully : " << shaderPath << std::endl;
    file.get(buffer, EOF);

    return buffer.str();
}

GLuint Program::compile_shader(GLenum type, const std::string& source, const std::string& dependancies_src){
    GLuint shader = glCreateShader(type);
    const char* src[] = {dependancies_src.c_str(), source.c_str()};

    glShaderSource(shader, 2, src, NULL);

    glCompileShader(shader);

    GLint status; 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status == GL_FALSE){
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        
        char *message= new char [length];
        
        glGetShaderInfoLog(shader, length, nullptr, message);
        
        //TODO should throw error
        std::cout << "[errors] while compiling shader : " << message << std::endl;
        
        delete [] message;
    }

    return shader;
}

void Program::link(GLuint vertex, GLuint frag){
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, frag);
    glLinkProgram(ID);

    GLint status; 
    glGetProgramiv(ID, GL_LINK_STATUS, &status);

    if(status == GL_FALSE){
        GLint length;
        glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &length);
        
        char *message= new char [length];
        
        glGetShaderInfoLog(ID, length, nullptr, message);

        //TODO should throw error
        std::cout << "[errors] while linking program :" << message << std::endl;
        
        delete [] message;
    }

}

Program::Program(const std::string& shaderPath, const std::vector<std::string>& dependancies){
    std::string src = read_shader(shaderPath);

    std::string version = "#version 330\n";
    std::string vertex_def = "#define VERTEX_SHADER\n";
    std::string frag_def = "#define FRAGMENT_SHADER\n";

    std::string dependancies_src = "";
    for(int i = 0; i<dependancies.size(); i++){
        dependancies_src+= read_shader(dependancies[i]) + "\n";
    }

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, version + vertex_def + src, dependancies_src);
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, version + frag_def + src, dependancies_src);

    link(vertex_shader, frag_shader);
}

Program::Program(const std::string& vertexShaderPath, const std::string& fragShaderPath, const std::vector<std::string>& dependancies){
    std::string vertex_src = read_shader(vertexShaderPath);
    std::string frag_src = read_shader(fragShaderPath);

    std::string dependancies_src = "#version 330\n";
    for(int i = 0; i<dependancies.size(); i++){
        dependancies_src+= read_shader(dependancies[i]) + "\n";
    }

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_src, dependancies_src);
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src, dependancies_src);

    link(vertex_shader, frag_shader);
}

void Program::use(){
    glUseProgram(ID);
}