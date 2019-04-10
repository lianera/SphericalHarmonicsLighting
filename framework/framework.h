#pragma once
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <string>
#include <memory>

#include "inputs.h"
#include "graphics.h"
#include "geometry.h"

struct GLFWwindow;

namespace fw{
	class InputProcessor;

	class Application{	// single instance
	public:
		Application() = default;
		void Run();
		void SetWindowSize(int width, int height)
		{
			window_width_ = width;
			window_height_ = height;
		}
		void SetWindowTitle(std::string title)
		{
			title_ = title;
		}
		void SetInputProcessor(InputProcessor* p)
		{
			input_processor_ = p;
		}
		float FrameRatio();
	protected:
		virtual void OnInit(){}
		virtual void OnUpdate(float dt){}
		virtual void OnShutdown(){}
		int WindowWidth()const{ return window_width_; };
		int WindowHeight()const{ return window_height_; }
		std::string WindowTitle()const{ return title_; }
	private:
		Application(const Application&) = delete;
		void operator=(const Application&) = delete;

		int window_width_ = 640, window_height_ = 480;
		std::string title_ = "GraphicsWorkshop";

		struct DestroyglfwWin{
			void operator()(GLFWwindow* ptr);
		};

		GLFWwindow* window_ = nullptr;
		InputProcessor* input_processor_ = nullptr;
		static Application* instance_;

		float pre_xpos_, pre_ypos_;

		void Init();
		void Shutdown();
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
	};
}// namespace fw

#endif