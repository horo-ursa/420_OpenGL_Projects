#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

struct splinePoint {
	float x;
	float y;
	float z;
	splinePoint(float a, float b, float c) {
		x = a;
		y = b;
		z = c;
	}
	splinePoint(glm::vec3 pos) {
		x = pos.x;
		y = pos.y;
		z = pos.z;
	}
	glm::vec3 getPosition() {
		return glm::vec3(x, y, z);
	}
};
