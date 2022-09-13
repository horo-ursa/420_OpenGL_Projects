#pragma once
#include <glm\glm.hpp>
#include <glm\gtx\string_cast.hpp>
#include <glm/ext.hpp>


struct Ray {
	glm::vec3 origin;
	glm::vec3 direction, direction_inv;
	double t;
	double t_min, t_max;

	Ray(const glm::vec3& ori, const glm::vec3& dir, const double _t = 0.0) : origin(ori), direction(dir), t(_t) {
		direction_inv = glm::vec3(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
		t_min = 0.0;
		t_max = (std::numeric_limits<double>::max)();
	}

	glm::vec3 operator()(double t) const { 
		auto temp = glm::vec3(direction.x * t, direction.y * t, direction.z * t);
		return origin + temp; 
	}

	friend std::ostream& operator<<(std::ostream& os, const Ray& r) {
		//os << "[origin = " << r.origin << ", direction = " << r.direction << ", time = " << r.t << "]\n";
		//return os;
	}
};