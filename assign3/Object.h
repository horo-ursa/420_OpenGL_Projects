#pragma once
#include "Ray.h"
#include "Intersection.h"
#include "Bounds3.h"
#include <glm\glm.hpp>
#include <vector>

#define EPSILON 0.0005f

class Object {
public:
	Object() {}
	virtual ~Object(){}
	virtual bool intersect(const Ray& ray) = 0;

	virtual Intersection getIntersection(Ray _ray) = 0;

	virtual glm::vec3 getDiffuseColor() const = 0;
	virtual Bounds3 getBounds() = 0;

	virtual glm::vec3 getSpecularColor() const = 0;
	virtual float getShiness() const = 0;
	virtual bool Barycentric(glm::vec3 p, float& alpha, float& beta, float& gamma) = 0;
	virtual std::vector<glm::vec3> getPoints() = 0;
};
