#pragma once
#include "BVH.h"
#include "Object.h"




class Triangle : public Object
{
public:
	glm::vec3 v0, v1, v2; // vertices A, B ,C , counter-clockwise order
	glm::vec3 e1, e2;     // 2 edges v1-v0, v2-v0;
	//glm::vec3 t0, t1, t2; // texture coords
	glm::vec3 normalv0, normalv1, normalv2;
	glm::vec3 difv0, difv1, difv2;
	glm::vec3 spev0, spev1, spev2;
	float shiness;
	//Material* m;

	Triangle(glm::vec3 _v0, glm::vec3 _v1, glm::vec3 _v2)
		: v0(_v0), v1(_v1), v2(_v2)
	{
		e1 = v1 - v0;
		e2 = v2 - v0;
		normalv0 = glm::normalize(normalv0);
		normalv1 = glm::normalize(normalv1);
		normalv2 = glm::normalize(normalv2);
	}


	bool intersect(const Ray& ray) override;
	
	Intersection getIntersection(Ray ray) override;

	glm::vec3 getDiffuseColor() const override;
	Bounds3 getBounds() override;
	glm::vec3 getSpecularColor() const override;
	float getShiness() const override;

	bool Barycentric(glm::vec3 p, float& alpha, float& beta, float& gamma) override
	{
		auto Ab = glm::length(glm::cross((v0 - v2), (p - v2))) * 0.5f;
		auto Ac = glm::length(glm::cross((v1 - v0), (p - v0))) * 0.5f;
		auto A = glm::length(glm::cross((v1 - v0), (v2 - v0))) * 0.5f;
		beta = Ab / A;
		gamma = Ac / A;
		alpha = 1.0f - beta - gamma;
		return true;
	}

	std::vector<glm::vec3> getPoints() override {
		std::vector<glm::vec3> points = { difv0, difv1, difv2, spev0, spev1, spev2, normalv0, normalv1, normalv2 };
		return points;
	}

	glm::vec3 interpolate(float alpha, float beta, float gamma, glm::vec3 vert1, glm::vec3 vert2, glm::vec3 vert3)
	{
		auto x = alpha * vert1.x + beta * vert2.x + gamma * vert3.x;
		auto y = alpha * vert1.y + beta * vert2.y + gamma * vert3.y;
		auto z = alpha * vert1.z + beta * vert2.z + gamma * vert3.z;
		return glm::vec3(x, y, z);
	}
};

inline bool Triangle::intersect(const Ray& ray) { 
	return true; 
}


inline Bounds3 Triangle::getBounds() { 
	return Union(Bounds3(v0, v1), v2); 
}

inline Intersection Triangle::getIntersection(Ray ray)
{
	Intersection inter;

	double b1, b2, t_tmp = 0;
	glm::vec3 S1 = glm::cross(ray.direction, e2);
	double det = glm::dot(S1, e1);
	if (fabs(det) < EPSILON)
		return inter;

	double det_inv = 1. / det;
	glm::vec3 S = ray.origin - v0;
	b1 = glm::dot(S, S1) * det_inv;
	if (b1 < 0 || b1 > 1) { return inter; }
		
	glm::vec3 S2 = glm::cross(S, e1);
	b2 = glm::dot(S2, ray.direction) * det_inv;
	if (b2 < 0 || b1 + b2 > 1) { return inter; }
		
	t_tmp = glm::dot(S2, e2) * det_inv;

	if (t_tmp < EPSILON) return inter;


	inter.distance = t_tmp;
	inter.coords = ray(t_tmp);
	inter.happened = true;
	float alpha, beta, gamma;
	Barycentric(inter.coords, alpha, beta, gamma);
	inter.normal = glm::normalize(interpolate(alpha, beta, gamma, normalv0, normalv1, normalv2));
	inter.obj = this;
	//inter.m = m;

	return inter;
}

inline glm::vec3 Triangle::getDiffuseColor() const
{
	return difv0;
}

inline glm::vec3 Triangle::getSpecularColor() const {
	return spev0;
}

inline float Triangle::getShiness() const {
	return shiness;
}
