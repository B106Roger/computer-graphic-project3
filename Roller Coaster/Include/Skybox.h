#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtGui/QOpenGLFunctions_4_3_Core>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLShaderProgram>

#include <math.h>
#include "point3d.h"
#include "TGATexture.h"

class Skybox
{
public:
	Skybox() { }
	
	void paintSkybox(GLfloat P[][4], GLfloat MV[][4]);

	void Init();
	void InitVAO();
	void InitVBO();
	void InitTexture();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath);
	unsigned int skyboxTextureID;

private:
	TextureImage skybox[6];
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;

	QOpenGLShaderProgram* shaderProgram;

	QOpenGLShader* vertexShader;
	QOpenGLShader* fragmentShader;
};

#endif