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

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/glew.h"
#include <GL/freeglut.h>
#endif

using namespace std;
using namespace matrixmath;
using namespace shaderutils;

#include "monkey.cpp"

GLfloat fov = 45.0;

GLfloat eye_position[3];
GLfloat eye_orientation[2];

GLfloat scale[16];
GLfloat translation[16];

GLfloat model[16];
GLfloat view[16];
GLfloat projection[16];
GLfloat matrix[16];

int mainwindow=-1;
GLuint width = 640, height = 480;
GLfloat aspect_ratio = (GLfloat)width/height;


ShaderProxy *proxy = 0;
GLuint program;

void initializeMatrix() {
	//ortho(projection, -100, 100, -100, 100, 0.0001, 1000.0);

	eye_position[0] = 0.0f;
	eye_position[1] = 0.0f;
	eye_position[2] = 0.0f;

	eye_orientation[0] = 0.0f;
	eye_orientation[1] = 0.0f;

	identity(model);
	identity(scale);
	identity(translation);

	GLfloat aspect = (GLfloat)width/height;
	perspective(projection, fov, aspect, 0.01f, 10000.0f);

	//ortho(projection, -(width/2), width/2, -(height/2), height/2, 0.01, 1000.0f);

	scaleMatrix4(scale,  20, 20, 20);
	translationMatrix4(translation, 0, 0, -100);
	multMatrix4(model, translation, model);
	multMatrix4(model, scale, model);
}

void updateMatrix() {
	identity(matrix);
	identity(view);

	GLfloat rotViewX[16];
	GLfloat rotViewY[16];

	identity(rotViewX);
	identity(rotViewY);

	rotationXMatrix4(rotViewX, eye_orientation[0]);
	rotationYMatrix4(rotViewY, eye_orientation[1]);

	//EXERCICIO IMPLEMENTAR MOVIMENTO DA CÂMERA (2 PONTOS NA SEGUNDA AP)

	multMatrix4(view, rotViewY, view);
	multMatrix4(view, rotViewX, view);

	transposeMatrix4(view, view);

	multMatrix4(matrix, projection, matrix);
	multMatrix4(matrix, view, matrix);
	multMatrix4(matrix, model, matrix);
}


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
	
	try {
		proxy->useProgram();
		proxy->setAttribute("coord3d", monkey_vertexList, sizeof(monkey_vertexList));
		proxy->setAttribute("color3d", cores, sizeof(cores));
		proxy->setElementPrimitive(monkey_indexList, sizeof(monkey_indexList));
		proxy->setUniformMatrix4fv("matrix", matrix, 1, GL_TRUE);
	} catch(string error) {
		cout<<error<<endl;
		return 0;
	}

	initializeMatrix();
	updateMatrix();
	
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
  if (!proxy) {
  	delete proxy;
  	proxy = 0;
  }
}

void fecharJanela() {
	glutDestroyWindow(mainwindow);
	finalizar();
	exit(0);
}


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
	glutPostRedisplay();
}

/* 
void receberEventoDoMouse(int button, int state, int x, int y){
	
	GLfloat nx = 2*(float)x/(width-1) - 1;
	GLfloat ny = 1 - (2*(float)y)/(height - 1);

	cout << "X: " << nx << " Y: " << ny << endl;
	
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		eye_orientation[1] -= nx*20 ;
		eye_orientation[0] += ny*20;
		updateMatrix();
		atualizarDesenho();
	}

//	updateMatrix();
//	glutPostRedisplay();
}
*/

void movendoMouse(int x, int y){

	GLfloat nx = 2*(float)x/(width-1) - 1;
	GLfloat ny = 1 - (2*(float)y)/(height - 1);

	cout << "X: " << nx << " Y: " << ny << endl;
	
	eye_orientation[1] += nx;
	eye_orientation[0] -= ny;

	updateMatrix();
	atualizarDesenho();
}

void atualizacaoJanela(int w, int h){
	width = w;
	height = h;
	aspect_ratio = (GLfloat)width/height;
	identity(projection);
	perspective(projection, 90, aspect_ratio, 0.01f, 1000.0f);
	updateMatrix();
	glViewport(0, 0, w, h);
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	/* Funções necessárias para iniciar a GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(width, height);
	mainwindow = glutCreateWindow("Meu Primeiro Cubo Minha Vida");

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
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
//		glutMouseFunc(receberEventoDoMouse);
		glutMotionFunc(movendoMouse);
		glutReshapeFunc(atualizacaoJanela);
		glutMainLoop();
	}

	/* Se o ocasionalmente programa sair ,
	liberamos os recursos da memória e completaremos ele com sucesso*/
	return EXIT_SUCCESS;
}
