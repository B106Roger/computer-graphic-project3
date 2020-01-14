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
#include <time.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVarLengthArray>

#include <QtOpenGL/QtOpenGL>

const int START_X = 80;//-4.0
const int START_Y = 0;//-2.5
const int START_Z = 0;
const int LENGTH_X = 1;//0.1
const int LENGTH_Y = 1;//0.1

const int HEIGHT_SCALE = 1.6;

const int WAVE_COUNT = 3;

const int STRIP_COUNT = 40;
const int STRIP_LENGTH = 10;
const int DATA_LENGTH = STRIP_LENGTH * 2 * (STRIP_COUNT - 1);




struct Name {
	GLuint vertex_buffer, normal_buffer;
	GLuint vertex_shader, fragment_shader, program;
	GLuint diffuse_texture, normal_texture;

	struct {
		GLint diffuse_texture, normal_texture;
	} uniforms;

	struct {
		GLint position;
		GLint normal;
	} attributes;
};

struct Value {
	GLfloat time;
	GLfloat wave_length[WAVE_COUNT],
		wave_height[WAVE_COUNT],
		wave_dir[WAVE_COUNT],
		wave_speed[WAVE_COUNT],
		wave_start[WAVE_COUNT * 2];
};

class GerstnerWater
{
public:
	QOpenGLShaderProgram* shaderProgram;
	QOpenGLShader* vertexShader;
	QOpenGLShader* geometryShader;
	QOpenGLShader* fragmentShader;
	// QVector<QVector2D> vertices;
	QOpenGLVertexArrayObject vao;
	QOpenGLBuffer vvbo;
	QOpenGLBuffer nvbo;
public:
	GerstnerWater();
	void Init();
	void InitVAO();
	void InitVBO();
	void InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath);
	void Paint(GLfloat P[][4], GLfloat MV[][4]);

private:
	GLfloat pt_strip[STRIP_COUNT*STRIP_LENGTH * 3] = { 0 };
	GLfloat pt_normal[STRIP_COUNT*STRIP_LENGTH * 3] = { 0 };
	GLfloat vertex_data[DATA_LENGTH * 3] = { 0 };
	GLfloat normal_data[DATA_LENGTH * 3] = { 0 };
	Value values;
	Name names;
	//wave_length, wave_height, wave_dir, wave_speed, wave_start.x, wave_start.y
	static const GLfloat wave_para[6][6];
	static const GLfloat gerstner_pt_a[22];
	static const GLfloat gerstner_pt_b[22];
	static const GLint gerstner_sort[6];

public:
	void initWave(void);
	GLuint initTexture(const char *filename);
	float gerstnerZ(float w_length, float w_height, float x_in, const GLfloat gerstner[22]);
	int normalizeF(float in[], float out[], int count);
	void calcuWave(void);
	void initGL(void);
};