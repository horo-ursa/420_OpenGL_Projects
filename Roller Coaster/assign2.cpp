// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 420 Computer Graphics
	Assignment 2: Roller Coasters
	Shixuan Fang
*/
#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "splinePoint.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"





//configure global opengl state
// -----------------------------

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
bool updatecamerapos = false;

float u_new = 0;
float u_current = 0;
float hMax = 0;
float g = 9.8f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//tangent, normal and binormal
std::vector<glm::vec3> tangentVector;
std::vector<glm::vec3> normalVector;
std::vector<glm::vec3> binormalVector;

//spline points, cross-section points, and wood part
std::vector<splinePoint> linePoint;
std::vector<glm::vec3> surroundPoint;
std::vector<glm::vec3> woodPoint;

//parallel line and t-shape
std::vector<splinePoint> paraLinePoint;
std::vector<glm::vec3> paraSurroundPoint;
std::vector<glm::vec3> paraTangentVector;
std::vector<glm::vec3> paraNormalVector;
std::vector<glm::vec3> paraBinormalVector;

//Tshape
std::vector<glm::vec3> TShapePoint;
std::vector<glm::vec3> paraTShapePoint;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//lighting
glm::vec3 lightPos(20.0f, 20.0f, 20.0f);


/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't find this file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);
	printf("%d\n", g_iNumOfSplines);
	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

void resize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		updatecamerapos = true;
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	cv::Mat3b image;
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		image = cv::imread(faces[i]);
		if (image.data){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.ptr()
			);
		}
		else{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	cv::Mat3b image;
	image = cv::imread(path);
	if (image.data)
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.ptr());
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}

	return textureID;
}

void create_spline_brute(std::vector<splinePoint> points)
{
	float s = 0.5;
	float basis[] = {
		-s, 2 - s, s - 2, s,
		2 * s, s - 3, 3 - 2 * s, -s,
		-s, 0, s, 0,
		0, 1, 0, 0
	};
	for (unsigned int i = 0; i < points.size() - 3; i++) {
		for (float u = 0; u <= 1; u += 0.001) {
			float temp[] = { u * u * u, u * u, u, 1 };
			cv::Mat tempMatrix(1, 4, CV_32F, temp);
			cv::Mat basisMatrix(4, 4, CV_32F, basis);
			float controlPoints[] = {
				points[i].x, points[i].y, points[i].z,
				points[i + 1].x, points[i + 1].y, points[i + 1].z,
				points[i + 2].x, points[i + 2].y, points[i + 2].z,
				points[i + 3].x, points[i + 3].y, points[i + 3].z,
			};
			cv::Mat controlMatrix(4, 3, CV_32F, controlPoints);
			cv::Mat finalMatrix = tempMatrix * basisMatrix * controlMatrix;
			//create point
			splinePoint point(finalMatrix.at<float>(0, 0), finalMatrix.at<float>(0, 1), finalMatrix.at<float>(0, 2));
			linePoint.push_back(point);

			//calculate tangent vector
			float tangent[] = { 3 * u * u, 2 * u, 1, 0 };
			cv::Mat tangentMatrix(4, 4, CV_32F, tangent);
			cv::Mat	tangentFinalMatrix = tangentMatrix * basisMatrix * controlMatrix;
			glm::vec3 tanVec(tangentFinalMatrix.at<float>(0, 0), tangentFinalMatrix.at<float>(0, 1), tangentFinalMatrix.at<float>(0, 2));
			tangentVector.push_back(glm::normalize(tanVec));
		}
	}

	for (int i = 0; i < tangentVector.size(); i++) {
		if (i == 0) {
			glm::vec3 arbitrary(tangentVector[i].x+1, tangentVector[i].y, tangentVector[i].z);
			glm::vec3 Normal = glm::normalize(glm::cross(tangentVector[i], arbitrary));
			glm::vec3 Binormal = glm::normalize(glm::cross(tangentVector[i], Normal));
			normalVector.push_back(Normal);
			binormalVector.push_back(Binormal);
		}
		else {
			glm::vec3 Normal = glm::normalize(glm::cross( binormalVector[i-1],tangentVector[i]));
			glm::vec3 Binormal = glm::normalize(glm::cross(tangentVector[i], Normal));
			normalVector.push_back(Normal);
			binormalVector.push_back(Binormal);
		}
	}
}

