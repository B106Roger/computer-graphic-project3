#ifndef MOUNTAIN_H
#define MOUNTAIN_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui>  
#include <QtOpenGL>  
#include <QtGui/QOpenGLFunctions_4_3_Core>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLShaderProgram>

#include <math.h>
#include "point3d.h"
#include <vector>
using namespace std;


class Mountain
{
public:
	static GLuint skyboxShaderID;
	struct Point
	{
		float x;
		float y;
		Point(int f_x, int f_y)
		{
			x = f_x;
			y = f_y;
		}
	};

	Mountain() {}
	Mountain(int width, int length, Point3d center, const QString &heightMapFile, const QString &textureFile);
	Mountain(Mountain*);

	void render(GLfloat P[][4], GLfloat MV[][4], bool wireframe = false, bool normals = false);
	QString fileName() const { return m_fileName; }
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath);
private:
	float side;
	QString m_fileName;
	GLint img_height;
	GLint img_width;
	QOpenGLTexture *texture;

	QVector3D eyePosition;
	Point3d centerPosition;

	QVector<Point3d> vertices;
	QVector<Point3d> m_target_normals;
	vector<vector<float>> heightMap;

	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	QOpenGLShaderProgram* shaderProgram;

	QOpenGLShader* vertexShader;
	QOpenGLShader* geometryShader;
	QOpenGLShader* fragmentShader;
};

#endif