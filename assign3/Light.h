#pragma once
#include <glm\glm.hpp>


class Light {
public:
	glm::vec3 color;
	glm::vec3 pos;
	Light(glm::vec3 po, glm::vec3 col) : color(col), pos(po) {}
	~Light(){}
};