void generateDataForCrossSection()
{
	//calculate all points for cross-section
	for (int i = 0; i < linePoint.size() - 1; i++) {
		splinePoint p0 = linePoint[i];
		splinePoint p1 = linePoint[i + 1];
		//generate eight rounding points
		float alpha = 0.01f;
		float beta = 2.0f; // to stratch x longer
		splinePoint v0(p0.getPosition() + alpha * (-normalVector[i] + beta * binormalVector[i]));
		splinePoint v1(p0.getPosition() + alpha * (normalVector[i] + beta *binormalVector[i]));
		splinePoint v2(p0.getPosition() + alpha * (normalVector[i] - beta *binormalVector[i]));
		splinePoint v3(p0.getPosition() + alpha * (-normalVector[i] - beta *binormalVector[i]));

		splinePoint v4(p1.getPosition() + alpha * (-normalVector[i + 1] + beta *binormalVector[i + 1]));
		splinePoint v5(p1.getPosition() + alpha * (normalVector[i + 1] + beta *binormalVector[i + 1]));
		splinePoint v6(p1.getPosition() + alpha * (normalVector[i + 1] - beta *binormalVector[i + 1]));
		splinePoint v7(p1.getPosition() + alpha * (-normalVector[i + 1] - beta *binormalVector[i + 1]));

		//compute normals
		glm::vec3 v1v5 = v5.getPosition() - v1.getPosition();
		glm::vec3 v1v0 = v0.getPosition() - v1.getPosition();
		glm::vec3 v1v2 = v2.getPosition() - v1.getPosition();
		glm::vec3 v2v6 = v6.getPosition() - v2.getPosition();
		glm::vec3 v2v3 = v3.getPosition() - v2.getPosition();
		glm::vec3 v3v7 = v7.getPosition() - v3.getPosition();
		glm::vec3 v3v0 = v0.getPosition() - v3.getPosition();
		glm::vec3 rightNormal = glm::normalize(glm::cross(v1v0, v1v5));
		glm::vec3 upNormal = glm::normalize(glm::cross(v1v5, v1v2));
		glm::vec3 leftNormal = glm::normalize(glm::cross(v2v6, v2v3));
		glm::vec3 downNormal = glm::normalize(glm::cross(v3v7, v3v0));


		//pushback to surrendPoint: 4 faces/8 triangles for each cube
		//position                                 //normal
		//right
		surroundPoint.push_back(v1.getPosition()); surroundPoint.push_back(rightNormal);
		surroundPoint.push_back(v5.getPosition()); surroundPoint.push_back(rightNormal);
		surroundPoint.push_back(v4.getPosition()); surroundPoint.push_back(rightNormal);
		surroundPoint.push_back(v4.getPosition()); surroundPoint.push_back(rightNormal);
		surroundPoint.push_back(v0.getPosition()); surroundPoint.push_back(rightNormal);
		surroundPoint.push_back(v1.getPosition()); surroundPoint.push_back(rightNormal);
		//left
		surroundPoint.push_back(v2.getPosition()); surroundPoint.push_back(leftNormal);
		surroundPoint.push_back(v6.getPosition()); surroundPoint.push_back(leftNormal);
		surroundPoint.push_back(v7.getPosition()); surroundPoint.push_back(leftNormal);
		surroundPoint.push_back(v7.getPosition()); surroundPoint.push_back(leftNormal);
		surroundPoint.push_back(v3.getPosition()); surroundPoint.push_back(leftNormal);
		surroundPoint.push_back(v2.getPosition()); surroundPoint.push_back(leftNormal);
		//up
		surroundPoint.push_back(v6.getPosition()); surroundPoint.push_back(upNormal);
		surroundPoint.push_back(v5.getPosition()); surroundPoint.push_back(upNormal);
		surroundPoint.push_back(v1.getPosition()); surroundPoint.push_back(upNormal);
		surroundPoint.push_back(v1.getPosition()); surroundPoint.push_back(upNormal);
		surroundPoint.push_back(v2.getPosition()); surroundPoint.push_back(upNormal);
		surroundPoint.push_back(v6.getPosition()); surroundPoint.push_back(upNormal);
		//down
		surroundPoint.push_back(v7.getPosition()); surroundPoint.push_back(downNormal);
		surroundPoint.push_back(v4.getPosition()); surroundPoint.push_back(downNormal);
		surroundPoint.push_back(v0.getPosition()); surroundPoint.push_back(downNormal);
		surroundPoint.push_back(v0.getPosition()); surroundPoint.push_back(downNormal);
		surroundPoint.push_back(v3.getPosition()); surroundPoint.push_back(downNormal);
		surroundPoint.push_back(v7.getPosition()); surroundPoint.push_back(downNormal);
	}
}

void createParallelSpline()
{
	float maxHeight = 0.0f;
	for (int i = 0; i < linePoint.size(); i++) {
		splinePoint p0 = linePoint[i];
		if (p0.y > maxHeight) {
			maxHeight = p0.y;
		}
		glm::vec3 binormal = 0.5f*binormalVector[i];
		splinePoint p1(p0.getPosition() + binormal);
		paraLinePoint.push_back(p1);
	}
	hMax = maxHeight; // compute the max height
}

