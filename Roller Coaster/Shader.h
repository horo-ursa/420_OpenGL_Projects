#ifndef SHADER_H
#define SHADER_H


#include <glad/glad.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>





class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void SetAcive();
    // utility uniform functions
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec4(const std::string& name, glm::vec4 vec) const;
    void SetVec3(const std::string& name, glm::vec3 vec) const;
	void SetMat4(const std::string& name, glm::mat4 vec) const;
};







#endif 
