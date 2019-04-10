#pragma once
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <array>
#include <string>
#include <GL/GLEW.h>
#include <glm/mat4x4.hpp>

namespace fw{

class SkyBox{
public:
	SkyBox(std::array<std::string, 6> textures);
	~SkyBox();
	void Draw(glm::mat4 viewproj);
private:
	SkyBox(const SkyBox&) = delete;
	void operator=(const SkyBox&) = delete;
	GLuint cubemap_;
	GLuint cube_;
	GLuint program_;
};

}// namespace fw

#endif