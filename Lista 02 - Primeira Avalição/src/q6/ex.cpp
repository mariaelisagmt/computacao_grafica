/* Usando o padrão de saída fprintf */
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
/* Usando o GLUT com gerenciador de janelas */

#include "matrixmath.hpp"
#include "shaderutils.hpp"
#include "scene.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/glew.h"
#include <GL/freeglut.h>
#endif

using namespace std;
using namespace matrixmath;
using namespace shaderutils;


GLfloat fov = 45.0;

GLfloat eye_position[3];
GLfloat eye_orientation[2];

GLfloat cube_vertices[] = {
	 1, -1,   1, //0
	 1,  1,   1, //1
	-1,  1,   1, //2
	-1, -1,   1, //3
	-1, -1,  -1, //4
	-1,  1,  -1, //5
	 1,  1,  -1, //6
	 1, -1,  -1, //7

};

GLfloat cube_colors[] = {
	0, 0, 0,
	0, 0, 1,
	0, 1, 0,
	0, 1, 1,

	1, 0, 0,
	1, 0, 1,
	1, 1, 0,
	1, 1, 1
};

GLuint cube_indices[] = {
		0, 1, 2, 3, //Face Fontal
		4, 5, 6, 7, //Face Traseira
		1, 6, 5, 2, //Face de Cima
		0, 3, 4, 7, //Face de Baixo
		2, 5, 4, 3, //Face da Esquerda
		7, 6, 1, 0, //Face da Direita
};

GLfloat scale[16];
GLfloat translation[16];

GLfloat model[16];
GLfloat view[16];
GLfloat projection[16];

GLfloat matrix[16];

bool mousePress = false;

GLfloat mousePosition[2];

GLuint program;

ShaderProxy *proxy = 0;

int mainwindow=-1;
GLuint width = 640, height = 480;


void initializeMatrix() {
	//ortho(projection, -100, 100, -100, 100, 0.0001, 1000.0);

	eye_position[0] = 0.0f;
	eye_position[1] = 0.0f;
	eye_position[2] = 0.0f;

	eye_orientation[0] = 0.0f;
	eye_orientation[1] = 0.0f;

	identity(model);
	identity(translation);
	identity(scale);

	GLfloat aspect = (GLfloat)width/height;
	perspective(projection, fov, aspect, 0.01f, 1000.0f);
	//ortho(projection, -130, 130, -100, 100, 0.0001, 1000.0);

	scaleMatrix4(scale,  20, 20, 20);
	translationMatrix4(translation, 0, 0, -200);

	multMatrix4(model, translation, model);
	multMatrix4(model, scale, model);
}

void updateMatrix() {
	identity(matrix);
	identity(view);

	GLfloat rotViewX[16];
	GLfloat rotViewY[16];
	GLfloat translationView[16];

	identity(rotViewX);
	identity(rotViewY);
	identity(translationView);

	rotationXMatrix4(rotViewX, eye_orientation[0]);
	rotationYMatrix4(rotViewY, eye_orientation[1]);

	//EXERCICIO IMPLEMENTAR MOVIMENTO DA CÂMERA (2 PONTOS NA SEGUNDA AP)

	translationMatrix4(translationView, -eye_position[0], -eye_position[1], -eye_position[2]);

	multMatrix4(view, rotViewX, view);
	multMatrix4(view, rotViewY, view);

	multMatrix4(view, translationView, view);

	multMatrix4(matrix, projection, matrix);
	multMatrix4(matrix, view, matrix);
	multMatrix4(matrix, model, matrix);
}


/* COLOCAREMOS AS VARIAVEIS GLOBAIS AQUI MAIS TARDE */

