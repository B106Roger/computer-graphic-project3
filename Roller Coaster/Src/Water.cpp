#include "Water.h"
#include <time.h>
#include <iostream>
using namespace std;
Water::Water()
{
	size = 100;
	numberOfSquare = 200;
}

void Water::Paint(GLfloat P[][4], GLfloat MV[][4])
{
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
	shaderProgram->setUniformValue("cameraPos", eyepos);


	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Enable Attribute 0
	shaderProgram->enableAttributeArray(0);
	// Set Attribute 0 to be position
	shaderProgram->setAttributeArray(0,GL_FLOAT,0,2,NULL);
	// 0: location; GL_FLOAT: 型別; 0: 從哪裡開始; 2: input vector維度; NULL: ?
	//unbind buffer
	vvbo.release();

	//Draw a triangle with 3 indices starting from the 0th index
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
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

	int width, height;
	void *pixels = read_tga("./Textures/water-texture-2.tga", &width, &height);

	if (!pixels)
		return ;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
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







PillarDrawer::PillarDrawer():
	height(50.f)
{
}

void PillarDrawer::Paint(GLfloat P[][4], GLfloat MV[][4], vector<Pnt3f> &inputLoc)
{
	//Bind the shader we want to draw with
	shaderProgram->bind();
	//Bind the VAO we want to draw
	vao.bind();
	clock_t utime = clock();
	GLfloat t = utime / 500.f;
	//pass projection matrix to shader
	shaderProgram->setUniformValue("proj_matrix", P);
	//pass modelview matrix to shader
	shaderProgram->setUniformValue("model_matrix", MV);


	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Enable Attribute 0
	shaderProgram->enableAttributeArray(0);
	// Set Attribute 0 to be position
	shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);
	// 0: location; GL_FLOAT: 型別; 0: 從哪裡開始; 2: input vector維度; NULL: ?
	//unbind buffer
	vvbo.release();

	//Draw a triangle with 3 indices starting from the 0th index
	for (auto &p : inputLoc)
	{
		shaderProgram->setUniformValue("inputLoc", QVector3D(p.x, p.y, p.z));
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}
	
	//Disable Attribute 0&1
	shaderProgram->disableAttributeArray(0);

	//unbind vao
	vao.release();
	//unbind vao
	shaderProgram->release();

}
void PillarDrawer::Init()
{
	InitShader("./Shader/Pillar.vs", "./Shader/Model.fs", "./Shader/Model.gs");
	InitVAO();
	InitVBO();
}
void PillarDrawer::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}

void PillarDrawer::InitVBO()
{
	//Set each vertex's position
	float DegToRadian = 3.1415026 / 180.f;
	for (int i = 0; i < 360; i+=10)
	{
		float x1 = sin(i * DegToRadian);
		float z1 = cos(i * DegToRadian);

		float x2 = sin((i + 10) * DegToRadian);
		float z2 = cos((i + 10) * DegToRadian);
		vertices
			<< QVector3D(x1, 0, z1)
			<< QVector3D(x2, 0, z2)
			<< QVector3D(x2, 1, z2)

			<< QVector3D(x2, 1, z2)
			<< QVector3D(x1, 1, z1)
			<< QVector3D(x1, 0, z1);
	}

	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(vertices.constData(), vertices.size() * sizeof(QVector3D));
}
void PillarDrawer::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
{

	// Create Shader
	shaderProgram = new QOpenGLShaderProgram();
	QFileInfo  vertexShaderFile(vertexShaderPath);
	if (vertexShaderFile.exists())
	{
		vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
		if (vertexShader->compileSourceFile(vertexShaderPath))
			shaderProgram->addShader(vertexShader);
		else
			qWarning() << "Vertex Shader Error " << vertexShader->log();
	}
	else
		qDebug() << vertexShaderFile.filePath() << " can't be found";

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
	if (fragmentShaderFile.exists())
	{
		fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
		if (fragmentShader->compileSourceFile(fragmentShaderPath))
			shaderProgram->addShader(fragmentShader);
		else
			qWarning() << "fragment Shader Error " << fragmentShader->log();
	}
	else
		qDebug() << fragmentShaderFile.filePath() << " can't be found";
	shaderProgram->link();
}