#ifndef StarRing_H
#define StarRing_H

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
#include "Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <qopenglfunctions_4_3_core.h>
using namespace std;


// enum ShaderType { NORMAL, REFLECTION, REFRACTION, TRAIN, PLANET, CHASER };

class StarRing
{
public:
	static float DEGREE_TO_RADIANT;
	static float RADIANT_TO_DEGREE;
	static GLuint skyboxShaderID;

	StarRing() {}
	StarRing(const QString &filePath, int s, Point3d p, ShaderType type = NORMAL);
	StarRing(StarRing*);

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
	void updateRotation(Point3d f_position, Point3d f_tangent, Point3d f_normal);
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
	QVector3D centerPoistion;
private:

	ShaderType shadertype;
	Point3d boundsMin;
	Point3d boundsMax;
	int scale;
	QString m_fileName;
	glm::mat4 *modelMatrices;
	unsigned int amount;

	QVector3D eyePosition;
	Point3d position;
	Point3d rotation;
	QOpenGLTexture *texture;

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
};

#endif