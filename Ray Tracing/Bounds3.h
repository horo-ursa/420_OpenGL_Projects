#pragma once


#include "Ray.h"
#include "glm/glm.hpp"
#include <limits>
#include <array>


class Bounds3
{
public:
	glm::vec3 pMin, pMax;
	
	Bounds3()
	{
		double minNum = std::numeric_limits<double>::lowest();
		double maxNum = std::numeric_limits<double>::max();
		pMin = glm::vec3(minNum, minNum, minNum);
		pMax = glm::vec3(maxNum, maxNum, maxNum);
	}

	Bounds3(const glm::vec3 p) : pMin(p), pMax(p) {}

	Bounds3(const glm::vec3 p1, const glm::vec3 p2)
	{
		pMin = glm::vec3(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
		pMax = glm::vec3(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
	}

	glm::vec3 Diagonal() const { return pMax - pMin; }

	int maxExtent() const
	{
		auto d = Diagonal();
		if (d.x > d.y && d.x > d.z) { return  0; }
		else if (d.y > d.x) { return 1; }
		else { return 2; }
	}

	double SurfaceArea() const
	{
		auto d = Diagonal();
		return 2 * (d.x * d.y + d.y * d.z + d.z * d.x);
	}
	glm::vec3 Centroid() { return 0.5f * pMin + 0.5f * pMax; }
	Bounds3 Intersect(const Bounds3& b)
	{
		return Bounds3(glm::vec3(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
					   fmax(pMin.z, b.pMin.z)), 
			           glm::vec3(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
					   fmin(pMax.z, b.pMax.z)));
	}
	glm::vec3 Offset(const glm::vec3& p) const
	{
		glm::vec3 o = p - pMin;
		if (pMax.x > pMin.x) { o.x /= pMax.x - pMin.x; }
		if (pMax.y > pMin.y) { o.y /= pMax.y - pMin.y; }
		if (pMax.z > pMin.z) { o.z /= pMax.z - pMin.z; }
		return o;
	}
	//check 2 AABBs overlap or not
	bool Overlaps(const Bounds3& b1, const Bounds3& b2)
	{
		bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
		bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
		bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
		return (x && y && z);
	}
	//check if a point p is in a AABB
	bool Inside(const glm::vec3& p, const Bounds3& b)
	{
		return (p.x >= b.pMin.x && p.x <= b.pMax.x
			 && p.y >= b.pMin.y && p.y <= b.pMax.y
			 && p.z >= b.pMin.z && p.z <= b.pMax.z);
	}
	//make getting pMin and pMax easier
	inline const glm::vec3& operator[](int i) const 
	{
		return (i == 0) ? pMin : pMax;
	}

	inline bool IntersectP(const Ray& ray, const glm::vec3& invDir,
		const std::array<int, 3>& dirIsNeg) const;
};

inline bool Bounds3::IntersectP(const Ray& ray, const glm::vec3& invDir,
	const std::array<int, 3>& dirIsNeg) const
{
	auto temp1 = glm::vec3(pMax.x - ray.origin.x, pMax.y - ray.origin.y, pMax.z - ray.origin.z);
	auto temp2 = glm::vec3(pMin.x - ray.origin.x, pMin.y - ray.origin.y, pMin.z - ray.origin.z);
	//auto ttop = temp1 * invDir;
	//auto tbot = temp2 * invDir;
	//auto ttop = (pMax - ray.origin) * invDir;
	//auto tbot = (pMin - ray.origin) * invDir;
	glm::vec3 ttop = glm::vec3((float)temp1.x * (float)invDir.x, (float)temp1.y * (float)invDir.y, (float)temp1.z * (float)invDir.z);
	glm::vec3 tbot = glm::vec3((float)temp2.x * (float)invDir.x, (float)temp2.y * (float)invDir.y, (float)temp2.z * (float)invDir.z);
	auto tmin = glm::vec3(std::min(ttop.x, tbot.x), std::min(ttop.y, tbot.y), std::min(ttop.z, tbot.z));
	auto tmax = glm::vec3(std::max(ttop.x, tbot.x), std::max(ttop.y, tbot.y), std::max(ttop.z, tbot.z));

	float t0 = std::max(tmin.x, (std::max)(tmin.y, tmin.z));
	float t1 = std::min(tmax.x, (std::min)(tmax.y, tmax.z));
	return t0 <= t1 && t1 >= 0;
}

inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2) {
	Bounds3 ret;
	ret.pMin = glm::vec3((std::min)(b1.pMin.x, b2.pMin.x),
						 (std::min)(b1.pMin.y, b2.pMin.y),
						 (std::min)(b1.pMin.z, b2.pMin.z));
	ret.pMax = glm::vec3((std::max)(b1.pMax.x, b2.pMax.x),
						 (std::max)(b1.pMax.y, b2.pMax.y),
						 (std::max)(b1.pMax.z, b2.pMax.z));
	return ret;
}

inline Bounds3 Union(const Bounds3& b1, const glm::vec3& p) {
	Bounds3 ret;
	ret.pMin = glm::vec3((std::min)(b1.pMin.x, p.x),
						 (std::min)(b1.pMin.y, p.y),
						 (std::min)(b1.pMin.z, p.z));
	ret.pMax = glm::vec3((std::max)(b1.pMax.x, p.x),
						 (std::max)(b1.pMax.y, p.y),
						 (std::max)(b1.pMax.z, p.z));
	return ret;
}