#include <array>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../framework/framework.h"

using namespace std;


class Env
{
public:
	Env(array<string, 6> cubemap, string sh_coef_file)
		:cubemap_(cubemap)
	{
		// load parameters
		ifstream ifs(sh_coef_file);
		if (!ifs)
			throw runtime_error("open " + sh_coef_file + " failed");
		int i = 0;
		float r, g, b;
		while (ifs >> r >> g >> b)
		{
			coefs_.push_back(glm::vec3(r, g, b));
			i++;
		}

	}

	void Init()
	{

		skybox_ = new fw::SkyBox(cubemap_);
	}

	void Shutdown()
	{
		delete skybox_;
	}

	vector<glm::vec3> getCoefficients()const
	{
		return coefs_;
	}

	void Draw(glm::mat4 view, glm::mat4 proj)
	{
		skybox_->Draw(proj*view);
	}
private:
	array<string, 6> cubemap_;
	fw::SkyBox* skybox_;
	vector<glm::vec3> coefs_;
};


const std::string sh_vertex_src = R"(
#version 330 core

uniform mat4 model_view_proj;
uniform mat4 normal_trans;
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT{
	vec3 normal;
}vs;

void main(void) {
	gl_Position = model_view_proj * vec4(position,1);
	//normal = normalize(ciNormalMatrix * ciNormal);
	vs.normal = vec3(normalize(normal_trans * vec4(normal, 0)));
}
)";

const std::string sh_fragment_src = R"(
#version 330

uniform int SH_NUM;

const float PI = 3.1415926535897932384626433832795;

uniform vec3 coef[16];

in VS_OUT{
	vec3 normal;
}vs;

out vec4 color;

void main(void) {
	float basis[16];

	float x = vs.normal.x;
	float y = vs.normal.y;
	float z = vs.normal.z;
	float x2 = x*x;
	float y2 = y*y;
	float z2 = z*z;
    
    basis[0] = 1.f / 2.f * sqrt(1.f / PI);
    basis[1] = sqrt(3.f / (4.f*PI))*z;
    basis[2] = sqrt(3.f / (4.f*PI))*y;
    basis[3] = sqrt(3.f / (4.f*PI))*x;
    basis[4] = 1.f / 2.f * sqrt(15.f / PI) * x * z;
    basis[5] = 1.f / 2.f * sqrt(15.f / PI) * z * y;
    basis[6] = 1.f / 4.f * sqrt(5.f / PI) * (-x*x - z*z + 2 * y*y);
    basis[7] = 1.f / 2.f * sqrt(15.f / PI) * y * x;
    basis[8] = 1.f / 4.f * sqrt(15.f / PI) * (x*x - z*z);
    basis[9] = 1.f / 4.f*sqrt(35.f / (2.f*PI))*(3 * x2 - z2)*z;
    basis[10] = 1.f / 2.f*sqrt(105.f / PI)*x*z*y;
    basis[11] = 1.f / 4.f*sqrt(21.f / (2.f*PI))*z*(4 * y2 - x2 - z2);
    basis[12] = 1.f / 4.f*sqrt(7.f / PI)*y*(2 * y2 - 3 * x2 - 3 * z2);
    basis[13] = 1.f / 4.f*sqrt(21.f / (2.f*PI))*x*(4 * y2 - x2 - z2);
    basis[14] = 1.f / 4.f*sqrt(105.f / PI)*(x2 - z2)*y;
    basis[15] = 1.f / 4.f*sqrt(35.f / (2 * PI))*(x2 - 3 * z2)*x;

	vec3 c = vec3(0,0,0);
	for (int i = 0; i < SH_NUM; i++)
		c += coef[i] * basis[i];
	color = vec4(c, 1);
}
)";

class Object
{
public:
	Object(string objfile)
		:objfile_(objfile)
	{

	}
	void Init()
	{
		model_ = fw::LoadModel(objfile_);
		model_program_ = fw::CreateProgram(sh_vertex_src, sh_fragment_src);
		SetDegree(3);
	}

	void SetDegree(int degree)
	{
		if (degree > 3)
			degree = 3;
		int SH_NUM = (degree + 1)*(degree + 1);
		glUseProgram(model_program_);
		glUniform1i(glGetUniformLocation(model_program_, "SH_NUM"), SH_NUM);
	}

	void Shutdown()
	{
		glDeleteProgram(model_program_);
	}

	void SetMVP(glm::mat4 model_view_proj, glm::mat4 normal_trans)
	{
		glUseProgram(model_program_);

		glUniformMatrix4fv(glGetUniformLocation(model_program_, "model_view_proj"),
			1, false, glm::value_ptr(model_view_proj));
		glUniformMatrix4fv(glGetUniformLocation(model_program_, "normal_trans"),
			1, false, glm::value_ptr(normal_trans));
	}

	void SetLighting(const vector<glm::vec3>& coefs)
	{
		glUseProgram(model_program_);
		glUniform3fv(glGetUniformLocation(model_program_, "coef"),
			coefs.size(), (float*)(coefs.data()));
	}