void createParallelCrossSection()
{
	//calculate all points for cross-section
	for (int i = 0; i < paraLinePoint.size() - 1; i++) {
		splinePoint p0 = paraLinePoint[i];
		splinePoint p1 = paraLinePoint[i + 1];
		//generate eight rounding points
		float alpha = 0.01f;
		float beta = 2.0f; // to stratch x longer
		splinePoint v0(p0.getPosition() + alpha * (-normalVector[i] + beta * binormalVector[i]));
		splinePoint v1(p0.getPosition() + alpha * (normalVector[i] + beta * binormalVector[i]));
		splinePoint v2(p0.getPosition() + alpha * (normalVector[i] - beta * binormalVector[i]));
		splinePoint v3(p0.getPosition() + alpha * (-normalVector[i] - beta * binormalVector[i]));

		splinePoint v4(p1.getPosition() + alpha * (-normalVector[i + 1] + beta * binormalVector[i + 1]));
		splinePoint v5(p1.getPosition() + alpha * (normalVector[i + 1] + beta * binormalVector[i + 1]));
		splinePoint v6(p1.getPosition() + alpha * (normalVector[i + 1] - beta * binormalVector[i + 1]));
		splinePoint v7(p1.getPosition() + alpha * (-normalVector[i + 1] - beta * binormalVector[i + 1]));

		//compute normals
		glm::vec3 v1v5 = v5.getPosition() - v1.getPosition();
		glm::vec3 v1v0 = v0.getPosition() - v1.getPosition();
		glm::vec3 v1v2 = v2.getPosition() - v1.getPosition();
		glm::vec3 v2v6 = v6.getPosition() - v2.getPosition();
		glm::vec3 v2v3 = v3.getPosition() - v2.getPosition();
		glm::vec3 v3v7 = v7.getPosition() - v3.getPosition();
		glm::vec3 v3v0 = v0.getPosition() - v3.getPosition();
		glm::vec3 rightNormal = glm::normalize(glm::cross(v1v0, v1v5));
		glm::vec3 upNormal = glm::normalize(glm::cross(v1v5, v1v2));
		glm::vec3 leftNormal = glm::normalize(glm::cross(v2v6, v2v3));
		glm::vec3 downNormal = glm::normalize(glm::cross(v3v7, v3v0));


		//pushback to surrendPoint: 4 faces/8 triangles for each cube
		//position                                 //normal
		//right
		paraSurroundPoint.push_back(v1.getPosition()); paraSurroundPoint.push_back(rightNormal);
		paraSurroundPoint.push_back(v5.getPosition()); paraSurroundPoint.push_back(rightNormal);
		paraSurroundPoint.push_back(v4.getPosition()); paraSurroundPoint.push_back(rightNormal);
		paraSurroundPoint.push_back(v4.getPosition()); paraSurroundPoint.push_back(rightNormal);
		paraSurroundPoint.push_back(v0.getPosition()); paraSurroundPoint.push_back(rightNormal);
		paraSurroundPoint.push_back(v1.getPosition()); paraSurroundPoint.push_back(rightNormal);
		//left
		paraSurroundPoint.push_back(v2.getPosition()); paraSurroundPoint.push_back(leftNormal);
		paraSurroundPoint.push_back(v6.getPosition()); paraSurroundPoint.push_back(leftNormal);
		paraSurroundPoint.push_back(v7.getPosition()); paraSurroundPoint.push_back(leftNormal);
		paraSurroundPoint.push_back(v7.getPosition()); paraSurroundPoint.push_back(leftNormal);
		paraSurroundPoint.push_back(v3.getPosition()); paraSurroundPoint.push_back(leftNormal);
		paraSurroundPoint.push_back(v2.getPosition()); paraSurroundPoint.push_back(leftNormal);
		//up
		paraSurroundPoint.push_back(v6.getPosition()); paraSurroundPoint.push_back(upNormal);
		paraSurroundPoint.push_back(v5.getPosition()); paraSurroundPoint.push_back(upNormal);
		paraSurroundPoint.push_back(v1.getPosition()); paraSurroundPoint.push_back(upNormal);
		paraSurroundPoint.push_back(v1.getPosition()); paraSurroundPoint.push_back(upNormal);
		paraSurroundPoint.push_back(v2.getPosition()); paraSurroundPoint.push_back(upNormal);
		paraSurroundPoint.push_back(v6.getPosition()); paraSurroundPoint.push_back(upNormal);
		//down
		paraSurroundPoint.push_back(v7.getPosition()); paraSurroundPoint.push_back(downNormal);
		paraSurroundPoint.push_back(v4.getPosition()); paraSurroundPoint.push_back(downNormal);
		paraSurroundPoint.push_back(v0.getPosition()); paraSurroundPoint.push_back(downNormal);
		paraSurroundPoint.push_back(v0.getPosition()); paraSurroundPoint.push_back(downNormal);
		paraSurroundPoint.push_back(v3.getPosition()); paraSurroundPoint.push_back(downNormal);
		paraSurroundPoint.push_back(v7.getPosition()); paraSurroundPoint.push_back(downNormal);
	}
}

