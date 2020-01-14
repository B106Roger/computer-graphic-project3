#include "Mountain.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVarLengthArray>

#include <QtOpenGL/QtOpenGL>
#include <QtGui/qmatrix.h>
#include <iostream>
using namespace std;


GLuint Mountain::skyboxShaderID = 0;

Mountain::Mountain(int length, int width, Point3d center ,const QString &heightMapFile, const QString &textureFile)
{
	QImage image(heightMapFile);
	 
	vertices.clear();
	img_height = image.height() - 1;
	img_width = image.width() - 1;
	centerPosition.x = length / 2 - center.x;
	centerPosition.z = width / 2 - center.z;
	side = 0.5;

	for (float y = 0; y < length; y+=side)
	{
		for (float x = 0; x < width; x+=side)
		{
			vector<Point3d> tmp;
			Point3d v1(x + side, 0, y - side);
			Point3d v2(x + side, 0, y);
			Point3d v3(x, 0, y);
			Point3d v4(x, 0, y - side);
			tmp.push_back(v1);
			tmp.push_back(v2);
			tmp.push_back(v3);
			tmp.push_back(v4);
			for (Point3d & ref : tmp)
			{
				float x_ratio = ref.x / float(width) * img_width;
				float y_ratio = ref.z / float(length) * img_height;
				if (x_ratio < 0)
					x_ratio = 0;
				if (y_ratio < 0)
					y_ratio = 0;
				if (x_ratio >= img_width - 1)
					x_ratio = img_width - 2;
				if (y_ratio >= img_height - 1)
					y_ratio = img_height - 2;
				
				int min_x = int(x_ratio);
				int min_y = int(y_ratio);
				int max_x = int(x_ratio) + 1;
				int max_y = int(y_ratio) + 1;
				

				float topHeight = qGray(image.pixel(min_x, min_y)) * abs(x_ratio - max_x) + qGray(image.pixel(max_x, min_y)) * (x_ratio - min_x);
				float downHeight = qGray(image.pixel(min_x, max_y)) * abs(x_ratio - max_x) + qGray(image.pixel(max_x, max_y)) * (x_ratio - min_x);

				ref.y = topHeight * abs(y_ratio - max_y) + downHeight * abs(y_ratio - min_y);
				ref.y /= 10.f;
				
			}

			vertices
				<< tmp[0]
				<< tmp[1]
				<< tmp[2]

				<< tmp[0]
				<< tmp[2]
				<< tmp[3];
		}
	}


	this->texture = new QOpenGLTexture(QImage(textureFile));
}
Mountain::Mountain(Mountain* copyVersion)
{
	shaderProgram = copyVersion->shaderProgram;
	heightMap = copyVersion->heightMap;
	InitVAO();
	InitVBO();
}

void Mountain::render(GLfloat P[][4], GLfloat MV[][4], bool wireframe, bool normals)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	

	glEnable(GL_DEPTH_TEST);
	/*glEnableClientState(GL_VERTEX_ARRAY);*/
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	QVector3D pos(centerPosition.x, centerPosition.y, centerPosition.z);
	GLuint textureID = texture->textureId();
	shaderProgram->bind();
	//Bind the VAO we want to draw
	vao.bind();
	//pass projection and modelview matrix to shader
	shaderProgram->setUniformValue("proj_matrix", P);
	shaderProgram->setUniformValue("model_matrix", MV);
	shaderProgram->setUniformValue("centerPosition", pos);
	shaderProgram->setUniformValue("mountain_length", img_height);
	shaderProgram->setUniformValue("mountain_width", img_width);

	// shaderProgram->setUniformValue("mountain_texture", textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	vvbo.bind();
	shaderProgram->enableAttributeArray(0);
	shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);
	if (false)
		glBindTexture(GL_TEXTURE_CUBE_MAP, Mountain::skyboxShaderID);
	vvbo.release();

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	shaderProgram->disableAttributeArray(0);
	vao.release();
	shaderProgram->release();

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);


	glDisable(GL_DEPTH_TEST);

	if (wireframe)
	{
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
}



void Mountain::Init()
{
	InitShader("./Shader/Mountain.vs", "./Shader/Mountain.fs", "./Shader/Mountain.gs");
	InitVAO();
	InitVBO();
}
void Mountain::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
void Mountain::InitVBO()
{
	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(vertices.constData(), vertices.size() * sizeof(Point3d));
}
void Mountain::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
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