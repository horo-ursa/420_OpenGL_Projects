#include "stdafx.h"
#include "BVH.h"


BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
	SplitMethod splitMethod)
	: maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
	primitives(std::move(p))
{
	time_t start, stop;
	time(&start);
	if (primitives.empty())
		return;

	root = recursiveBuild(primitives);

 	time(&stop);
	double diff = difftime(stop, start);
	int hrs = (int)diff / 3600;
	int mins = ((int)diff / 60) - (hrs * 60);
	int secs = (int)diff - (hrs * 3600) - (mins * 60);

	printf(
		"\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
		hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
	BVHBuildNode* node = new BVHBuildNode();

	//compute AABB for all objects
	Bounds3 bounds;
	for (auto& obj : objects) {
		bounds = Union(bounds, obj->getBounds());
	}

	//termination: only one object in BVHnode
	if (objects.size() == 1) {
		node->left = nullptr;
		node->right = nullptr;
		node->bounds = objects[0]->getBounds();
		node->object = objects[0];
		return node;
	}
	else if (objects.size() == 2) {
		node->left = recursiveBuild(std::vector<Object*>{objects[0]});
		node->right = recursiveBuild(std::vector<Object*>{objects[1]});
		node->bounds = Union(node->left->bounds, node->right->bounds);
		return node;
	}
	else {
		Bounds3 centroidBounds;
		for (int i = 0; i < objects.size(); ++i) {
			centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
		}
		//find longest axis, sort on that axis, and then split in the middle
		int dim = centroidBounds.maxExtent();
		switch (dim) {
		case 0:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().x <
					f2->getBounds().Centroid().x;
				});
			break;
		case 1:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().y <
					f2->getBounds().Centroid().y;
				});
			break;
		case 2:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().z <
					f2->getBounds().Centroid().z;
				});
			break;
		}

		auto beginning = objects.begin();
		auto ending = objects.end();
		auto middling = objects.begin() + (objects.size() / 2);

		auto leftshapes = std::vector<Object*>(beginning, middling);
		auto rightshapes = std::vector<Object*>(middling, ending);

		assert(objects.size() == (leftshapes.size() + rightshapes.size()));

		node->left = recursiveBuild(leftshapes);
		node->right = recursiveBuild(rightshapes);

		node->bounds = Union(node->left->bounds, node->right->bounds);
	}
	return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
	Intersection isect;
	if (!root)
	{
		return isect;
	}
	isect = BVHAccel::getIntersection(root, ray);
	return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const 
{
	//recursively test intersection
	//printf("%s", "doing ray BVH intersection\n");
	Intersection inter;
	const std::array<int, 3> isNeg = { int(ray.direction.x), int(ray.direction.y), int(ray.direction.z) };
	glm::vec3 invDir = glm::vec3(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
	//ternimation1: empty node
	if (node == nullptr) {
		return inter;
	}
	//termination2: no intersection
	if (!node->bounds.IntersectP(ray, invDir, isNeg)) {
		return inter;
	}
	//termination3: leaf node
	if (node->left == nullptr && node->right == nullptr) {
		return node->object->getIntersection(ray);
	}
	//recursive
	Intersection leftHit = getIntersection(node->left, ray);
	Intersection rightHit = getIntersection(node->right, ray);
	//return the nearest intersection
	return leftHit.distance < rightHit.distance ? leftHit : rightHit;
}