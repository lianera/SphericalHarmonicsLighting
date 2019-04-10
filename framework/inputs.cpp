#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include "inputs.h"

using namespace fw;

const float PI = float(M_PI);

ObserverInput::ObserverInput(glm::vec3 camera_position, glm::vec3 camera_up)
:camera_position_(camera_position), camera_up_(camera_up), model_trans_(glm::mat4(1))
{
	if (camera_position_ == glm::vec3{ 0, 0, 0 })
		camera_position_ = { 0.01f, 0, 0 };
	camera_view_ = glm::lookAt(glm::vec3(camera_position_), glm::vec3{ 0, 0, 0 }, camera_up_);
}

void ObserverInput::OnMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT){
		lbutton_down_ = (action == GLFW_PRESS);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT){
		rbutton_down_ = (action == GLFW_PRESS);
	}

}

void ObserverInput::OnMouseMove(float x, float y)
{
	if (rbutton_down_){		// drag scene
		float dx = (x - lastx_)*0.01f;
		float dy = (lasty_ - y)*0.01f;
		auto dir = glm::normalize(camera_position_);
		auto right = normalize(cross(dir, camera_up_));
		auto hrot = glm::rotate(dx, camera_up_);
		auto vrot = glm::rotate(dy, right);
		auto newpos = hrot*vrot*glm::vec4{ camera_position_, 1 };
		auto newdir = glm::normalize(glm::vec3(newpos));
		if (abs(glm::dot(newdir, camera_up_)) < 0.99f){
			camera_position_ = newpos;
			camera_view_ = glm::lookAt(glm::vec3(newpos), glm::vec3{ 0, 0, 0 }, camera_up_);
		}
	}
	if (lbutton_down_){		// drag model
		float dx = (x - lastx_)*0.01f;
		float dy = (lasty_ - y)*0.01f;
		auto dir = glm::normalize(camera_position_);
		auto right = normalize(cross(dir, camera_up_));
		model_trans_ = glm::rotate(dx, camera_up_)*model_trans_;
		model_trans_ = glm::rotate(dy, right)*model_trans_;
	}
	lastx_ = x;
	lasty_ = y;
}


void ObserverInput::OnMouseWheel(float xoffset, float yoffset)
{
	float dw = 1.f - yoffset*0.1f;
	auto newpos = glm::scale(glm::vec3{ dw, dw, dw })*glm::vec4{ camera_position_, 1 };
	float cam_dist = glm::length(newpos);
	if (cam_dist < 1000.f && cam_dist > 0.1f){
		camera_position_ = newpos;
		camera_view_ = glm::lookAt(glm::vec3(newpos), glm::vec3{ 0, 0, 0 }, camera_up_);
	}
}