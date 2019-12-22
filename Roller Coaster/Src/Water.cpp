#include "Water.h"
#include <iostream>
using namespace std;
Water::Water()
{
	size = 200;
	numberOfSquare = 400;
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
	//pass time to shader
	shaderProgram->setUniformValue("time", t);

	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Enable Attribute 0
	shaderProgram->enableAttributeArray(0);
	// Set Attribute 0 to be position
	shaderProgram->setAttributeArray(0,GL_FLOAT,0,2,NULL);
	// 0: location; GL_FLOAT: 型別; 0: 從哪裡開始; 2: input vector維度; NULL: ?
	//unbind buffer
	vvbo.release();


	//cvbo.bind();
	////// Allocate and initialize the information
	//cvbo.allocate(colors.constData(), colors.size() * sizeof(QVector3D));
	//// Enable Attribute 1
	//shaderProgram->enableAttributeArray(1);
	//// Set Attribute 0 to be color
	//shaderProgram->setAttributeArray(1,GL_FLOAT,0,3,NULL);
	////unbind buffer
	//cvbo.release();


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
	InitShader("./Shader/Water.vs","./Shader/Water.fs", "./Shader/Water.gs");
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
	float side = size / numberOfSquare;
	float initial_X = -size / 2, initial_Y = size / 2;
	float time = clock() / 1000.f;
	for (float y = size / 2; y > -size / 2; y -= side)
	{
		float wave = 2.f;
		float a = 5.f;
		for (float x = -size / 2; x < size / 2; x += side)
		{
			vertices
				<< QVector2D(x + side, y - side)
				<< QVector2D(x + side, y)
				<< QVector2D(x, y)

				<< QVector2D(x, y)
				<< QVector2D(x, y - side)
				<< QVector2D(x + side, y - side);
		}
	}

	
	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(vertices.constData(),vertices.size() * sizeof(QVector2D));

	//Set each vertex's color
	/*colors<<QVector3D(0.0f,1.0f,0.0f)
		  <<QVector3D(0.0f,1.0f,0.0f)
		  <<QVector3D(0.0f,1.0f,0.0f);*/
	// Create Buffer for color
	//cvbo.create();
	//// Bind the buffer so that it is the current active buffer.
	//cvbo.bind();
	//// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	//cvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	//// Allocate and initialize the information
	//cvbo.allocate(colors.constData(),colors.size() * sizeof(QVector3D));

}
void Water::InitShader(QString vertexShaderPath,QString fragmentShaderPath,QString geomoetryShaderPath)
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

	QFileInfo  geometryShaderFile(geomoetryShaderPath);
	if (geometryShaderFile.exists())
	{
		geometryShader = new QOpenGLShader(QOpenGLShader::Geometry);
		if (geometryShader->compileSourceFile(geomoetryShaderPath))
			shaderProgram->addShader(geometryShader);
		else
			qWarning() << "Geometry Shader Error " << geometryShader->log();
	}
	else
		qDebug() << geometryShaderFile.filePath() << " can't be found";

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