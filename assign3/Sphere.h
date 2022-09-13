#pragma once

#include "Object.h"
#include <glm\glm.hpp>
#include "Bounds3.h"



inline  bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (fabs(discr - 0.0f) < EPSILON) x0 = x1 = -0.5f * b / a;
	else {
		float q = (b > 0) ? -0.5f * (b + sqrt(discr)) : -0.5f * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);
	return true;
}

class Sphere : public Object {
public:
	glm::vec3 center;
	float radius, radius2;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	float shiness;
	Sphere(const glm::vec3 &c, const float &r) : center(c), radius(r), radius2(r * r) {}
	
	bool intersect(const Ray& ray) {
		// analytic solution
		glm::vec3 L = ray.origin - center;
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.f * glm::dot(ray.direction, L);
		float c = glm::dot(L, L) - radius2;
		float t0, t1;
		if (!solveQuadratic(a, b, c, t0, t1)) { return false; }

		if (t0 < EPSILON) t0 = t1;
		if (t0 < EPSILON) return false;
		return true;
	}


	Intersection getIntersection(Ray ray) {
		Intersection result;
		result.happened = false;
		glm::vec3 L = ray.origin - center;
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2 * glm::dot(ray.direction, L);
		float c = glm::dot(L, L) - radius2;
		float t0, t1;
		if (!solveQuadratic(a, b, c, t0, t1)) return result;

		if (t0 < EPSILON) t0 = t1;
		if (t0 < EPSILON) return result;
		result.happened = true;

		result.coords = glm::vec3(ray.origin + ray.direction * t0);
		result.normal = glm::normalize(glm::vec3(result.coords - center));
		result.obj = this;
		result.distance = t0;
		return result;

	}


	glm::vec3 getDiffuseColor()const override {
		return diffuseColor;
	}
	Bounds3 getBounds() {
		return Bounds3(glm::vec3(center.x - radius, center.y - radius, center.z - radius),
			glm::vec3(center.x + radius, center.y + radius, center.z + radius));
	}

	glm::vec3 getSpecularColor() const {
		return specularColor;
	}

	float getShiness() const
	{
		return shiness;
	}
	
	/*for sphere, these two functions are useless*/
	bool Barycentric(glm::vec3 p, float& alpha, float& beta, float& gamma) override
	{
		return false;
	}

	std::vector<glm::vec3> getPoints() override {
		std::vector<glm::vec3> temp;
		return temp;
	}
};
