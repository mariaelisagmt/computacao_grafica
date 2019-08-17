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
using namespace scene;

#include "cubo.cpp"

GLuint camera_mode = 0;

//VARIAVEIS UTILIZADAS NO MAPEAMENTO DE DADOS DO PROGRAMA PARA OS SHADERS

GLuint program; //ID do programa, lembrando que um programa aqui eh a combinacao de vertex com fragment shader


int mainwindow=-1; //ID da janela de visualizacao criada pelo GLUT
GLuint width = 640, height = 480;

ShaderProxy *proxy = 0;
Camera *camera = 0;

GLfloat normais[cubo_countVertices*3];

std::vector< ShadedObject* > objects;

GLfloat fov = 45.0;

void initializeMatrix() {
	
	camera->setPerspective(fov, width/(float)height, 0.0001f, 1000.0f);
	objects[0]->data.translate(-1.5, 0, -10);
	objects[1]->data.translate( 1.5, 0, -10);
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

	getNormals(cubo_countVertices, cubo_indexList, cubo_normalIndexList, cubo_normalList, 
	cubo_countPrimitives,	cubo_primitiveSize, normais);

	const GLuint QTD_ATRIBUTOS = 2;

	proxy = new ShaderProxy(program, QTD_ATRIBUTOS);
	camera = new Camera(proxy);
		
	ShadedObject *cubo1 = new ShadedObject("cubo", proxy);
	ShadedObject *cubo2 = new ShadedObject("cubo", proxy);
	//Azul
	cubo1->data.setDiffuseReflection(0.0f, 0.0f, 1.0f, 1.0f);
	cubo1->data.setAmbientReflection(0.1f, 0.1f, 0.1f, 1.0f);
	cubo1->data.setSpecularReflection(1.0f, 1.0f, 1.0f, 1.0f, 100.0f);
	//vermelho
	cubo2->data.setDiffuseReflection(1.0f, 0.0f, 0.0f, 1.0f);
	cubo2->data.setAmbientReflection(0.1f, 0.1f, 0.1f, 1.0f);
	cubo2->data.setSpecularReflection(1.0f, 1.0f, 1.0f, 1.0f, 100.0f);

	objects.push_back(cubo1);
	objects.push_back(cubo2);
	
	initializeMatrix();
	
	try {
		proxy->useProgram();
		proxy->setAttribute("coord3d", cubo_vertexList, sizeof(cubo_vertexList));
		proxy->setAttribute("normal3d", normais, sizeof(normais));
		proxy->setElementPrimitive(cubo_indexList, sizeof(cubo_indexList));
		camera->update();
		objects[0]->update();
		objects[1]->update();

	} catch(string error) {
		cout<<error<<endl;
		return 0;
	}
	return 1;
}

/**
*
* Atualiza o desenho na tela de acordo com o modelo de cena atual.
*
*/
void atualizarDesenho()
{
	
	glClearColor(0.4, 0.4, 0.4, 0.4);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	try {
		camera->update();
		objects[0]->update();
		objects[0]->drawElements();
		objects[1]->update();
		objects[1]->drawElements();	
	} 
	
	catch(string error) {
		cout<<error<<endl;
	}
	glutSwapBuffers();
}

/**
*
* Desaloca o programa gerado.
*
*/
void finalizar()
{
  glDeleteProgram(program);
  if (!proxy) {
  	delete proxy;
  	proxy = 0;
  }
}
/**
*
* Garante que a janela da aplicacao sera fechada.
*
*/
void fecharJanela() {
	glutDestroyWindow(mainwindow);
	finalizar();
	exit(0);
}

void timer(int value){

	if(camera_mode != 1){
		camera->setViewIdentity();
		camera->translate(-1.5,0,0);
		camera_mode = 1;
	}
	else{
		camera->setViewIdentity();
		camera->translate(1.5,0,0);
		camera_mode = 2;
	}
	glutTimerFunc(2000, timer, 1);
	glutPostRedisplay();
}

/**
*
* Ponto de entrada do programa.
*
*/
int main(int argc, char* argv[])
{
	/* Funções necessárias para iniciar a GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(width, height);
	mainwindow = glutCreateWindow("API Scene");

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
		glutTimerFunc(2000, timer, 1);

		glutMainLoop();
	}

	/* Se o ocasionalmente programa sair ,
	liberamos os recursos da memória e completaremos ele com sucesso*/
	return EXIT_SUCCESS;
}
