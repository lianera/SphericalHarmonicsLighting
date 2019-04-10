#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Graphics.h"

using namespace std;
using namespace fw;

map<string, Texture> Model::loaded_mesh_;

GLuint fw::CreateProgram(string vertex_src, string fragment_src)
{
	std::vector<std::tuple<std::string, GLenum>> shaders;
	shaders.push_back(std::tuple<std::string, GLenum>(vertex_src, GL_VERTEX_SHADER));
	shaders.push_back(std::tuple<std::string, GLenum>(fragment_src, GL_FRAGMENT_SHADER));
	return CreateProgram(shaders);
}

GLuint fw::CreateProgram(vector<tuple<string, GLenum>> shader_src)
{
	// create program
	GLuint program = glCreateProgram();
	vector<GLuint> shaders;
	// setup shader deletion
	auto delete_shaders = [](const vector<GLuint>& shaders){
		for (auto s : shaders){
			glDeleteShader(s);
		}
	};
	// setup compile error checker
	auto check_compile = [](GLint shader, std::string* info)->bool{
		GLint success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE){
			// get info size
			GLint logsize = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

			// get info
			GLchar* errinfo = new GLchar[logsize];
			glGetShaderInfoLog(shader, logsize, &logsize, errinfo);
			*info = std::string(errinfo, errinfo + logsize);
			delete[] errinfo;
			return false;
		}
		return true;
	};

	// read each shader
	for (auto info : shader_src){
		string src = get<0>(info);
		GLenum type = get<1>(info);
		// open file stream

		// create shader
		GLuint shader = glCreateShader(type);
		shaders.push_back(shader);
		const GLchar* str = src.c_str();
		glShaderSource(shader, 1, &str, NULL);
		glCompileShader(shader);

		// check compile result
		std::string info;
		if (!check_compile(shader, &info)){
			delete_shaders(shaders);
			glDeleteProgram(program);
			throw runtime_error("A error ocurred when compile shader");
		}
		// attach to program
		glAttachShader(program, shader);
	}
	// link
	glLinkProgram(program);
	// clean up
	delete_shaders(shaders);
	return program;
}

GLuint fw::LoadTexture(string filename)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, channels;

	// load image
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	return textureID;
}

void Mesh::Draw(GLuint program)
{
	// setup materials
	GLuint diffuse_count = 1;
	GLuint specular_count = 1;
	for (GLuint i = 0; i < textures_.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream ss;
		string num;
		string type = textures_[i].type_;
		if (type == "texture_diffuse")
			ss << diffuse_count++;
		else if (type == "texture_specular")
			ss << specular_count++;
		num = ss.str();
		string mattype = "material." + type + num;
		GLuint loc = glGetUniformLocation(program, mattype.c_str());
		glUniform1i(loc, i);
		glBindTexture(GL_TEXTURE_2D, textures_[i].id_);
	}
	glActiveTexture(GL_TEXTURE0);

	// draw
	glBindVertexArray(vao_);
	//glDrawArrays(GL_TRIANGLES, 0, indices_.size());
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::SetupMesh()
{
	// Vertex buffer object setup
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);

	// Vertex array object setup
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	// Elememt buffer object setup
	glGenBuffers(1, &ebo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), &indices_[0], GL_STATIC_DRAW);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal_));
	// texture coordination
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoords_));

	// detach
	glBindVertexArray(0);
}

void Model::Draw(GLuint program)
{
	for (auto& mesh : meshes_)
		mesh.Draw(program);
}

void Model::LoadModel(string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		throw runtime_error(string("Loading model error:") + importer.GetErrorString());
	}
	dir_ = path.substr(0, path.find_last_of('/'));
	ProcNode(scene->mRootNode, scene);
}

void Model::ProcNode(aiNode* node, const aiScene* scene)
{
	// process meshes
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes_.push_back(ProcMesh(mesh, scene));
	}

	//--- recursively walk on node ---
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		ProcNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	// process vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v;
		const auto& mv = mesh->mVertices[i];
		if (!mesh->mNormals)
			throw runtime_error("model missing normal");
		const auto& mn = mesh->mNormals[i];

		v.position_ = glm::vec3(mv.x, mv.y, mv.z);
		v.normal_ = glm::vec3(mn.x, mn.y, mn.z);
		if (mesh->HasTextureCoords(0)){
			const auto& mtex = mesh->mTextureCoords[0][i];
			v.texcoords_ = glm::vec2(mtex.x, mtex.y);
		}
		else{
			v.texcoords_ = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(v);
	}
	// process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = LoadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = LoadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// find in recoreds
		string texfile = str.C_Str();
		auto texiter = loaded_mesh_.find(texfile);
		if (texiter != loaded_mesh_.end()){
			// used loaded texture
			textures.push_back(texiter->second);
		}
		else{
			Texture texture;
			texture.id_ = LoadTexture(dir_ + '/' + str.C_Str());
			texture.type_ = typeName;
			texture.path_ = str;
			textures.push_back(texture);
			loaded_mesh_[texfile] = texture;
		}
	}
	return textures;
}

GLuint fw::LoadCubemap(array<string, 6> facefiles)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	for (unsigned int i = 0; i < facefiles.size(); i++){

		int width, height, channels;

		// load image
		unsigned char *data = stbi_load(facefiles[i].c_str(), &width, &height, &channels, 0);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return tex;
}

shared_ptr<Model> fw::LoadModel(string filename)
{
	shared_ptr<Model> model = std::make_shared<Model>(filename);
	return model;
}


const GLfloat* CubeVertices(GLuint* size){
	static const GLfloat cube[] = {
		// six faces vertices          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	*size = sizeof(cube);
	return cube;
}

GLuint fw::CreateCube()
{
	GLuint vbo, vao;
	GLuint size;
	const GLfloat* vertices = CubeVertices(&size);
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindVertexArray(vao);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	return vao;
}

