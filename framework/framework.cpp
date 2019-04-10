#include <GL/glew.h>
#include <GLFW/glfw3.h>  
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include "framework.h"
#include "inputs.h"

using namespace fw;
using namespace std;

Application* Application::instance_ = NULL;

namespace{
	void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}

void Application::Init()
{
	instance_ = this;
	bool glfw_inited = false;
	try{
		GLenum err = glfwInit();
		if (!err){
			throw runtime_error((const char*)glewGetErrorString(err));
		}

		glfw_inited = true;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glEnable(GL_MULTISAMPLE);

		// create window
		window_ = glfwCreateWindow(window_width_, window_height_, title_.c_str(), NULL, NULL);
		if (!window_)
			throw runtime_error("create window failed");

		/* Make the window's context current */
		glfwMakeContextCurrent(window_);

		if (glewInit() != GLEW_OK)
			throw runtime_error("init glew failed");

		// input proc
		//glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glfwSetKeyCallback(window_, KeyCallback);
		glfwSetCursorPosCallback(window_, MouseCallback);
		glfwSetMouseButtonCallback(window_, MouseButtonCallback);
		glfwSetScrollCallback(window_, MouseWheelCallback);

		glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glfwSwapBuffers(window_);
		
		double xpos, ypos;
		glfwGetCursorPos(window_, &xpos, &ypos);
		pre_xpos_ = float(xpos);
		pre_ypos_ = float(ypos);
	}
	catch (const exception& e){
		if (glfw_inited)
			glfwTerminate();
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

void Application::Shutdown()
{
	glfwDestroyWindow(window_);
	glfwTerminate();
}

void Application::Run()
{
	Init();
	this->OnInit();
	double lasttime = glfwGetTime();
	try{
		// the window is closed?
		while (!glfwWindowShouldClose(window_)){
			// time calculate
			double curtime = glfwGetTime();
			double deltatime = curtime - lasttime;
			lasttime = curtime;

			// Poll for and process events
			glfwPollEvents();

			this->OnUpdate(float(deltatime));
			
			// Swap front and back buffers
			glfwSwapBuffers(window_);

		}
	}
	catch (const exception& e){
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	catch (...){
		std::cerr << "unknow error" << std::endl;
		exit(1);
	}
	this->OnShutdown();
	Shutdown();
}

float Application::FrameRatio()
{
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	return float(width) / float(height);
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Application* instance = Application::instance_;
	if (instance->input_processor_)
		instance->input_processor_->OnKey(key, scancode, action, mods);
}


void Application::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Application* instance = Application::instance_;
	if (instance->input_processor_)
		instance->input_processor_->OnMouseMove(float(xpos), float(ypos));
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Application* instance = Application::instance_;
	if (instance->input_processor_)
		instance->input_processor_->OnMouseButton(button, action, mods);

}

void Application::MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Application* instance = Application::instance_;
	if (instance->input_processor_)
		instance->input_processor_->OnMouseWheel(float(xoffset), float(yoffset));
}