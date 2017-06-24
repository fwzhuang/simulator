#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GLSLShader.h"
#include <GL/freeglut.h>
#include "load_obj.h"

//singleton

struct VAO_Buffer
{
	GLuint vao;
	GLuint array_buffer;
	GLuint index_buffer;
	GLuint texture;
	GLuint index_size;
};

class Scene
{
public:
	static Scene* getInstance(int argc, char** argv);
	~Scene(); //closeFunc() 
	void add(Obj& object);   //add objects,bind VAOs 
	void render();

	//void add(Simulation& simulation);
	void RenderBuffer(VAO_Buffer vao_buffer);
	vector<VAO_Buffer> obj_vaos;
	

private:
	Scene(int argc, char** argv);  //initial
	inline void check_GL_error();
	void loadShader();
	

private:
	static Scene* pscene;       //pscene points to the Scene(singleton)
	//Simulation* p_simulation;
	GLSLShader renderShader;
	enum attributes { position, texture, normal };

private:
	static void DrawGrid();                  // OPENGL场景的各种函数
	static void RenderGPU_CUDA();
	static void onRender();
	static void OnReshape(int nw, int nh);
	static void OnIdle();
	static void OnMouseMove(int x, int y);
	static void OnMouseDown(int button, int s, int x, int y);
	static void OnKey(unsigned char key, int, int);
	static void OnShutdown();

private:

	static int oldX, oldY;    // OPENGL场景的各种参数declaration
	static float rX, rY;
	static int state;
	static float dist, dy;
	static GLint viewport[4];
	static GLfloat modelview[16];
	static GLfloat projection[16];
	static glm::vec3 Up, Right, viewDir;
	static int selected_index;
	static const int width = 1024, height = 1024;
};



