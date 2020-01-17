#include <QtGui/QOpenGLFunctions_4_3_Core>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLShaderProgram>
#include <QVector>
#include <QVector3D>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include "TGATexture.h"
#include "Utilities/Pnt3f.H"


class Water 
{
public:
	QOpenGLShaderProgram* shaderProgram;
	QOpenGLShader* vertexShader;
	QOpenGLShader* geometryShader;
	QOpenGLShader* fragmentShader;
	QVector<QVector2D> vertices;
	// QVector<QVector3D> colors;
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	QOpenGLBuffer cvbo;
	GLuint skyboxTexture;
	float size;
	int numberOfSquare;
	GLuint texture;
	QVector3D eyepos;

public:
	Water();
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath,QString fragmentShaderPath, QString geomoetryShaderPath);
	void Paint(GLfloat P[][4], GLfloat MV[][4]);
};

class PillarDrawer
{
public:
	QOpenGLShaderProgram* shaderProgram;
	QOpenGLShader* vertexShader;
	QOpenGLShader* geometryShader;
	QOpenGLShader* fragmentShader;
	QVector<QVector3D> vertices;
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	GLuint skyboxTexture;
	float height;
	QVector3D location;
	GLuint texture;
	QVector3D eyepos;

public:
	PillarDrawer();
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath);
	void Paint(GLfloat P[][4], GLfloat MV[][4], vector<Pnt3f> &inputLoc);
};