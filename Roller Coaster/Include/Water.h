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

	float size;
	int numberOfSquare;

public:
	Water();
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath,QString fragmentShaderPath, QString geomoetryShaderPath);
	void Paint(GLfloat* ProjectionMatrix, GLfloat* ModelViewMatrix);
	void DimensionTransformation(GLfloat source[],GLfloat target[][4]);
};