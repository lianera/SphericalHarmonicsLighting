#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <string>
#include <tuple>  
#include <memory>
#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>

namespace fw{
	using glm::vec3;
	using glm::vec2;
	using std::string;
	using std::tuple;
	using std::vector;
	using std::map;
	using std::shared_ptr;
	using std::array;

	struct Vertex{
		vec3 position_;
		vec3 normal_;
		vec2 texcoords_;
	};

	struct Texture{
		GLuint id_;
		string type_;
		aiString path_;
	};

	class Mesh{
	public:
		Mesh(const vector<Vertex>& vertices,
			const vector<GLuint>& indices,
			const vector<Texture>& textures)
			:vertices_(vertices), indices_(indices), textures_(textures)
		{
			SetupMesh();
		}

		void Draw(GLuint program);

		vector<Vertex> vertices_;
		vector<GLuint> indices_;
		vector<Texture> textures_;
	private:
		GLuint vbo_, vao_, ebo_;

		void SetupMesh();
	};

	class Model{
		static map<string, Texture> loaded_mesh_;
	public:
		Model(string path)
		{
			LoadModel(path);
		}
		void Draw(GLuint program);
	private:
		vector<Mesh> meshes_;
		string dir_;

		void LoadModel(string path);
		void ProcNode(aiNode* node, const aiScene* scene);
		Mesh ProcMesh(aiMesh* mesh, const aiScene* scene);
		vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string type_name);
	};

	/** load texture from file
	*/
	GLuint LoadTexture(string filename);

	/** load cubemap texture
	* order +x -x +y -y +z -z
	*/
	GLuint LoadCubemap(array<string, 6> facefiles);

	/** load model from file
	*/
	shared_ptr<Model> LoadModel(string filename);

	/** load shaders & attach to a new program, note you have to delete program manually
	*/
	GLuint CreateProgram(vector<tuple<string, GLenum>> shader_src);

	GLuint CreateProgram(string vertex_src, string fragment_src);

	/*	primitives
	*/
	GLuint CreateCube();
};

#endif