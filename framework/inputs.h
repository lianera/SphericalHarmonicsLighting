#pragma once
#ifndef INPUTS_H
#define INPUTS_H

#include <glm/mat4x4.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace fw{
	class InputProcessor{
	public:
		virtual void OnKey(int key, int scancode, int action, int mods){};
		virtual void OnMouseButton(int button, int action, int mods){};
		virtual void OnMouseMove(float x, float y){};
		virtual void OnMouseWheel(float xoffset, float yoffset){};
	};

	class ObserverInput
		:public InputProcessor{
		glm::vec3 camera_position_;
		glm::vec3 camera_up_;
		glm::mat4 camera_view_;
		glm::mat4 model_trans_;

		bool lbutton_down_ = false;
		bool rbutton_down_ = false;
		float lastx_ = 0.f, lasty_ = 0.f;
	public:
		ObserverInput(glm::vec3 camera_position = { 1, 0, 0 }, glm::vec3 camera_up = { 0, 1, 0 });
		glm::mat4 GetCameraView()const{ return camera_view_; }
		glm::mat4 GetModelTransform()const{ return model_trans_; }
		void OnMouseButton(int button, int action, int mods);
		void OnMouseMove(float x, float y);
		void OnMouseWheel(float xoffset, float yoffset);
	private:
	
	};
}// namespace fw

#endif