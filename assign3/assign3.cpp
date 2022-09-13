/*
CSCI 420 Computer Graphics
Assignment 3: Ray Tracer
Name: Shixuan Fang
*/


#include "stdafx.h"

#include "Ray.h"
#include "BVH.h"
#include "Bounds3.h"
#include "Intersection.h"
#include "Object.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Light.h"

#include <random>
#include <mutex>
#include <thread>

# if defined(_MSC_VER)
# ifndef _CRT_SECURE_NO_DEPRECATE
# define _CRT_SECURE_NO_DEPRECATE (1)
# endif
# pragma warning(disable : 4996)
# endif

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10


//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0f



struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

typedef struct _Triangle
{
	struct Vertex v[3];
} _Triangle;

typedef struct _Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
} _Sphere;

typedef struct _Light
{
	double position[3];
	double color[3];
} _Light;


_Triangle triangles[MAX_TRIANGLES];
_Sphere spheres[MAX_SPHERES];
_Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

//global data
BVHAccel* mBVH;
std::vector<Light*> mLights;
std::vector<Object*> mObjects;
glm::vec3 eye_pos = glm::vec3(0, 0, 0);
int maxRecurDepth = 0;
unsigned char buffer[HEIGHT][WIDTH][3];
bool MSAA = false;
bool blur = false;
bool softShadow = false;
bool recurRayCast = false;
std::mutex mutex_ins;
int sample_per_light = 0;
float light_radius = 0.f;