void createWoodPart()
{
	for (int i = 5; i < linePoint.size() - 15; i+=15) {
		splinePoint p0 = linePoint[i];
		glm::vec3 binormal = 0.5f*binormalVector[i];
		splinePoint p1(p0.getPosition() + binormal);

		//generate eight rounding points
		float alpha = 0.01;
		splinePoint v0(p0.getPosition() + alpha * (-normalVector[i] + tangentVector[i]));
		splinePoint v1(p0.getPosition() + alpha * (normalVector[i] + tangentVector[i]));
		splinePoint v2(p0.getPosition() + alpha * (normalVector[i] - tangentVector[i]));
		splinePoint v3(p0.getPosition() + alpha * (-normalVector[i] - tangentVector[i]));

		splinePoint v4(p1.getPosition() + alpha * (-normalVector[i] + tangentVector[i]));
		splinePoint v5(p1.getPosition() + alpha * (normalVector[i] + tangentVector[i]));
		splinePoint v6(p1.getPosition() + alpha * (normalVector[i] - tangentVector[i]));
		splinePoint v7(p1.getPosition() + alpha * (-normalVector[i] - tangentVector[i]));

		glm::vec3 v1v5 = v5.getPosition() - v1.getPosition();
		glm::vec3 v1v0 = v0.getPosition() - v1.getPosition();
		glm::vec3 v1v2 = v2.getPosition() - v1.getPosition();
		glm::vec3 v2v6 = v6.getPosition() - v2.getPosition();
		glm::vec3 v2v3 = v3.getPosition() - v2.getPosition();
		glm::vec3 v3v7 = v7.getPosition() - v3.getPosition();
		glm::vec3 v3v0 = v0.getPosition() - v3.getPosition();
		glm::vec3 rightNormal = glm::normalize(glm::cross(v1v0, v1v5));
		glm::vec3 upNormal = glm::normalize(glm::cross(v1v5, v1v2));
		glm::vec3 leftNormal = glm::normalize(glm::cross(v2v6, v2v3));
		glm::vec3 downNormal = glm::normalize(glm::cross(v3v7, v3v0));

		//right
		woodPoint.push_back(v1.getPosition()); woodPoint.push_back(rightNormal);
		woodPoint.push_back(v5.getPosition()); woodPoint.push_back(rightNormal);
		woodPoint.push_back(v4.getPosition()); woodPoint.push_back(rightNormal);
		woodPoint.push_back(v4.getPosition()); woodPoint.push_back(rightNormal);
		woodPoint.push_back(v0.getPosition()); woodPoint.push_back(rightNormal);
		woodPoint.push_back(v1.getPosition()); woodPoint.push_back(rightNormal);
		//left
		woodPoint.push_back(v2.getPosition()); woodPoint.push_back(leftNormal);
		woodPoint.push_back(v6.getPosition()); woodPoint.push_back(leftNormal);
		woodPoint.push_back(v7.getPosition()); woodPoint.push_back(leftNormal);
		woodPoint.push_back(v7.getPosition()); woodPoint.push_back(leftNormal);
		woodPoint.push_back(v3.getPosition()); woodPoint.push_back(leftNormal);
		woodPoint.push_back(v2.getPosition()); woodPoint.push_back(leftNormal);
		//up
		woodPoint.push_back(v6.getPosition()); woodPoint.push_back(upNormal);
		woodPoint.push_back(v5.getPosition()); woodPoint.push_back(upNormal);
		woodPoint.push_back(v1.getPosition()); woodPoint.push_back(upNormal);
		woodPoint.push_back(v1.getPosition()); woodPoint.push_back(upNormal);
		woodPoint.push_back(v2.getPosition()); woodPoint.push_back(upNormal);
		woodPoint.push_back(v6.getPosition()); woodPoint.push_back(upNormal);
		//down
		woodPoint.push_back(v7.getPosition()); woodPoint.push_back(downNormal);
		woodPoint.push_back(v4.getPosition()); woodPoint.push_back(downNormal);
		woodPoint.push_back(v0.getPosition()); woodPoint.push_back(downNormal);
		woodPoint.push_back(v0.getPosition()); woodPoint.push_back(downNormal);
		woodPoint.push_back(v3.getPosition()); woodPoint.push_back(downNormal);
		woodPoint.push_back(v7.getPosition()); woodPoint.push_back(downNormal);
	}
}

