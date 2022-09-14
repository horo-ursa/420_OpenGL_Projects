#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "ImageProcess.h"


class Texture
{
public:
	unsigned int ID;
	cv::Mat3b image;
	Texture(char* picPath);
	void SetActive(int& slot);
private:

};
