#include "Tire.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVarLengthArray>

#include <QtOpenGL/QtOpenGL>
#include <QtGui/qmatrix.h>
#define DEBUG_FLAG

GLuint Tire::skyboxShaderID = 0;
float Tire::DEGREE_TO_RADIANT = 3.1415926f / 180.f;
float Tire::RADIANT_TO_DEGREE = 180.f / 3.1415926f;

Tire::Tire()
{
	Init();
	angle = 0;
}
Tire::Tire(Tire* copyVersion)
{
	m_points = copyVersion->m_points;
	m_edgeIndices = copyVersion->m_edgeIndices;
	m_pointIndices = copyVersion->m_pointIndices;
	shaderProgram = copyVersion->shaderProgram;
	InitVAO();
	InitVBO();
}

void Tire::render(GLfloat P[][4], GLfloat MV[][4])
{
	glEnable(GL_DEPTH_TEST);
	/*glEnableClientState(GL_VERTEX_ARRAY);*/
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glShadeModel(GL_SMOOTH);

		Point3d bounds = boundsMax - boundsMin;
		Point3d boundedContant = (boundsMin + bounds * 0.5);
		GLfloat boundScale = this->scale / qMax(bounds.x, qMax(bounds.y, bounds.z));
		QVector3D inputPos = { position.x, position.y, position.z };
		QVector3D inputRot = { rotation.x, rotation.y, rotation.z };
		QVector3D inputConst = { boundedContant.x,boundedContant.y,boundedContant.z };

		shaderProgram->bind();
		//Bind the VAO we want to draw
		vao.bind();
		//pass projection and modelview matrix to shader
		shaderProgram->setUniformValue("proj_matrix", P);
		shaderProgram->setUniformValue("model_matrix", MV);
		shaderProgram->setUniformValue("scale", boundScale);
		shaderProgram->setUniformValue("inputPos", inputPos);
		shaderProgram->setUniformValue("inputRot", inputRot);
		shaderProgram->setUniformValue("inputConst", inputConst);
		shaderProgram->setUniformValue("myMatrix", rotMatrix);

		shaderProgram->setUniformValue("isWheel", 0);
		shaderProgram->setUniformValue("angle", angle);

		vvbo.bind();
		shaderProgram->enableAttributeArray(0);
		shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);

		//glBindTexture(GL_TEXTURE_CUBE_MAP, Tire::skyboxShaderID);
		vvbo.release();
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_points.size() / 2);
		glDrawArrays(GL_TRIANGLE_FAN, m_points.size() / 2, m_points.size() / 2);
		
		//glDrawElements(GL_TRIANGLES, m_pointIndices.size(), GL_UNSIGNED_INT, m_pointIndices.data());
		shaderProgram->disableAttributeArray(0);
		vao.release();

		shaderProgram->release();

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}
	/*glDisableClientState(GL_VERTEX_ARRAY);*/
	glDisable(GL_DEPTH_TEST);
}

void Tire::updateRotation(Point3d f_rotationDir, Point3d f_position)
{
	rotation = f_rotationDir;
	position = f_position;
}

void Tire::updateRotation(Point3d f_position, Point3d f_tangent, Point3d f_normal)
{
	f_tangent = f_tangent.normalize();
	f_normal = f_normal.normalize();
	Point3d binomal = cross(f_tangent, f_normal).normalize();
	rotMatrix[0][0] = -f_tangent.x;
	rotMatrix[0][1] = f_normal.x;
	rotMatrix[0][2] = -binomal.x;

	rotMatrix[1][0] = -f_tangent.y;
	rotMatrix[1][1] = f_normal.y;
	rotMatrix[1][2] = -binomal.y;

	rotMatrix[2][0] = -f_tangent.z;
	rotMatrix[2][1] = f_normal.z;
	rotMatrix[2][2] = -binomal.z;

	position = Point3d(f_position.x, f_position.y + 3, f_position.z);
}

void Tire::Init()
{
	InitShader("./Shader/TireModel.vs", "./Shader/TireModel.fs", "./Shader/TireModel.gs");
	InitVAO();
	InitVBO();
}
void Tire::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
void Tire::InitVBO()
{
	const float degToRedian = 1.f / 180 * 3.1415926;
	radius = 5.f;
	m_points << Point3d(0, 0, -3);
	for (int i = 0; i < 360; i++)
	{
		m_points << Point3d(radius * sin(i*degToRedian), radius * cos(i*degToRedian), -3);
	}
	m_points << m_points[1];
	m_points << Point3d(0, 0, 3);
	int targetsize = m_points.size();
	for (int i = 0; i < 360; i++)
	{
		m_points << Point3d(radius * sin(i*degToRedian), radius * cos(i*degToRedian), 3);
	}
	m_points << m_points[targetsize];

	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(m_points.constData(), m_points.size() * sizeof(Point3d));
}
void Tire::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
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