/* Write a jpg image from buffer*/
void save_jpg(char* file)
{
	if (file == NULL)
		return;

	// Allocate a picture buffer // 

	cv::Mat3b bufferBGR = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", file);
	// unsigned char buffer[HEIGHT][WIDTH][3];
	for (int r = 0; r < HEIGHT; r++) {
		for (int c = 0; c < WIDTH; c++) {
			/*Gaussian Blur*/
			if (blur) {
				int count = 0;
				unsigned char red, green, blue;
				if (r - 1 < 0 && c - 1 < 0) {//left up corner
					red = 0.25f * (buffer[r][c][0] + buffer[r][c + 1][0] + buffer[r + 1][c][0] + buffer[r + 1][c + 1][0]);
					green = 0.25f * (buffer[r][c][1] + buffer[r][c + 1][1] + buffer[r + 1][c][1] + buffer[r + 1][c + 1][1]);
					blue = 0.25f * (buffer[r][c][2] + buffer[r][c + 1][2] + buffer[r + 1][c][2] + buffer[r + 1][c + 1][2]);
				}
				else if (r - 1 < 0 && c + 1 >= WIDTH) {//right up corner
					red = 0.25f * (buffer[r][c][0] + buffer[r][c - 1][0] + buffer[r + 1][c][0] + buffer[r + 1][c - 1][0]);
					green = 0.25f * (buffer[r][c][1] + buffer[r][c - 1][1] + buffer[r + 1][c][1] + buffer[r + 1][c - 1][1]);
					blue = 0.25f * (buffer[r][c][2] + buffer[r][c - 1][2] + buffer[r + 1][c][2] + buffer[r + 1][c - 1][2]);
				}
				else if (r + 1 >= HEIGHT && c - 1 < 0) {//left down corner
					red = 0.25f * (buffer[r][c][0] + buffer[r][c + 1][0] + buffer[r - 1][c][0] + buffer[r - 1][c + 1][0]);
					green = 0.25f * (buffer[r][c][1] + buffer[r][c + 1][1] + buffer[r - 1][c][1] + buffer[r - 1][c + 1][1]);
					blue = 0.25f * (buffer[r][c][2] + buffer[r][c + 1][2] + buffer[r - 1][c][2] + buffer[r - 1][c + 1][2]);
				}
				else if (r + 1 >= HEIGHT && c + 1 >= WIDTH) {//right down corner
					red = 0.25f * (buffer[r][c][0] + buffer[r][c - 1][0] + buffer[r - 1][c - 1][0] + buffer[r - 1][c][0]);
					green = 0.25f * (buffer[r][c][1] + buffer[r][c - 1][1] + buffer[r - 1][c - 1][1] + buffer[r - 1][c][1]);
					blue = 0.25f * (buffer[r][c][2] + buffer[r][c - 1][2] + buffer[r - 1][c - 1][2] + buffer[r - 1][c][2]);
				}
				else if (r - 1 < 0) {//up edge
					red = 1.0f / 6 * (buffer[r][c - 1][0] + buffer[r][c][0] + buffer[r][c + 1][0] + buffer[r + 1][c - 1][0] + buffer[r + 1][c][0] + buffer[r + 1][c + 1][0]);
					green = 1.0f / 6 * (buffer[r][c - 1][1] + buffer[r][c][1] + buffer[r][c + 1][1] + buffer[r + 1][c - 1][1] + buffer[r + 1][c][1] + buffer[r + 1][c + 1][1]);
					blue = 1.0f / 6 * (buffer[r][c - 1][2] + buffer[r][c][2] + buffer[r][c + 1][2] + buffer[r + 1][c - 1][2] + buffer[r + 1][c][2] + buffer[r + 1][c + 1][2]);
				}
				else if (r + 1 >= HEIGHT) {//down edge
					red = 1.0f / 6 * (buffer[r][c - 1][0] + buffer[r][c][0] + buffer[r][c + 1][0] + buffer[r - 1][c - 1][0] + buffer[r - 1][c][0] + buffer[r + 1][c - 1][0]);
					green = 1.0f / 6 * (buffer[r][c - 1][1] + buffer[r][c][1] + buffer[r][c + 1][1] + buffer[r - 1][c - 1][1] + buffer[r - 1][c][1] + buffer[r + 1][c - 1][1]);
					blue = 1.0f / 6 * (buffer[r][c - 1][2] + buffer[r][c][2] + buffer[r][c + 1][2] + buffer[r - 1][c - 1][2] + buffer[r - 1][c][2] + buffer[r + 1][c - 1][2]);
				}
				else if (c - 1 < 0) {//left edge
					red = 1.0f / 6 * (buffer[r - 1][c][0] + buffer[r][c][0] + buffer[r + 1][c][0] + buffer[r - 1][c + 1][0] + buffer[r][c + 1][0] + buffer[r + 1][c + 1][0]);
					green = 1.0f / 6 * (buffer[r - 1][c][1] + buffer[r][c][1] + buffer[r + 1][c][1] + buffer[r - 1][c + 1][1] + buffer[r][c + 1][1] + buffer[r + 1][c + 1][1]);
					blue = 1.0f / 6 * (buffer[r - 1][c][2] + buffer[r][c][2] + buffer[r + 1][c][2] + buffer[r - 1][c + 1][2] + buffer[r][c + 1][2] + buffer[r + 1][c + 1][2]);
				}
				else if (c + 1 >= WIDTH) {//right edge
					red = 1.0f / 6 * (buffer[r - 1][c][0] + buffer[r][c][0] + buffer[r + 1][c][0] + buffer[r - 1][c - 1][0] + buffer[r][c - 1][0] + buffer[r + 1][c - 1][0]);
					green = 1.0f / 6 * (buffer[r - 1][c][1] + buffer[r][c][1] + buffer[r + 1][c][1] + buffer[r - 1][c - 1][1] + buffer[r][c - 1][1] + buffer[r + 1][c - 1][1]);
					blue = 1.0f / 6 * (buffer[r - 1][c][2] + buffer[r][c][2] + buffer[r + 1][c][2] + buffer[r - 1][c - 1][2] + buffer[r][c - 1][2] + buffer[r + 1][c - 1][2]);
				}
				else {//safe
					red = 1.0f / 9 * (buffer[r - 1][c - 1][0] + buffer[r][c - 1][0] + buffer[r + 1][c - 1][0] + buffer[r - 1][c][0] + buffer[r][c][0] + buffer[r + 1][c][0] + buffer[r - 1][c + 1][0] + buffer[r][c + 1][0] + buffer[r + 1][c + 1][0]);
					green = 1.0f / 9 * (buffer[r - 1][c - 1][1] + buffer[r][c - 1][1] + buffer[r + 1][c - 1][1] + buffer[r - 1][c][1] + buffer[r][c][1] + buffer[r + 1][c][1] + buffer[r - 1][c + 1][1] + buffer[r][c + 1][1] + buffer[r + 1][c + 1][1]);
					blue = 1.0f / 9 * (buffer[r - 1][c - 1][2] + buffer[r][c - 1][2] + buffer[r + 1][c - 1][2] + buffer[r - 1][c][2] + buffer[r][c][2] + buffer[r + 1][c][2] + buffer[r - 1][c + 1][2] + buffer[r][c + 1][2] + buffer[r + 1][c + 1][2]);
				}
				bufferBGR.at<cv::Vec3b>(r, c) = cv::Vec3b(blue, green, red);
			}
			else {
				unsigned char red = buffer[r][c][0];
				unsigned char green = buffer[r][c][1];
				unsigned char blue = buffer[r][c][2];
				bufferBGR.at<cv::Vec3b>(r, c) = cv::Vec3b(blue, green, red);
			}
			
		}
	}
	//if (cv::imwrite(filename, bufferBGR)) {
	if (cv::imwrite(file, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

void parse_check(char* expected, char* found)
{
	if (stricmp(expected, found))
	{
		char error[100];
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parse error, abnormal abortion\n");
		exit(0);
	}
}

void parse_doubles(FILE* file, char* check, double p[3])
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check(check, str);
	fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
	//printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
}

void parse_rad(FILE* file, double* r)
{
	char str[100];
	fscanf(file, "%s", str);
	char temp[] = "rad:";
	parse_check(temp, str);
	fscanf(file, "%lf", r);
	//printf("rad: %f\n", *r);
}

void parse_shi(FILE* file, double* shi)
{
	char s[100];
	fscanf(file, "%s", s);
	char temp[] = "shi:";
	parse_check(temp, s);
	fscanf(file, "%lf", shi);
	//printf("shi: %f\n", *shi);
}

int loadScene(char* argv)
{
	FILE* file = fopen(argv, "r");
	int number_of_objects;
	char type[50];
	int i;
	_Triangle t;
	_Sphere s;
	_Light l;
	fscanf(file, "%i", &number_of_objects);

	//printf("number of objects: %i\n", number_of_objects);
	char str[200];

	char temp1[] = "amb:";
	char temp2[] = "pos:";
	char temp3[] = "nor:";
	char temp4[] = "dif:";
	char temp5[] = "spe:";
	char temp6[] = "col:";

	parse_doubles(file, temp1, ambient_light);

	for (i = 0; i < number_of_objects; i++)
	{
		fscanf(file, "%s\n", type);
		//printf("%s\n", type);
		if (stricmp(type, "triangle") == 0)
		{

			//printf("found triangle\n");
			int j;

			for (j = 0; j < 3; j++)
			{
				parse_doubles(file, temp2, t.v[j].position);
				parse_doubles(file, temp3, t.v[j].normal);
				parse_doubles(file, temp4, t.v[j].color_diffuse);
				parse_doubles(file, temp5, t.v[j].color_specular);
				parse_shi(file, &t.v[j].shininess);
			}

			if (num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
			/*put into vector?*/
			auto proMatrix = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.0f, 100.0f);
			/*
			auto v0 = glm::vec4(t.v[0].position[0], t.v[0].position[1], t.v[0].position[2],0.f) * proMatrix;
			auto v1 = glm::vec4(t.v[1].position[0], t.v[1].position[1], t.v[1].position[2],0.f) * proMatrix;
			auto v2 = glm::vec4(t.v[2].position[0], t.v[2].position[1], t.v[2].position[2], 0.f) * proMatrix;
			Triangle* temp = new Triangle(v0, v1, v2);*/

			auto v0 = glm::vec3(t.v[0].position[0], t.v[0].position[1], t.v[0].position[2]);
			auto v1 = glm::vec3(t.v[1].position[0], t.v[1].position[1], t.v[1].position[2]);
			auto v2 = glm::vec3(t.v[2].position[0], t.v[2].position[1], t.v[2].position[2]);
			Triangle* temp = new Triangle(v0, v1, v2);

			//normal vector
			temp->normalv0 = glm::vec3(t.v[0].normal[0], t.v[0].normal[1], t.v[0].normal[2]);
			temp->normalv1 = glm::vec3(t.v[1].normal[0], t.v[1].normal[1], t.v[1].normal[2]);
			temp->normalv2 = glm::vec3(t.v[2].normal[0], t.v[2].normal[1], t.v[2].normal[2]);
			//shiness, similarly assume shiness is same for each vertex
			temp->shiness = t.v[0].shininess;
			//diffuse color for each vertex
			temp->difv0 = glm::vec3(t.v[0].color_diffuse[0], t.v[0].color_diffuse[1], t.v[0].color_diffuse[2]);
			temp->difv1 = glm::vec3(t.v[1].color_diffuse[0], t.v[1].color_diffuse[1], t.v[1].color_diffuse[2]);
			temp->difv2 = glm::vec3(t.v[2].color_diffuse[0], t.v[2].color_diffuse[1], t.v[2].color_diffuse[2]);
			//specular color for each vertex
			temp->spev0 = glm::vec3(t.v[0].color_specular[0], t.v[0].color_specular[1], t.v[0].color_specular[2]);
			temp->spev1 = glm::vec3(t.v[1].color_specular[0], t.v[1].color_specular[1], t.v[1].color_specular[2]);
			temp->spev2 = glm::vec3(t.v[2].color_specular[0], t.v[2].color_specular[1], t.v[2].color_specular[2]);
			mObjects.push_back(temp);
		}
		else if (stricmp(type, "sphere") == 0)
		{
			//printf("found sphere\n");

			parse_doubles(file, temp2, s.position);
			parse_rad(file, &s.radius);
			parse_doubles(file, temp4, s.color_diffuse);
			parse_doubles(file, temp5, s.color_specular);
			parse_shi(file, &s.shininess);

			if (num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;

			auto center = glm::vec3(s.position[0], s.position[1], s.position[2]);
			float radius = s.radius;
			Sphere* temp = new Sphere(center, radius);
			temp->shiness = s.shininess;
			temp->diffuseColor = glm::vec3(s.color_diffuse[0], s.color_diffuse[1], s.color_diffuse[2]);
			temp->specularColor = glm::vec3(s.color_specular[0], s.color_specular[1], s.color_specular[2]);
			mObjects.push_back(temp);
		}
		else if (stricmp(type, "light") == 0)
		{
			//printf("found light\n");
			parse_doubles(file, temp2, l.position);
			parse_doubles(file, temp6, l.color);

			if (num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;

			auto pos = glm::vec3(l.position[0], l.position[1], l.position[2]);
			auto color = glm::vec3(l.color[0], l.color[1], l.color[2]);
			Light* temp = new Light(pos, color);
			mLights.push_back(temp);
		}
		else
		{
			printf("unknown type in scene description:\n%s\n", type);
			exit(0);
		}
	}
	fclose(file);
	return 0;
}


//function forward declaration
void buildBVH();
void cleanMemory();
glm::vec3 castRay(const Ray& ray, int depth);
glm::vec3 recurCastRay(const Ray& ray, int depth);
void renderScene();
void recurRenderScene();
Intersection bvhIntersect(const Ray& ray);
//inline void UpdateProgress(float progress);
glm::vec3 interpolate(float alpha, float beta, float gamma, glm::vec3 vert0, glm::vec3 vert1, glm::vec3 vert2);
float sample_light(int spp, glm::vec3 shadePos, glm::vec3 lightPos, glm::vec3 biNormal, glm::vec3 tangent);
float generate_random_number(float x1, float x2);

inline void UpdateProgress(float progress)
{
	int barWidth = 70;

	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();
};

inline float clamp(const float& lo, const float& hi, const float& v)
{
	return std::max(lo, std::min(hi, v));
}

void buildBVH() {
	std::cout << "Generating BVH tree" << std::endl;
	mBVH = new BVHAccel(mObjects, 1, BVHAccel::SplitMethod::NAIVE);
}

void cleanMemory()
{
	for (int i = 0; i < mObjects.size(); i++) {
		delete mObjects[i];
	}
	for (int i = 0; i < mLights.size(); i++) {
		delete mLights[i];
	}
}

/*whitted-style light transport algorithm*/
glm::vec3 recurCastRay(const Ray& ray, int depth) {
	//termination: over max depth
	if (depth >= maxRecurDepth) {
		return glm::vec3(0.0, 0.0, 0.0);
	}
	Intersection intersection = bvhIntersect(ray);
	glm::vec3 normal = glm::normalize(intersection.normal);
	glm::vec3 localPhongColor = glm::vec3(0, 0, 0);
	glm::vec3 inshadowColor = glm::vec3(0, 0, 0);
	glm::vec3 hitPoint = intersection.coords;
	Object* hitObject = intersection.obj;
	glm::vec3 COLOR;
	float ks = 0.f;
	glm::vec3 reflectDir = glm::normalize(glm::reflect(ray.direction, normal));
	auto inter = bvhIntersect(Ray(hitPoint, reflectDir));
	if (!inter.happened) {
		COLOR = glm::vec3();
	}
	else {
		//accumulated reflect color
		ks = hitObject->getSpecularColor().x;
		glm::vec3 reflectDir = glm::normalize(glm::reflect(ray.direction, normal));
		COLOR = ks * castRay(Ray(hitPoint, reflectDir), depth + 1);
	}
	/*local blinn-phong shading model*/
	for (int i = 0; i < mLights.size(); i++) {
		auto lightDir = glm::normalize(mLights[i]->pos - hitPoint);
		auto viewDir = glm::normalize(eye_pos - hitPoint);
		auto refDir = glm::reflect(-lightDir, normal);
		/*to avoid self-occlusion*/
		glm::vec3 shadowPointOrig = hitPoint;
		/*check whether this light can lit this hitPoint*/
		auto shadowIntersection = bvhIntersect(Ray(shadowPointOrig, lightDir));
		auto lightDistance = glm::length(mLights[i]->pos - shadowPointOrig);

		//biased soft shadow: generate five rays, number of points in shadow
		//calculate perpendicular to pr
		auto biNormal = glm::normalize(glm::cross(lightDir, glm::vec3(0.0f, 1.0f, 0.0f)));
		// Handle case where L = up -> perpL should then be (1,0,0)
		if (biNormal.x <= FLT_EPSILON && biNormal.y <= FLT_EPSILON && biNormal.z <= FLT_EPSILON) {
			biNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		glm::vec3 tangent = glm::normalize(glm::cross(biNormal, lightDir));

		float alpha, beta, gamma;
		glm::vec3 lightColor = glm::vec3();
		if (softShadow) {
			auto ratio = sample_light(100, shadowPointOrig, mLights[i]->pos, biNormal, tangent);
			auto pointsColor = hitObject->getPoints();
			/*this is for triangles*/
			if (hitObject->Barycentric(hitPoint, alpha, beta, gamma))
			{
				auto interpoDiffuse = interpolate(alpha, beta, gamma, pointsColor[0], pointsColor[1], pointsColor[2]);
				auto interpoSpecular = interpolate(alpha, beta, gamma, pointsColor[3], pointsColor[4], pointsColor[5]);
				auto interpoNormal = interpolate(alpha, beta, gamma, pointsColor[6], pointsColor[7], pointsColor[8]);
				normal = interpoNormal;
				auto difColor = interpoDiffuse * std::max(glm::dot(interpoNormal, lightDir), 0.0f);
				auto speColor = interpoSpecular * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
				lightColor = mLights[i]->color * (1.0f - ratio) * (difColor + speColor);
			}
			/*this is for spheres*/
			else {
				auto difColor = hitObject->getDiffuseColor() * std::max(glm::dot(normal, lightDir), 0.0f);
				auto speColor = hitObject->getSpecularColor() * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
				lightColor = mLights[i]->color * (1.0f - ratio) * (difColor + speColor);
			}
			localPhongColor += lightColor;
		}
		else {
			bool inShadow = (bvhIntersect(Ray(shadowPointOrig, lightDir)).happened) & (shadowIntersection.distance <= lightDistance);
			if (!inShadow) {
				auto pointsColor = hitObject->getPoints();
				if (hitObject->Barycentric(hitPoint, alpha, beta, gamma))
				{
					auto interpoDiffuse = interpolate(alpha, beta, gamma, pointsColor[0], pointsColor[1], pointsColor[2]);
					auto interpoSpecular = interpolate(alpha, beta, gamma, pointsColor[3], pointsColor[4], pointsColor[5]);
					auto interpoNormal = interpolate(alpha, beta, gamma, pointsColor[6], pointsColor[7], pointsColor[8]);
					normal = interpoNormal;
					auto difColor = interpoDiffuse * std::max(glm::dot(interpoNormal, lightDir), 0.0f);
					auto speColor = interpoSpecular * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
					lightColor = mLights[i]->color * (difColor + speColor);
				}
				/*this is for spheres*/
				else {
					auto difColor = hitObject->getDiffuseColor() * std::max(glm::dot(normal, lightDir), 0.0f);
					auto speColor = hitObject->getSpecularColor() * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
					lightColor = mLights[i]->color * (difColor + speColor);
				}
				localPhongColor += lightColor;
			}
			else {
				localPhongColor += inshadowColor;
			}
		}
	}

	COLOR += (1 - ks) * localPhongColor;
	return COLOR;
}

glm::vec3 castRay(const Ray& ray, int depth)
{
	//termination: over max depth
	if (depth >= maxRecurDepth) { 
		return glm::vec3(0.0, 0.0, 0.0);
	}
	Intersection intersection = bvhIntersect(ray);
	glm::vec3 normal = glm::normalize(intersection.normal);
	glm::vec3 localPhongColor = glm::vec3(0, 0, 0);
	glm::vec3 inshadowColor = glm::vec3(0, 0, 0);
	glm::vec3 hitPoint = intersection.coords;
	Object* hitObject = intersection.obj;
	
	/*local blinn-phong shading model*/
	for (int i = 0; i < mLights.size(); i++) {
		auto lightDir = glm::normalize(mLights[i]->pos - hitPoint);
		auto viewDir = glm::normalize(eye_pos - hitPoint);
		auto refDir = glm::reflect(-lightDir, normal);
		/*to avoid self-occlusion*/
		glm::vec3 shadowPointOrig = hitPoint;
		/*check whether this light can lit this hitPoint*/
		auto shadowIntersection = bvhIntersect(Ray(shadowPointOrig, lightDir));
		auto lightDistance = glm::length(mLights[i]->pos - shadowPointOrig);

		//biased soft shadow: generate five rays, number of points in shadow
		//calculate perpendicular to pr
		auto biNormal = glm::normalize(glm::cross(lightDir, glm::vec3(0.0f, 1.0f, 0.0f)));
		// Handle case where L = up -> perpL should then be (1,0,0)
		if (biNormal.x <= FLT_EPSILON && biNormal.y <= FLT_EPSILON && biNormal.z <= FLT_EPSILON){
			biNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		glm::vec3 tangent = glm::normalize(glm::cross(biNormal, lightDir)); 

		float alpha, beta, gamma;
		glm::vec3 lightColor = glm::vec3();
		if (softShadow) {
			auto ratio = sample_light(100, shadowPointOrig, mLights[i]->pos, biNormal, tangent);
			auto pointsColor = hitObject->getPoints();
			/*this is for triangles*/
			if (hitObject->Barycentric(hitPoint, alpha, beta, gamma))
			{
				auto interpoDiffuse = interpolate(alpha, beta, gamma, pointsColor[0], pointsColor[1], pointsColor[2]);
				auto interpoSpecular = interpolate(alpha, beta, gamma, pointsColor[3], pointsColor[4], pointsColor[5]);
				auto interpoNormal = interpolate(alpha, beta, gamma, pointsColor[6], pointsColor[7], pointsColor[8]);
				normal = interpoNormal;
				auto difColor = interpoDiffuse * std::max(glm::dot(interpoNormal, lightDir), 0.0f);
				auto speColor = interpoSpecular * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
				lightColor = mLights[i]->color * (1.0f - ratio)  * (difColor + speColor);
			}
			/*this is for spheres*/
			else {
				auto difColor = hitObject->getDiffuseColor() * std::max(glm::dot(normal, lightDir), 0.0f);
				auto speColor = hitObject->getSpecularColor() * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
				lightColor = mLights[i]->color * (1.0f - ratio) * (difColor + speColor);
			}
			localPhongColor += lightColor;
		}
		else {
			bool inShadow = (bvhIntersect(Ray(shadowPointOrig, lightDir)).happened) & (shadowIntersection.distance <= lightDistance);
			if (!inShadow) {
				auto pointsColor = hitObject->getPoints();
				if (hitObject->Barycentric(hitPoint, alpha, beta, gamma))
				{
					auto interpoDiffuse = interpolate(alpha, beta, gamma, pointsColor[0], pointsColor[1], pointsColor[2]);
					auto interpoSpecular = interpolate(alpha, beta, gamma, pointsColor[3], pointsColor[4], pointsColor[5]);
					auto interpoNormal = interpolate(alpha, beta, gamma, pointsColor[6], pointsColor[7], pointsColor[8]);
					normal = interpoNormal;
					auto difColor = interpoDiffuse * std::max(glm::dot(interpoNormal, lightDir), 0.0f);
					auto speColor = interpoSpecular * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
					lightColor = mLights[i]->color * (difColor + speColor);
				}
				/*this is for spheres*/
				else {
					auto difColor = hitObject->getDiffuseColor() * std::max(glm::dot(normal, lightDir), 0.0f);
					auto speColor = hitObject->getSpecularColor() * std::pow(std::max(0.0f, (glm::dot(refDir, viewDir))), hitObject->getShiness());
					lightColor = mLights[i]->color * (difColor + speColor);
				}
				localPhongColor += lightColor;
			}
			else {
				localPhongColor += inshadowColor;
			}
		}
	}
	return localPhongColor;
}

Intersection bvhIntersect(const Ray& ray)
{
	return mBVH->Intersect(ray);
}

void renderScene()
{
	float scale = tan(glm::radians(fov * 0.5));
	float imageAspectRatio = WIDTH / (float)HEIGHT;
	int m = 0;
	int process = 0;

	auto castRayMultiThreading = [&](uint32_t rowStart, uint32_t rowEnd, uint32_t colStart, uint32_t colEnd)
	{
		//glm::vec3 pixelColor = glm::vec3();
		for (uint32_t j = rowStart; j < rowEnd; ++j) {
			int m = j * WIDTH + colStart;
			for (uint32_t i = colStart; i < colEnd; ++i) {
				auto pixelColor = glm::vec3();
				if (MSAA) {
					// generate primary ray direction
					float x1 = (2.f * (i + 0.25f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
					float x2 = (2.f * (i + 0.75f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
					float y1 = (1.f - 2.f * (j + 0.25f) / (float)HEIGHT) * scale;
					float y2 = (1.f - 2.f * (j + 0.75f) / (float)HEIGHT) * scale;
					auto dir1 = glm::normalize(glm::vec3(x1, y1, -1)),
						dir2 = glm::normalize(glm::vec3(x1, y2, -1)),
						dir3 = glm::normalize(glm::vec3(x2, y1, -1)),
						dir4 = glm::normalize(glm::vec3(x2, y2, -1));
					Ray ray1(eye_pos, dir1), ray2(eye_pos, dir2), ray3(eye_pos, dir3), ray4(eye_pos, dir4);
					Intersection inter1 = bvhIntersect(ray1);
					Intersection inter2 = bvhIntersect(ray2);
					Intersection inter3 = bvhIntersect(ray3);
					Intersection inter4 = bvhIntersect(ray4);
					if (!inter1.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += castRay(ray1, 0); }
					if (!inter2.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += castRay(ray2, 0); }
					if (!inter3.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += castRay(ray3, 0); }
					if (!inter4.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += castRay(ray4, 0); }

					pixelColor = pixelColor / 4.0f + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2]);
				}
				else {
					float x = (2 * (i + 0.5) / (float)WIDTH - 1) * imageAspectRatio * scale;
					float y = (1 - 2 * (j + 0.5) / (float)HEIGHT) * scale;

					glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
					Ray ray(eye_pos, dir);
					Intersection intersection = bvhIntersect(ray);
					if (!intersection.happened) {
						pixelColor = glm::vec3(1.0, 1.0, 1.0);
					}
					else {
						pixelColor = castRay(ray, 0) + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2]);
					}
				}

				m++;
				process++;

				buffer[j][i][0] = (unsigned char)(255 * clamp(0, 1, pixelColor.x));
				buffer[j][i][1] = (unsigned char)(255 * clamp(0, 1, pixelColor.y));
				buffer[j][i][2] = (unsigned char)(255 * clamp(0, 1, pixelColor.z));
			}

			// lock for update progress
			{
				std::lock_guard<std::mutex> g1(mutex_ins);
				UpdateProgress(1.0 * process / WIDTH / HEIGHT);
			}
		}
	};

	int id = 0;
	constexpr int bx = 8;
	constexpr int by = 8;
	int strideX = (WIDTH + 1) / bx;
	int strideY = (HEIGHT + 1) / by;
	std::vector<std::thread> threadList;
	for (int i = 0; i < HEIGHT; i += strideX){
		for (int j = 0; j < WIDTH; j += strideY){
			threadList.push_back(std::thread(castRayMultiThreading, i, std::min(i + strideX, HEIGHT), j, std::min(j + strideY, WIDTH)));
			id++;
		}
	}

	for (auto& t : threadList) {
		t.join();
	}
	UpdateProgress(1.f);
	
	/*for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			glm::vec3 pixelColor;
			//MSAA
			if (MSAA) {
				float x1 = (2.f * (i + 0.25f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
				float x2 = (2.f * (i + 0.75f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
				float y1 = (1.f - 2.f * (j + 0.25f) / (float)HEIGHT) * scale;
				float y2 = (1.f - 2.f * (j + 0.75f) / (float)HEIGHT) * scale;
				auto dir1 = glm::normalize(glm::vec3(x1, y1, -1)), 
					 dir2 = glm::normalize(glm::vec3(x1, y2, -1)),
					 dir3 = glm::normalize(glm::vec3(x2, y1, -1)), 
					 dir4 = glm::normalize(glm::vec3(x2, y2, -1));
				Ray ray1(eye_pos, dir1), ray2(eye_pos, dir2), ray3(eye_pos, dir3), ray4(eye_pos, dir4);
				Intersection inter1 = bvhIntersect(ray1);
				Intersection inter2 = bvhIntersect(ray2);
				Intersection inter3 = bvhIntersect(ray3);
				Intersection inter4 = bvhIntersect(ray4);
				if (!inter1.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
				else { pixelColor += castRay(ray1, 0); }
				if (!inter2.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
				else { pixelColor += castRay(ray2, 0); }
				if (!inter3.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
				else { pixelColor += castRay(ray3, 0); }
				if (!inter4.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
				else { pixelColor += castRay(ray4, 0); }

				pixelColor = (pixelColor + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2])) / 4.0f;
			}
			else {
				float x = (2.f * (i + 0.5f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
				float y = (1.f - 2.f * (j + 0.5f) / (float)HEIGHT) * scale;
				glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
				//glm::vec3 dir = glm::vec3(x, y, -1);
				Ray ray(eye_pos, dir);
				//frameBuffer[m++] = castRay(ray, 0);
				Intersection intersection = bvhIntersect(ray);
				if (!intersection.happened) {
					pixelColor = glm::vec3(1.0, 1.0, 1.0);
				}else{
					pixelColor = castRay(ray, 0) + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2]);
				}

			}
			buffer[j][i][0] = (unsigned char)(255 * clamp(0, 1, pixelColor.x));
			buffer[j][i][1] = (unsigned char)(255 * clamp(0, 1, pixelColor.y));
			buffer[j][i][2] = (unsigned char)(255 * clamp(0, 1, pixelColor.z));
		}
		UpdateProgress(j / (float)HEIGHT);
	}
	UpdateProgress(1.f);
	/*FILE* fp = fopen("test1.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
	for (auto i = 0; i < HEIGHT * WIDTH; ++i) {
		static unsigned char color[3];
		color[0] = (unsigned char)(255 * clamp(0, 1, frameBuffer[i].x));
		color[1] = (unsigned char)(255 * clamp(0, 1, frameBuffer[i].y));
		color[2] = (unsigned char)(255 * clamp(0, 1, frameBuffer[i].z));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);*/
}

void recurRenderScene() {
	float scale = tan(glm::radians(fov * 0.5));
	float imageAspectRatio = WIDTH / (float)HEIGHT;
	int m = 0;
	int process = 0;

	auto castRayMultiThreading = [&](uint32_t rowStart, uint32_t rowEnd, uint32_t colStart, uint32_t colEnd)
	{
		//glm::vec3 pixelColor = glm::vec3();
		for (uint32_t j = rowStart; j < rowEnd; ++j) {
			int m = j * WIDTH + colStart;
			for (uint32_t i = colStart; i < colEnd; ++i) {
				auto pixelColor = glm::vec3();
				if (MSAA) {
					// generate primary ray direction
					float x1 = (2.f * (i + 0.25f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
					float x2 = (2.f * (i + 0.75f) / (float)WIDTH - 1.f) * imageAspectRatio * scale;
					float y1 = (1.f - 2.f * (j + 0.25f) / (float)HEIGHT) * scale;
					float y2 = (1.f - 2.f * (j + 0.75f) / (float)HEIGHT) * scale;
					auto dir1 = glm::normalize(glm::vec3(x1, y1, -1)),
						dir2 = glm::normalize(glm::vec3(x1, y2, -1)),
						dir3 = glm::normalize(glm::vec3(x2, y1, -1)),
						dir4 = glm::normalize(glm::vec3(x2, y2, -1));
					Ray ray1(eye_pos, dir1), ray2(eye_pos, dir2), ray3(eye_pos, dir3), ray4(eye_pos, dir4);
					Intersection inter1 = bvhIntersect(ray1);
					Intersection inter2 = bvhIntersect(ray2);
					Intersection inter3 = bvhIntersect(ray3);
					Intersection inter4 = bvhIntersect(ray4);
					if (!inter1.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += recurCastRay(ray1, 0); }
					if (!inter2.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += recurCastRay(ray2, 0); }
					if (!inter3.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += recurCastRay(ray3, 0); }
					if (!inter4.happened) { pixelColor += glm::vec3(1.0, 1.0, 1.0); }
					else { pixelColor += recurCastRay(ray4, 0); }

					pixelColor = pixelColor / 4.0f + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2]);
				}
				else {
					float x = (2 * (i + 0.5) / (float)WIDTH - 1) * imageAspectRatio * scale;
					float y = (1 - 2 * (j + 0.5) / (float)HEIGHT) * scale;

					glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
					Ray ray(eye_pos, dir);
					Intersection intersection = bvhIntersect(ray);
					if (!intersection.happened) {
						pixelColor = glm::vec3(1.0, 1.0, 1.0);
					}
					else {
						pixelColor = recurCastRay(ray, 0) + glm::vec3(ambient_light[0], ambient_light[1], ambient_light[2]);
					}
				}

				m++;
				process++;

				buffer[j][i][0] = (unsigned char)(255 * clamp(0, 1, pixelColor.x));
				buffer[j][i][1] = (unsigned char)(255 * clamp(0, 1, pixelColor.y));
				buffer[j][i][2] = (unsigned char)(255 * clamp(0, 1, pixelColor.z));
			}

			// lock for update progress
			{
				std::lock_guard<std::mutex> g1(mutex_ins);
				UpdateProgress(1.0 * process / WIDTH / HEIGHT);
			}
		}
	};

	int id = 0;
	constexpr int bx = 8;
	constexpr int by = 8;
	int strideX = (WIDTH + 1) / bx;
	int strideY = (HEIGHT + 1) / by;
	std::vector<std::thread> threadList;
	for (int i = 0; i < HEIGHT; i += strideX) {
		for (int j = 0; j < WIDTH; j += strideY) {
			threadList.push_back(std::thread(castRayMultiThreading, i, std::min(i + strideX, HEIGHT), j, std::min(j + strideY, WIDTH)));
			id++;
		}
	}

	for (auto& t : threadList) {
		t.join();
	}
	UpdateProgress(1.f);
}

glm::vec3 interpolate(float alpha, float beta, float gamma, glm::vec3 vert1, glm::vec3 vert2, glm::vec3 vert3)
{
	auto x = alpha * vert1.x + beta * vert2.x + gamma * vert3.x;
	auto y = alpha * vert1.y + beta * vert2.y + gamma * vert3.y;
	auto z = alpha * vert1.z + beta * vert2.z + gamma * vert3.z;
	return glm::vec3(x, y, z);
}

float generate_random_number(float x1, float x2)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(x1, x2);
	return dis(gen);
}

float sample_light(int spp, glm::vec3 shadePos, glm::vec3 lightPos, glm::vec3 biNormal, glm::vec3 tangent)
{
	int count = 0;
	std::vector<glm::vec3> pointList;
	float M_PI = 3.141592653589793f;
	for (int i = 0; i < spp; i++) {
		float a = generate_random_number(0.f, 1.f) * 2 * M_PI;
		float r = light_radius * sqrt(generate_random_number(0.f, 1.f));
		float x = r * cos(a);
		float y = r * sin(a);
		pointList.push_back(lightPos + tangent * x + biNormal * y);
	}
	for (auto& point : pointList) {
		glm::vec3 dir = point - shadePos;
		auto inter = bvhIntersect(Ray(shadePos, dir));
		if (inter.happened && inter.distance <= glm::length(point - shadePos)) {
			count++;
		}
	}
	return (float)count / (float)spp;
}

int main() {
	//1. configure settings
	MSAA = false;
	softShadow = false;
	recurRayCast = true;
	sample_per_light = 200; //for soft shadow
	light_radius = 0.2f; //0.2 for test2 and SIGGRAPH, 1.0 for spheres, 0.5 for table
	char load_file_name[] = "examples/table.scene";
	char save_file_name[] = "examples/test1_sdf.jpg";
	eye_pos = glm::vec3(0, 0, 0);
	maxRecurDepth = 3;
	//2. load file and get data
	loadScene(load_file_name);
	//3. use object data to create a BVH tree
	buildBVH();
	//4. render the scene
	auto start = std::chrono::system_clock::now();
	std::cout << "start rendering" << std::endl;
	if (recurRayCast) {
		recurRenderScene();
	}
	else {
		renderScene();
	}
	auto stop = std::chrono::system_clock::now();

	std::cout << "Render complete: \n";
	std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
	std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
	/*5. save file*/
	save_jpg(save_file_name);

	//6. clean all dynamic allocated data
	cleanMemory();
	return 0;
}
