#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "geometry.h"
#include "graphics.h"

using namespace fw;

namespace{
	static const std::string skybox_vertex_src =
		R"(
			#version 330 core								
			layout(location = 0) in vec3 position;			
			uniform mat4 viewproj;							
			out vec3 TexCoords;							
			void main()
			{									
				vec4 pos = viewproj * vec4(position, 1);	
				gl_Position = pos.xyww;						
				TexCoords = position;
			}
)";
	static const std::string skybox_fragment_src =
		R"(
			#version 330 core							
			uniform samplerCube skybox;					
			in vec3 TexCoords;							
			out vec4 color;								
			void main()
			{								
				color = texture(skybox, TexCoords);
			}
)";	
}

SkyBox::SkyBox(std::array<std::string, 6> textures)
{
	cubemap_ = LoadCubemap(textures);
	cube_ = CreateCube();
	program_ = CreateProgram(skybox_vertex_src, skybox_fragment_src);
}

SkyBox::~SkyBox()
{
	glDeleteProgram(program_);
	glDeleteTextures(1, &cubemap_);
	glDeleteBuffers(1, &cube_);
}

void SkyBox::Draw(glm::mat4 viewproj)
{
	// draw skybox
	// note: the sky box depth is always 1
	glDepthFunc(GL_LEQUAL);
	glUseProgram(program_);
	auto viewproj_pos = glGetUniformLocation(program_, "viewproj");
	auto s = glm::scale(glm::vec3{ 10000.f });
	glUniformMatrix4fv(viewproj_pos, 1, GL_FALSE, glm::value_ptr(viewproj*s));

	glBindVertexArray(cube_);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program_, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}