int inicializar(void)
{
	try {
		program = genProgram("shader.vs", "shader.fs");
	} catch(string error) {
		cout<<error<<endl;

		return 0;
	}

	const GLuint QTD_ATRIBUTOS = 2;

	proxy = new ShaderProxy(program, QTD_ATRIBUTOS);
	initializeMatrix();
	updateMatrix();
	try {
		proxy->useProgram();

		proxy->setAttribute("coord3d", cube_vertices, sizeof(cube_vertices));
		proxy->setAttribute("color3d", cube_colors, sizeof(cube_colors));
		proxy->setElementPrimitive(cube_indices, sizeof(cube_indices));
		proxy->setUniformMatrix4fv("matrix", matrix, 1, GL_TRUE);
	} catch(string error) {
		cout<<error<<endl;
		return 0;
	}

	
	return 1;
}

void atualizarDesenho()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	proxy->useProgram();
	proxy->setUniformMatrix4fv("matrix", matrix, 1, GL_TRUE);
	proxy->drawElements();
	glutSwapBuffers();
}

void finalizar()
{
  glDeleteProgram(program);
}

void fecharJanela() {
	glutDestroyWindow(mainwindow);
	finalizar();
	exit(0);
}

void moveMouse(int x, int y){

	GLfloat deltaX = x - mousePosition[0];
	GLfloat deltaY = y - mousePosition[1];
	GLfloat mouse_Sensitivity = 0.25f;

	if(mousePress){
		eye_orientation[0] += mouse_Sensitivity * deltaY;
		eye_orientation[1] += mouse_Sensitivity * deltaX;
		mousePosition[0] = x;
		mousePosition[1] = y;
	}
	updateMatrix();
	atualizarDesenho();
}

void eventoDoMouse(int button, int state, int x, int y){

	if(state == GLUT_DOWN){
		mousePress = true;
		mousePosition[0] = x;
		mousePosition[1] = y;
		cout << x << ","<< y <<endl;
	}
	else{
		mousePress = false;
	}
}

void eventoDeTeclado(unsigned char key, int x, int y) {
	GLfloat dx = 0, dz = 0, speed = 0.12f;;
	
	if (key == 'w') {
		dz = 10;
	} else if (key == 's') {
		dz = -10;
	}

	if (key == 'a') {
		dx = -10;
	} else if (key == 'd') {
		dx = 10;
	}

	eye_position[0] += (-dz * view[8] + dx * view[0]) * speed;
	eye_position[1] += (-dz * view[9] + dx * view[1]) * speed;
	eye_position[2] += (-dz * view[10] + dx * view[2]) * speed;
	
	updateMatrix();
	atualizarDesenho();
}

/*
void eventoDeTeclado(unsigned char key, int x, int y) {
	if (key == 'i') {
		eye_orientation[0] += 10.0f;
	} else if (key == 'k') {
		eye_orientation[0] -= 10.0f;
	}

	if (key == 'j') {
		eye_orientation[1] += 10;
	} else if (key == 'l') {
		eye_orientation[1] -= 10;
	}

	updateMatrix();
	atualizarDesenho();
}

*/	

int main(int argc, char* argv[])
{
	/* Funções necessárias para iniciar a GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(width, height);
	mainwindow = glutCreateWindow("Meu Primeiro Cubo Minha Vida");

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	const char* ver = (const char*)glGetString(GL_VERSION);
	char major_number = ver[0];
	if (major_number < '2'){
		cout<<"Seu computador  nao suporta a versao 2.1 do OpenGL"<<endl;
		return 0;
	}

	#ifndef __APPLE__
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		fprintf(stderr, "Erro: %s\n", glewGetErrorString(glew_status));
		return EXIT_FAILURE;
	}
	#endif

	/* Quando as funções de inicialização são executadas sem erros,
	o programa pode iniciar os recursos */
	if (1 == inicializar())
	{
		/* Pode então mostrar se tudo correr bem */
		glutWMCloseFunc(fecharJanela);
		glutDisplayFunc(atualizarDesenho);
		glutKeyboardFunc(eventoDeTeclado);
		glutMouseFunc(eventoDoMouse);
		glutMotionFunc(moveMouse);
		glutMainLoop();
	}

	/* Se o ocasionalmente programa sair ,
	liberamos os recursos da memória e completaremos ele com sucesso*/
	return EXIT_SUCCESS;
}
