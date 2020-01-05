#ifndef MODEL_H
#define MODEL_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtGui/QOpenGLFunctions_4_3_Core>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLShaderProgram>

#include <math.h>
#include "point3d.h"

class Model
{
public:
	Model() {}
	Model(const QString &filePath, int s, Point3d p);

	void render(GLfloat P[][4], GLfloat MV[][4], bool wireframe = false, bool normals = false);
	QString fileName() const { return m_fileName; }
	int faces() const { return m_pointIndices.size() / 3; }
	int edges() const { return m_edgeIndices.size() / 2; }
	int points() const { return m_points.size(); }
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath);

	void updateRotation(Point3d, Point3d);
private:
	Point3d boundsMin;
	Point3d boundsMax;
	int scale;
	QString m_fileName;

	Point3d position;
	Point3d rotation;

	QVector<Point3d> m_points;
	QVector<Point3d> m_target_normals;
	QVector<unsigned int> m_edgeIndices;
	QVector<unsigned int> m_pointIndices;


	
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	QOpenGLShaderProgram* shaderProgram;

	static QOpenGLShader* vertexShader;
	static QOpenGLShader* geometryShader;
	static QOpenGLShader* fragmentShader;
	static bool firstInitShader;
};

#endif