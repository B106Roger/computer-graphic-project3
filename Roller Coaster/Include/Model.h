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
#include <vector>
using namespace std;


enum ShaderType { NORMAL, REFLECTION, REFRACTION ,TRAIN };

class Model
{
public:
	static float DEGREE_TO_RADIANT;
	static float RADIANT_TO_DEGREE;
	static GLuint skyboxShaderID;

	Model() {}
	Model(const QString &filePath, int s, Point3d p, ShaderType type = NORMAL);
	Model(Model*);

	void render(GLfloat P[][4], GLfloat MV[][4], bool wireframe = false, bool normals = false);
	QString fileName() const { return m_fileName; }
	int faces() const { return m_pointIndices.size() / 3; }
	int edges() const { return m_edgeIndices.size() / 2; }
	int points() const { return m_points.size(); }
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath);

	void updateRotation(Point3d f_rotation, Point3d f_position);
	void updateRotation(Point3d f_position, Point3d f_tangent, Point3d f_normal );
	void setEyePosition(float x, float y, float z) { eyePosition[0] = x; eyePosition[1] = y; eyePosition[2] = z; };

	float getPosition(int i)
	{
		switch (i)
		{
		case 0:
			return position.x;
		case 1:
			return position.y;
		case 2:
			return position.z;
		}
	};
private:

	ShaderType shadertype;
	Point3d boundsMin;
	Point3d boundsMax;
	int scale;
	QString m_fileName;

	QVector3D eyePosition;
	Point3d position;
	Point3d rotation;

	GLfloat rotMatrix[3][3];
	QVector<Point3d> m_points;
	QVector<Point3d> m_target_normals;
	QVector<unsigned int> m_edgeIndices;
	QVector<unsigned int> m_pointIndices;

	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	QOpenGLShaderProgram* shaderProgram;

	QOpenGLShader* vertexShader;
	QOpenGLShader* geometryShader;
	QOpenGLShader* fragmentShader;

	static vector<vector<float>> Multiply(const vector<vector<float>> &m1, const vector<vector<float>> &m2)
	{
		vector<vector<float>> result(m1.size(), vector<float>(m2[0].size(), 0));
		for (int i = 0; i < result.size(); i++)
		{
			for (int j = 0; j < result[0].size(); j++)
			{
				for (int k = 0; k < m1[0].size(); k++)
				{
					result[i][j] += m1[i][k] * m2[k][j];
				}
			}
		}
		return result;
	}
};

#endif