#pragma once
#include <glm\glm.hpp>
#include <limits.h>


class Object;

struct Intersection {
	bool happened;
	glm::vec3 coords;
	glm::vec3 normal;
	double distance;
	Object* obj;

	Intersection() {
		happened = false;
		coords = glm::vec3();
		normal = glm::vec3();
		distance = (std::numeric_limits<double>::max)();
		obj = nullptr;
	}
};