void createTshape()
{
	//calculate all points for cross-section
	for (int i = 0; i < linePoint.size() - 1; i++) {
		splinePoint p0 = linePoint[i];
		splinePoint p1 = linePoint[i + 1];
		//generate eight rounding points
		float alpha = 0.01f;
		float beta = 4.0f; // to stratch x longer
		float gama = 0.3f; 
		splinePoint v0(p0.getPosition() + alpha * gama * (-normalVector[i] + binormalVector[i]));
		splinePoint v1(p0.getPosition() + alpha * (beta * normalVector[i] + gama * binormalVector[i]));
		splinePoint v2(p0.getPosition() + alpha * (beta * normalVector[i] - gama * binormalVector[i]));
		splinePoint v3(p0.getPosition() + alpha * gama *(-normalVector[i] - binormalVector[i]));

		splinePoint v4(p1.getPosition() + alpha * gama * (-normalVector[i + 1] + binormalVector[i + 1]));
		splinePoint v5(p1.getPosition() + alpha * (beta * normalVector[i + 1] + gama * binormalVector[i + 1]));
		splinePoint v6(p1.getPosition() + alpha * (beta * normalVector[i + 1] - gama * binormalVector[i + 1]));
		splinePoint v7(p1.getPosition() + alpha * gama * (-normalVector[i + 1] - binormalVector[i + 1]));

		//compute normals
		glm::vec3 v1v5 = v5.getPosition() - v1.getPosition();
		glm::vec3 v1v0 = v0.getPosition() - v1.getPosition();
		glm::vec3 v1v2 = v2.getPosition() - v1.getPosition();
		glm::vec3 v2v6 = v6.getPosition() - v2.getPosition();
		glm::vec3 v2v3 = v3.getPosition() - v2.getPosition();
		glm::vec3 v3v7 = v7.getPosition() - v3.getPosition();
		glm::vec3 v3v0 = v0.getPosition() - v3.getPosition();
		glm::vec3 rightNormal = glm::normalize(glm::cross(v1v0, v1v5));
		glm::vec3 upNormal = glm::normalize(glm::cross(v1v5, v1v2));
		glm::vec3 leftNormal = glm::normalize(glm::cross(v2v6, v2v3));
		glm::vec3 downNormal = glm::normalize(glm::cross(v3v7, v3v0));

		//position                                 //normal
		//right
		TShapePoint.push_back(v1.getPosition()); TShapePoint.push_back(rightNormal);
		TShapePoint.push_back(v5.getPosition()); TShapePoint.push_back(rightNormal);
		TShapePoint.push_back(v4.getPosition()); TShapePoint.push_back(rightNormal);
		TShapePoint.push_back(v4.getPosition()); TShapePoint.push_back(rightNormal);
		TShapePoint.push_back(v0.getPosition()); TShapePoint.push_back(rightNormal);
		TShapePoint.push_back(v1.getPosition()); TShapePoint.push_back(rightNormal);
		//left
		TShapePoint.push_back(v2.getPosition()); TShapePoint.push_back(leftNormal);
		TShapePoint.push_back(v6.getPosition()); TShapePoint.push_back(leftNormal);
		TShapePoint.push_back(v7.getPosition()); TShapePoint.push_back(leftNormal);
		TShapePoint.push_back(v7.getPosition()); TShapePoint.push_back(leftNormal);
		TShapePoint.push_back(v3.getPosition()); TShapePoint.push_back(leftNormal);
		TShapePoint.push_back(v2.getPosition()); TShapePoint.push_back(leftNormal);
		//up
		TShapePoint.push_back(v6.getPosition()); TShapePoint.push_back(upNormal);
		TShapePoint.push_back(v5.getPosition()); TShapePoint.push_back(upNormal);
		TShapePoint.push_back(v1.getPosition()); TShapePoint.push_back(upNormal);
		TShapePoint.push_back(v1.getPosition()); TShapePoint.push_back(upNormal);
		TShapePoint.push_back(v2.getPosition()); TShapePoint.push_back(upNormal);
		TShapePoint.push_back(v6.getPosition()); TShapePoint.push_back(upNormal);
		//down
		TShapePoint.push_back(v7.getPosition()); TShapePoint.push_back(downNormal);
		TShapePoint.push_back(v4.getPosition()); TShapePoint.push_back(downNormal);
		TShapePoint.push_back(v0.getPosition()); TShapePoint.push_back(downNormal);
		TShapePoint.push_back(v0.getPosition()); TShapePoint.push_back(downNormal);
		TShapePoint.push_back(v3.getPosition()); TShapePoint.push_back(downNormal);
		TShapePoint.push_back(v7.getPosition()); TShapePoint.push_back(downNormal);
	}
}

