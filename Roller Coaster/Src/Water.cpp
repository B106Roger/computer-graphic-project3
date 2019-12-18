#include "Water.h"
#include <iostream>
using namespace std;
Water::Water()
{
	size = 200;
	numberOfSquare = 200;
}
void Water::DimensionTransformation(GLfloat source[],GLfloat target[][4])
{
	//for uniform value, transfer 1 dimension to 2 dimension
	int i = 0;
	for(int j=0;j<4;j++)
		for(int k=0;k<4;k++)
		{
			target[j][k] = source[i];
			i++;
		}
}
void Water::Paint(GLfloat* ProjectionMatrix, GLfloat* ModelViewMatrix)
{
	//glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_LINE);
	GLfloat P[4][4];
	GLfloat MV[4][4];
	DimensionTransformation(ProjectionMatrix,P);
	DimensionTransformation(ModelViewMatrix,MV);

	//Bind the shader we want to draw with
	shaderProgram->bind();
	//Bind the VAO we want to draw
	vao.bind();
	clock_t utime = clock();
	GLfloat t = utime / 1000.f;
	//pass projection matrix to shader
	shaderProgram->setUniformValue("proj_matrix",P);
	//pass modelview matrix to shader
	shaderProgram->setUniformValue("model_matrix",MV);
	shaderProgram->setUniformValue("time", t);

	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Enable Attribute 0
	shaderProgram->enableAttributeArray(0);
	// Set Attribute 0 to be position
	shaderProgram->setAttributeArray(0,GL_FLOAT,0,3,NULL);
	//unbind buffer
	vvbo.release();


	cvbo.bind();
	//// Allocate and initialize the information
	cvbo.allocate(colors.constData(), colors.size() * sizeof(QVector3D));
	// Enable Attribute 1
	shaderProgram->enableAttributeArray(1);
	// Set Attribute 0 to be color
	shaderProgram->setAttributeArray(1,GL_FLOAT,0,3,NULL);
	//unbind buffer
	cvbo.release();


	//Draw a triangle with 3 indices starting from the 0th index
	glDrawArrays(GL_TRIANGLES,0,vertices.size());
	//Disable Attribute 0&1
	shaderProgram->disableAttributeArray(0);
	shaderProgram->disableAttributeArray(1);

	//unbind vao
	vao.release();
	//unbind vao
	shaderProgram->release();
	
}
void Water::Init()
{
	InitShader("./Shader/Water.vs","./Shader/Water.fs");
	InitVAO();
	InitVBO();
}
void Water::InitVAO()
{
	 // Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
#define DISTANCE(x,y) pow(pow((x),2.f)+pow((y),2.f),0.5f)
void Water::InitVBO()
{
	//Set each vertex's position
	int side = size / numberOfSquare;
	int initial_X = -size / 2, initial_Y = size / 2;
	float time = clock() / 1000.f;


		for (int y = size / 2; y > -size / 2; y -= side)
		{
			int i = 0;
			float wave = 5.f;
			float a = 5.f;
			for (int x = -size / 2; x < size / 2; x += side)
			{
				//int height = sin(DISTANCE(x + side / 2, y - side /2)) * 2 + 2;
				vertices
					<< QVector3D(x + side, sin(DISTANCE(x + side, y - side) / a) * wave + wave, y - side)
					<< QVector3D(x + side, sin(DISTANCE(x + side, y) / a) * wave + wave, y)
					<< QVector3D(x, sin(DISTANCE(x, y) / a) * wave + wave, y)

					<< QVector3D(x, sin(DISTANCE(x, y) / a) * wave + wave, y)
					<< QVector3D(x, sin(DISTANCE(x, y - side) / a) * wave + wave, y - side)
					<< QVector3D(x + side, sin(DISTANCE(x + side, y - side) / a) * wave + wave, y - side);

				QVector3D normal1 = QVector3D::crossProduct(
					QVector3D(x + side, sin(DISTANCE(x + side, y) / a) * wave + wave, y) - QVector3D(x + side, sin(DISTANCE(x + side, y - side) / a) * wave + wave, y - side),
					QVector3D(x + side, sin(DISTANCE(x + side, y) / a) * wave + wave, y) - QVector3D(x, sin(DISTANCE(x, y) / a) * wave + wave, y)
				);
				QVector3D normal2 = QVector3D::crossProduct(
					QVector3D(x, sin(DISTANCE(x, y - side) / a) * wave + wave, y - side) - QVector3D(x, sin(DISTANCE(x, y) / a) * wave + wave, y),
					QVector3D(x, sin(DISTANCE(x, y - side) / a) * wave + wave, y - side) - QVector3D(x + side, sin(DISTANCE(x + side, y - side) / a) * wave + wave, y - side)
				);
				normal1.normalize();
				normal2.normalize();
				colors
					<< normal1
					<< normal1
					<< normal1
					<< normal2
					<< normal2
					<< normal2;
			}
		}

	
	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(vertices.constData(),vertices.size() * sizeof(QVector3D));

	//Set each vertex's color
	/*colors<<QVector3D(0.0f,1.0f,0.0f)
		  <<QVector3D(0.0f,1.0f,0.0f)
		  <<QVector3D(0.0f,1.0f,0.0f);*/
	// Create Buffer for color
	cvbo.create();
	// Bind the buffer so that it is the current active buffer.
	cvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	cvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	cvbo.allocate(colors.constData(),colors.size() * sizeof(QVector3D));

}
void Water::InitShader(QString vertexShaderPath,QString fragmentShaderPath)
{
	// Create Shader
	shaderProgram = new QOpenGLShaderProgram();
	QFileInfo  vertexShaderFile(vertexShaderPath);
	if(vertexShaderFile.exists())
	{
		vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
 		if(vertexShader->compileSourceFile(vertexShaderPath))
 			shaderProgram->addShader(vertexShader);
 		else
 			qWarning() << "Vertex Shader Error " << vertexShader->log();
	}
	else
		qDebug()<<vertexShaderFile.filePath()<<" can't be found";

	QFileInfo  fragmentShaderFile(fragmentShaderPath);
	if(fragmentShaderFile.exists())
	{
		fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
		if(fragmentShader->compileSourceFile(fragmentShaderPath))
			shaderProgram->addShader(fragmentShader);
		else
			qWarning() << "fragment Shader Error " << fragmentShader->log();
	}
	else
		qDebug()<<fragmentShaderFile.filePath()<<" can't be found";
	shaderProgram->link();
}