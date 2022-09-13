#pragma once
#include <glm\glm.hpp>


enum MaterialType { DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };

class Material {
public:
	MaterialType m_type;
	glm::vec3 m_color;
	glm::vec3 m_emission;
	float ior;
	float Kd, Ks;
	float specularExponent;

	inline Material(MaterialType t = DIFFUSE_AND_GLOSSY, glm::vec3 c = glm::vec3(1, 1, 1), glm::vec3 e = glm::vec3(0, 0, 0));
	inline MaterialType getType();
	inline glm::vec3 getColor();
	inline glm::vec3 getColorAt(double u, double v);
	inline glm::vec3 getEmission();
};

Material::Material(MaterialType t, glm::vec3 c, glm::vec3 e) {
	m_type = t;
	m_color = c;
	m_emission = e;
}

MaterialType Material::getType() {
	return m_type;
}

glm::vec3 Material::getColor() {
	return m_color;
}

glm::vec3 Material::getEmission() {
	return m_emission;
}

glm::vec3 Material::getColorAt(double u, double v) {
	return glm::vec3(0, 0, 0);
}