void createParallelTshape()
{
	//calculate all points for cross-section
	for (int i = 0; i < paraLinePoint.size() - 1; i++) {
		splinePoint p0 = paraLinePoint[i];
		splinePoint p1 = paraLinePoint[i + 1];
		//generate eight rounding points
		float alpha = 0.01f;
		float beta = 4.0f; // to stratch x longer
		float gama = 0.3f;
		splinePoint v0(p0.getPosition() + alpha * gama * (-normalVector[i] + binormalVector[i]));
		splinePoint v1(p0.getPosition() + alpha * (beta * normalVector[i] + gama * binormalVector[i]));
		splinePoint v2(p0.getPosition() + alpha * (beta * normalVector[i] - gama * binormalVector[i]));
		splinePoint v3(p0.getPosition() + alpha * gama *(-normalVector[i] - binormalVector[i]));

		splinePoint v4(p1.getPosition() + alpha * gama * (-normalVector[i + 1] + binormalVector[i + 1]));
		splinePoint v5(p1.getPosition() + alpha * (beta * normalVector[i + 1] + gama * binormalVector[i + 1]));
		splinePoint v6(p1.getPosition() + alpha * (beta * normalVector[i + 1] - gama * binormalVector[i + 1]));
		splinePoint v7(p1.getPosition() + alpha * gama * (-normalVector[i + 1] - binormalVector[i + 1]));

		//compute normals
		glm::vec3 v1v5 = v5.getPosition() - v1.getPosition();
		glm::vec3 v1v0 = v0.getPosition() - v1.getPosition();
		glm::vec3 v1v2 = v2.getPosition() - v1.getPosition();
		glm::vec3 v2v6 = v6.getPosition() - v2.getPosition();
		glm::vec3 v2v3 = v3.getPosition() - v2.getPosition();
		glm::vec3 v3v7 = v7.getPosition() - v3.getPosition();
		glm::vec3 v3v0 = v0.getPosition() - v3.getPosition();
		glm::vec3 rightNormal = glm::normalize(glm::cross(v1v0, v1v5));
		glm::vec3 upNormal = glm::normalize(glm::cross(v1v5, v1v2));
		glm::vec3 leftNormal = glm::normalize(glm::cross(v2v6, v2v3));
		glm::vec3 downNormal = glm::normalize(glm::cross(v3v7, v3v0));

		//position                                 //normal
		//right
		paraTShapePoint.push_back(v1.getPosition()); paraTShapePoint.push_back(rightNormal);
		paraTShapePoint.push_back(v5.getPosition()); paraTShapePoint.push_back(rightNormal);
		paraTShapePoint.push_back(v4.getPosition()); paraTShapePoint.push_back(rightNormal);
		paraTShapePoint.push_back(v4.getPosition()); paraTShapePoint.push_back(rightNormal);
		paraTShapePoint.push_back(v0.getPosition()); paraTShapePoint.push_back(rightNormal);
		paraTShapePoint.push_back(v1.getPosition()); paraTShapePoint.push_back(rightNormal);
		//left
		paraTShapePoint.push_back(v2.getPosition()); paraTShapePoint.push_back(leftNormal);
		paraTShapePoint.push_back(v6.getPosition()); paraTShapePoint.push_back(leftNormal);
		paraTShapePoint.push_back(v7.getPosition()); paraTShapePoint.push_back(leftNormal);
		paraTShapePoint.push_back(v7.getPosition()); paraTShapePoint.push_back(leftNormal);
		paraTShapePoint.push_back(v3.getPosition()); paraTShapePoint.push_back(leftNormal);
		paraTShapePoint.push_back(v2.getPosition()); paraTShapePoint.push_back(leftNormal);
		//up
		paraTShapePoint.push_back(v6.getPosition()); paraTShapePoint.push_back(upNormal);
		paraTShapePoint.push_back(v5.getPosition()); paraTShapePoint.push_back(upNormal);
		paraTShapePoint.push_back(v1.getPosition()); paraTShapePoint.push_back(upNormal);
		paraTShapePoint.push_back(v1.getPosition()); paraTShapePoint.push_back(upNormal);
		paraTShapePoint.push_back(v2.getPosition()); paraTShapePoint.push_back(upNormal);
		paraTShapePoint.push_back(v6.getPosition()); paraTShapePoint.push_back(upNormal);
		//down
		paraTShapePoint.push_back(v7.getPosition()); paraTShapePoint.push_back(downNormal);
		paraTShapePoint.push_back(v4.getPosition()); paraTShapePoint.push_back(downNormal);
		paraTShapePoint.push_back(v0.getPosition()); paraTShapePoint.push_back(downNormal);
		paraTShapePoint.push_back(v0.getPosition()); paraTShapePoint.push_back(downNormal);
		paraTShapePoint.push_back(v3.getPosition()); paraTShapePoint.push_back(downNormal);
		paraTShapePoint.push_back(v7.getPosition()); paraTShapePoint.push_back(downNormal);
	}
}

