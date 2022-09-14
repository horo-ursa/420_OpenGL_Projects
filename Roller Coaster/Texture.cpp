#include "stdafx.h"
#include "Texture.h"

Texture::Texture(char* picPath)
{
	ImageProcess::readImage(picPath, image, false);
	cv::flip(image, image, 0);
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image.cols,
		image.rows,
		0,
		GL_BGR,
		GL_UNSIGNED_BYTE,
		image.ptr()
	);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::SetActive(int& slot){
	std::cout << slot << std::endl;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
	slot++;
}