	void Draw()
	{
		glUseProgram(model_program_);
		model_->Draw(model_program_);
	}
private:
	string objfile_;
	shared_ptr<fw::Model> model_;
	GLuint model_program_;

};


class SHLightingApp
	:public fw::Application {
public:
	SHLightingApp(vector< Env* > envs, vector< Object* > objs)
		:envs_(envs), objs_(objs),
		current_env_(0), current_obj_(0), degree_(3)
	{}

	void SwitchEnv(int step = 1)
	{
		envs_[current_env_]->Shutdown();
		current_env_ = (current_env_ + step + envs_.size()) % envs_.size();
		envs_[current_env_]->Init();
	}
	void SwitchObj(int step = 1)
	{
		objs_[current_obj_]->Shutdown();
		current_obj_ = (current_obj_ + step + objs_.size()) % objs_.size();
		objs_[current_obj_]->Init();
	}

	void SetDegree(int degree) { degree_ = degree; }
private:

	vector< Env* > envs_;
	vector< Object* > objs_;
	int current_env_;
	int current_obj_;
	int degree_;

	class SHInput : public fw::ObserverInput
	{
	public:
		SHInput(SHLightingApp* app, glm::vec3 camera_pos, glm::vec3 camera_up)
			:app_(app), fw::ObserverInput(camera_pos, camera_up)
		{}

	private:
		SHLightingApp* app_;
		virtual void OnKey(int key, int scancode, int action, int mods) override
		{
			if (action == GLFW_PRESS)
			{
				if (key == GLFW_KEY_PAGE_UP)
					app_->SwitchEnv(-1);
				if (key == GLFW_KEY_PAGE_DOWN)
					app_->SwitchEnv(1);
				if (key == GLFW_KEY_UP)
					app_->SwitchObj(-1);
				if (key == GLFW_KEY_DOWN)
					app_->SwitchObj(1);
				if (key == GLFW_KEY_0)
					app_->SetDegree(0);
				if (key == GLFW_KEY_1)
					app_->SetDegree(1);
				if (key == GLFW_KEY_2)
					app_->SetDegree(2);
				if (key == GLFW_KEY_3)
					app_->SetDegree(3);
			}

		}
	};

	SHInput* input_proc_;

	void OnInit() override
	{
		// input proc
		input_proc_ = new SHInput(this, { 3.f, 3.f, 3.f }, { 0.f, 1.f, 0.f });
		this->SetInputProcessor(input_proc_);

		envs_[current_env_]->Init();
		objs_[current_obj_]->Init();

		// setup opengl
		glViewport(0, 0, WindowWidth(), WindowHeight());
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);

	}

	void OnUpdate(float dt) override
	{
		glEnable(GL_DEPTH_TEST);

		glm::mat4 view = input_proc_->GetCameraView();
		glm::mat4 proj = glm::perspective(glm::radians(60.f), FrameRatio(), 0.1f, 100.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		envs_[current_env_]->Draw(view, proj);

		// compute transforms
		glm::mat4 model_trans = input_proc_->GetModelTransform();
		glm::mat4 model_view_proj = proj * view*model_trans;
		glm::mat4 normal_trans = glm::transpose(glm::inverse(model_trans));

		auto coefs = envs_[current_env_]->getCoefficients();
		objs_[current_obj_]->SetMVP(model_view_proj, normal_trans);
		objs_[current_obj_]->SetLighting(coefs);
		objs_[current_obj_]->SetDegree(degree_);
		objs_[current_obj_]->Draw();
	}

	void OnShutdown() override
	{
		delete input_proc_;
		envs_[current_env_]->Shutdown();
		objs_[current_obj_]->Shutdown();


	}
};

int main(int argc, char *argv[])
{

	try {
		if (argc < 5)
			throw invalid_argument("Usage: ./lighting N directory1 format1 ... directoryN formatN M model1 ... modelM");
		int k = 1;
		int N = stoi(argv[k++]);

		vector< Env*> envs(N);
		for(int i = 0; i < N; i++)
		{
			string dir = argv[k++];
			if (dir.back() != '/' && dir.back() != '\\')
				dir += '/';

			string format = argv[k++];
			array<string, 6> faces = { "posx", "negx", "posy", "negy", "posz", "negz" };
			array<string, 6> cube_textures;
			for (int i = 0; i < 6; i++)
				cube_textures[i] = dir + faces[i] + "." + format;
			string sh_coef_file = dir + "coefficients.txt";
			envs[i] = new Env(cube_textures, sh_coef_file);
		}

		int M = stoi(argv[k++]);
		vector< Object* > objs(M);
		for (int i = 0; i < M; i++)
			objs[i] = new Object(string(argv[k++]));

		SHLightingApp app(envs, objs);
		app.SetWindowSize(800, 600);
		app.SetWindowTitle("Spherical Harmonics Lighting");
		app.Run();
		for (auto e : envs)
			delete e;
		for (auto o : objs)
			delete o;
	}
	catch (exception e) {
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}