void UpdateCamera(int pos) {
	camera.MoveAlongSpline(linePoint[pos], tangentVector[pos], normalVector[pos], binormalVector[pos]);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}
	loadSplines(argv[1]);
	std::vector<splinePoint> points;
	for (int i = 0; i < g_Splines->numControlPoints; i++) {
		splinePoint temp(g_Splines->points[i].x, g_Splines->points[i].y, g_Splines->points[i].z);
		points.push_back(temp);
	}

	//using glfw to open a window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//for MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assign2", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//set up glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//setup viewport
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//setup resize callback
	glfwSetFramebufferSizeCallback(window, resize);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// build and compile our shader program
	// ------------------------------------
	//Shader lightingShader("shaders/VertexShader.glsl", "shaders/FragmentShader.glsl");
	//Shader lightCubeShader("shaders/cubeVS.glsl", "shaders/cubeFS.glsl");
	Shader shader("shaders/cubemapvs.glsl", "shaders/cubemapfs.glsl");
	Shader floarShader("shaders/simplevs.glsl", "shaders/simplefs.glsl");
	Shader skyboxShader("shaders/skyboxVS.glsl", "shaders/skyboxFS.glsl");
	Shader railwayShader("shaders/enviroMapVS.glsl", "shaders/enviroMapFS.glsl");
	Shader lightingShader("shaders/cubeVS.glsl", "shaders/cubeFS.glsl");
	Shader lightCubeShader("shaders/simplevs.glsl", "shaders/simplefs.glsl");

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//floar VAO
	float floarVertices[] = {
		-5, -0.5, 5,
		-5, -0.5, -5,
		5, -0.5, 5,
		5, -0.5, -5
	};
	unsigned int floarVAO, floarVBO;
	glGenVertexArrays(1, &floarVAO);
	glGenBuffers(1, &floarVBO);
	glBindVertexArray(floarVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floarVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floarVertices), &floarVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//spline
	splinePoint point1(1.0, 2.0, 0.0);
	splinePoint point2(6.0, 2.0, 0.0);
	splinePoint point3(11.0, 2.0, 0.0);
	splinePoint point4(13.0, 1.0, 0.0);
	splinePoint point5(16.0, 0.0, 0.0);
	splinePoint point6(18.0, 0.0, 3.0);
	splinePoint point7(18.0, 4.0, 8.0);
	splinePoint point8(18.0, 0.0, 12.0);
	splinePoint point9(18.0, 4.0, 20.0);
	splinePoint point10(21.0, 3.0, 23.0);
	splinePoint point11(18.0, 6.0, 26.0);
	splinePoint point12(15.0, 3.0, 29.0);
	splinePoint point13(18.0, 6.0, 32.0);
	splinePoint point14(18.0, 0.0, 38.0);
	splinePoint point15(16.0, 0.0, 42.0);
	//points = {point4, point5, point6, point7, point8, point9, point10, point11, point12};
	create_spline_brute(points);
	unsigned int splineVAO, splineVBO;
	glGenVertexArrays(1, &splineVAO);
	glGenBuffers(1, &splineVBO);
	glBindVertexArray(splineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, splineVBO);
	glBufferData(GL_ARRAY_BUFFER, linePoint.size() * sizeof(splinePoint), linePoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(splinePoint), (void*)0);
	

	//cross-section
	generateDataForCrossSection();
	unsigned int crossVAO, crossVBO;
	glGenVertexArrays(1, &crossVAO);
	glGenBuffers(1, &crossVBO);
	glBindVertexArray(crossVAO);
	glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
	glBufferData(GL_ARRAY_BUFFER, surroundPoint.size() * 3 * sizeof(float), surroundPoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	
	//parallel cross-section
	createParallelSpline();
	createParallelCrossSection();
	unsigned int paraVAO, paraVBO;
	glGenVertexArrays(1, &paraVAO);
	glGenBuffers(1, &paraVBO);
	glBindVertexArray(paraVAO);
	glBindBuffer(GL_ARRAY_BUFFER, paraVBO);
	glBufferData(GL_ARRAY_BUFFER, paraSurroundPoint.size() * 3 * sizeof(float), paraSurroundPoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	//wood part of cross-section
	createWoodPart();
	unsigned int woodVAO, woodVBO;
	glGenVertexArrays(1, &woodVAO);
	glGenBuffers(1, &woodVBO);
	glBindVertexArray(woodVAO);
	glBindBuffer(GL_ARRAY_BUFFER, woodVBO);
	glBufferData(GL_ARRAY_BUFFER, woodPoint.size() * 3 * sizeof(float), woodPoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	//create T-shape part
	createTshape();
	unsigned int tshapeVAO, tshapeVBO;
	glGenVertexArrays(1, &tshapeVAO);
	glGenBuffers(1, &tshapeVBO);
	glBindVertexArray(tshapeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, tshapeVBO);
	glBufferData(GL_ARRAY_BUFFER, TShapePoint.size() * 3 * sizeof(float), TShapePoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	//create para T-shape part
	createParallelTshape();
	unsigned int paratshapeVAO, paratshapeVBO;
	glGenVertexArrays(1, &paratshapeVAO);
	glGenBuffers(1, &paratshapeVBO);
	glBindVertexArray(paratshapeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, paratshapeVBO);
	glBufferData(GL_ARRAY_BUFFER, paraTShapePoint.size() * 3 * sizeof(float), paraTShapePoint.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	//create light cube
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// load textures (we now use a utility function to keep the code more organized)
	// -----------------------------------------------------------------------------
	/*Texture diffuseMap("LightMap/container2.png") ;
	Texture specularMap("LightMap/container2_specular.png");
	Texture cubeTexture("Textures/container.jpg");*/

	std::vector<std::string> faces{
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	unsigned int cubetexture = loadTexture("Textures/container.jpg");

	// shader configuration
	// --------------------
	shader.SetAcive();
	shader.SetInt("texture1", 0);

	skyboxShader.SetAcive();
	skyboxShader.SetInt("skybox", 0);

	railwayShader.SetAcive();
	railwayShader.SetInt("skybox", 0);

	//which pos to move the camera
	int pos = 0;
	//locate camera to starting point
	UpdateCamera(0);

	//setup rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//process input
		processInput(window);

		//rendering stuff
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw scene as normal

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		//shader.SetAcive();
		//shader.SetMat4("model", model);
		//shader.SetMat4("view", view);
		//shader.SetMat4("projection", projection);

		//draw surrounding cross-section
		railwayShader.SetAcive();
		railwayShader.SetMat4("model", model);
		railwayShader.SetMat4("view", view);
		railwayShader.SetMat4("projection", projection);
		railwayShader.SetVec3("cameraPos", camera.Position);
		glBindVertexArray(crossVAO);
		glDrawArrays(GL_TRIANGLES, 0, surroundPoint.size());

		//draw parallel surrounding cross-section
		railwayShader.SetAcive();
		railwayShader.SetMat4("model", model);
		railwayShader.SetMat4("view", view);
		railwayShader.SetMat4("projection", projection);
		railwayShader.SetVec3("cameraPos", camera.Position);
		glBindVertexArray(paraVAO);
		glDrawArrays(GL_TRIANGLES, 0, paraSurroundPoint.size());

		//draw T-shape on original cross-section
		railwayShader.SetAcive();
		railwayShader.SetMat4("model", model);
		railwayShader.SetMat4("view", view);
		railwayShader.SetMat4("projection", projection);
		railwayShader.SetVec3("cameraPos", camera.Position);
		glBindVertexArray(tshapeVAO);
		glDrawArrays(GL_TRIANGLES, 0, TShapePoint.size());

		//draw para T-shape on original cross-section
		railwayShader.SetAcive();
		railwayShader.SetMat4("model", model);
		railwayShader.SetMat4("view", view);
		railwayShader.SetMat4("projection", projection);
		railwayShader.SetVec3("cameraPos", camera.Position);
		glBindVertexArray(paratshapeVAO);
		glDrawArrays(GL_TRIANGLES, 0, paraTShapePoint.size());

		//draw wood part of cross-section
		lightingShader.SetAcive();
		//light property
		glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
		glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);
		lightingShader.SetVec3("light.direction", direction);
		lightingShader.SetVec3("viewPos", camera.Position);
		lightingShader.SetVec3("light.ambient", ambientColor);
		lightingShader.SetVec3("light.diffuse", diffuseColor);
		lightingShader.SetVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f)); 
		lightingShader.SetVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
		lightingShader.SetVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
		lightingShader.SetVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f)); // specular lighting doesn't have full effect on this object's material
		lightingShader.SetFloat("material.shininess", 32.0f);
		//mvp
		lightingShader.SetMat4("projection", projection);
		lightingShader.SetMat4("view", view);
		lightingShader.SetMat4("model", model);
		glBindVertexArray(woodVAO);
		glDrawArrays(GL_TRIANGLES, 0, woodPoint.size());

		//draw light cube
		lightCubeShader.SetAcive();
		lightCubeShader.SetMat4("projection", projection);
		lightCubeShader.SetMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lightCubeShader.SetMat4("model", model);

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw skybox as last: open later
		// ---------------------------------------------------------------		
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.SetAcive();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.SetMat4("view", view);
		skyboxShader.SetMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
		
		//update camera position
		if (updatecamerapos && pos < linePoint.size()) {
			float mag = tangentVector[pos].length();
			u_new = u_current + 2*  sqrt(2 * g * (hMax - linePoint[pos].y)) / mag;
			UpdateCamera(pos);
			pos = u_new + (u_new - u_current);
			u_current = u_new + 0.5f;
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------


	// glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------;
	glfwTerminate();
	return 0;
}
