#include "GerstnerWater.h"
#include <iostream>
using namespace std;

const GLfloat GerstnerWater::wave_para[6][6] = {
	{ 1.6,	0.12,	0.9,	0.06,	0.0,	0.0 },
	{ 1.3,	0.1,	1.14,	0.09,	0.0,	0.0 },
	{ 0.2,	0.01,	0.8,	0.08,	0.0,	0.0 },
	{ 0.18,	0.008,	1.05,	0.1,	0.0,	0.0 },
	{ 0.23,	0.005,	1.15,	0.09,	0.0,	0.0 },
	{ 0.12,	0.003,	0.97,	0.14,	0.0,	0.0 }
};
const GLfloat GerstnerWater::gerstner_pt_a[22] = {
	0.0,0.0, 41.8,1.4, 77.5,5.2, 107.6,10.9,
	132.4,17.7, 152.3,25.0, 167.9,32.4, 179.8,39.2,
	188.6,44.8, 195.0,48.5, 200.0,50.0
};
const GLfloat GerstnerWater::gerstner_pt_b[22] = {
	0.0,0.0, 27.7,1.4, 52.9,5.2, 75.9,10.8,
	97.2,17.6, 116.8,25.0, 135.1,32.4, 152.4,39.2,
	168.8,44.8, 184.6,48.5, 200.0,50.0
};
const GLint GerstnerWater::gerstner_sort[6] = {
	0, 0, 1, 1, 1, 1
};


GerstnerWater::GerstnerWater()
{
}

void GerstnerWater::Paint(GLfloat P[][4], GLfloat MV[][4])
{
	calcuWave();
	shaderProgram->bind();
	vao.bind();

	clock_t utime = clock();
	GLfloat t = utime / 100.f;
	values.time = t;
	static QVector4D materAmbient(0.1, 0.1, 0.3, 1.0);
	static QVector4D materSpecular(0.8, 0.8, 0.9, 1.0);
	static QVector4D lightDiffuse(0.7, 0.7, 0.8, 1.0);
	static QVector4D lightAmbient(0.0, 0.0, 0.0, 1.0);
	static QVector4D lightSpecular(1.0, 1.0, 1.0, 1.0);
	static QVector4D envirAmbient(0.1, 0.1, 0.3, 1.0);

	shaderProgram->setUniformValue("materAmbient", materAmbient);
	shaderProgram->setUniformValue("materSpecular", materSpecular);
	shaderProgram->setUniformValue("lightDiffuse", lightDiffuse);
	shaderProgram->setUniformValue("lightAmbient", lightAmbient);
	shaderProgram->setUniformValue("lightSpecular", lightSpecular);
	shaderProgram->setUniformValue("envirAmbient", envirAmbient);
	shaderProgram->setUniformValue("time", values.time);
	shaderProgram->setUniformValue("perspProjMat", P);
	shaderProgram->setUniformValue("modelViewMat", MV);

	vvbo.bind();
	vvbo.allocate(vertex_data, DATA_LENGTH * 3);
	shaderProgram->enableAttributeArray(0);
	shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);

	vvbo.release();

	nvbo.bind();
	nvbo.allocate(normal_data, DATA_LENGTH * 3);
	shaderProgram->enableAttributeArray(1);
	shaderProgram->setAttributeArray(1, GL_FLOAT, 0, 3, NULL);
	nvbo.release();

	for (int c = 0; c < (STRIP_COUNT - 1); c++)
	{
		glBindTexture(GL_TEXTURE_2D, names.diffuse_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, STRIP_LENGTH * 2 * c, STRIP_LENGTH * 2);

	}

	shaderProgram->disableAttributeArray(0);
	shaderProgram->disableAttributeArray(1);

	vao.release();
	shaderProgram->release();
}
void GerstnerWater::Init()
{
	InitShader("./Shader/GerstnerWater.vs", "./Shader/GerstnerWater.fs", "./Shader/GerstnerWater.gs");
	InitVAO();
	InitVBO();
	initWave();
	initGL();
	
}
void GerstnerWater::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
#define DISTANCE(x,y) pow(pow((x),2.f)+pow((y),2.f),0.5f)
void GerstnerWater::InitVBO()
{
	//Set each vertex's position
	//float side = size / numberOfSquare;
	//float initial_X = -size / 2, initial_Y = size / 2;
	//float time = clock() / 1000.f;
	//for (float y = size / 2; y > -size / 2; y -= side)
	//{
	//	float wave = 2.f;
	//	float a = 5.f;
	//	for (float x = -size / 2; x < size / 2; x += side)
	//	{
	//		vertices
	//			<< QVector2D(x + side, y - side)
	//			<< QVector2D(x + side, y)
	//			<< QVector2D(x, y)

	//			<< QVector2D(x, y)
	//			<< QVector2D(x, y - side)
	//			<< QVector2D(x + side, y - side);
	//	}
	//}


	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	// Allocate and initialize the information
	// vvbo.allocate(vertices.constData(), vertices.size() * sizeof(QVector2D));

	// Create Buffer for position
	nvbo.create();
	// Bind the buffer so that it is the current active buffer.
	nvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	nvbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	// Allocate and initialize the information
	// nvbo.allocate(vertices.constData(), vertices.size() * sizeof(QVector2D));
}
void GerstnerWater::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
{
	vertexShaderPath = "./Shader/wave-gerstner-light-vs.glsl";
	fragmentShaderPath = "./Shader/wave-gerstner-light-fs.glsl";
	geomoetryShaderPath = "./Shader/wave-gerstner-light-gs.glsl";

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

	/*QFileInfo  geometryShaderFile(geomoetryShaderPath);
	if (geometryShaderFile.exists())
	{
		geometryShader = new QOpenGLShader(QOpenGLShader::Geometry);
		if (geometryShader->compileSourceFile(geomoetryShaderPath))
			shaderProgram->addShader(geometryShader);
		else
			qWarning() << "Geometry Shader Error " << geometryShader->log();
	}
	else
		qDebug() << geometryShaderFile.filePath() << " can't be found";*/

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
void GerstnerWater::initWave(void)
{
	//Initialize values{}
	values.time = 0.0;
	for (int w = 0; w < WAVE_COUNT; w++)
	{
		values.wave_length[w] = wave_para[w][0];
		values.wave_height[w] = wave_para[w][1];
		values.wave_dir[w] = wave_para[w][2];
		values.wave_speed[w] = wave_para[w][3];
		values.wave_start[w * 2] = wave_para[w][4];
		values.wave_start[w * 2 + 1] = wave_para[w][5];
	}

	//Initialize pt_strip[]
	int index = 0;
	for (int i = 0; i < STRIP_COUNT; i++)
	{
		for (int j = 0; j < STRIP_LENGTH; j++)
		{
			pt_strip[index] = START_X + i * LENGTH_X;
			pt_strip[index + 1] = START_Y + j * LENGTH_Y;
			if (pt_strip[index] == 0)
				int a = 0;
			index += 3;

		}
	}
}

GLuint GerstnerWater::initTexture(const char *filename)
{
	int width, height;
	void *pixels = read_tga(filename, &width, &height);
	GLuint texture;

	if (!pixels)
		return 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);

	free(pixels);
	return texture;

}


float GerstnerWater::gerstnerZ(float w_length, float w_height, float x_in, const GLfloat gerstner[22])
{
	x_in = x_in * 400.0 / w_length;

	while (x_in < 0.0)
		x_in += 400.0;
	while (x_in > 400.0)
		x_in -= 400.0;
	if (x_in > 200.0)
		x_in = 400.0 - x_in;

	int i = 0;
	float yScale = w_height / 50.0;
	while (i<18 && (x_in<gerstner[i] || x_in >= gerstner[i + 2]))
		i += 2;
	if (x_in == gerstner[i])
		return gerstner[i + 1] * yScale;
	if (x_in > gerstner[i])
		return ((gerstner[i + 3] - gerstner[i + 1]) * (x_in - gerstner[i]) / (gerstner[i + 2] - gerstner[i]) + gerstner[i + 3]) * yScale;
}

int GerstnerWater::normalizeF(float in[], float out[], int count)
{
	int t = 0;
	float l = 0.0;

	if (count <= 0.0) {
		//printf("normalizeF(): Number of dimensions should be larger than zero.\n");
		return 1;
	}
	while (t<count && in[t]<0.0000001 && in[t]>-0.0000001) {
		t++;
	}
	if (t == count) {
		//printf("normalizeF(): The input vector is too small.\n");
		return 1;
	}
	for (t = 0; t<count; t++)
		l += in[t] * in[t];
	if (l < 0.0000001) {
		l = 0.0;
		for (t = 0; t<count; t++)
			in[t] *= 10000.0;
		for (t = 0; t<count; t++)
			l += in[t] * in[t];
	}
	l = sqrt(l);
	for (t = 0; t<count; t++)
		out[t] /= l;

	return 0;
}


void GerstnerWater::calcuWave(void)
{
	//Calculate pt_strip[z], poly_normal[] and pt_normal[]
	int index = 0;
	float d, wave;
	for (int i = 0; i<STRIP_COUNT; i++)
	{
		for (int j = 0; j<STRIP_LENGTH; j++)
		{
			wave = 0.0;
			for (int w = 0; w<WAVE_COUNT; w++) {
				d = (pt_strip[index] - values.wave_start[w * 2] + (pt_strip[index + 1] - values.wave_start[w * 2 + 1]) * tan(values.wave_dir[w])) * cos(values.wave_dir[w]);
				if (gerstner_sort[w] == 1) {
					wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_a);
				}
				else {
					wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_b);
				}
			}
			pt_strip[index + 2] = START_Z + wave*HEIGHT_SCALE;
			index += 3;
		}
	}

	index = 0;
	for (int i = 0; i<STRIP_COUNT; i++)
	{
		for (int j = 0; j<STRIP_LENGTH; j++)
		{
			int p0 = index - STRIP_LENGTH * 3, p1 = index + 3, p2 = index + STRIP_LENGTH * 3, p3 = index - 3;
			float xa, ya, za, xb, yb, zb;
			if (i > 0) {
				if (j > 0) {
					xa = pt_strip[p0] - pt_strip[index], ya = pt_strip[p0 + 1] - pt_strip[index + 1], za = pt_strip[p0 + 2] - pt_strip[index + 2];
					xb = pt_strip[p3] - pt_strip[index], yb = pt_strip[p3 + 1] - pt_strip[index + 1], zb = pt_strip[p3 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
				}
				if (j < STRIP_LENGTH - 1) {
					xa = pt_strip[p1] - pt_strip[index], ya = pt_strip[p1 + 1] - pt_strip[index + 1], za = pt_strip[p1 + 2] - pt_strip[index + 2];
					xb = pt_strip[p0] - pt_strip[index], yb = pt_strip[p0 + 1] - pt_strip[index + 1], zb = pt_strip[p0 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
				}
			}
			if (i < STRIP_COUNT - 1) {
				if (j > 0) {
					xa = pt_strip[p3] - pt_strip[index], ya = pt_strip[p3 + 1] - pt_strip[index + 1], za = pt_strip[p3 + 2] - pt_strip[index + 2];
					xb = pt_strip[p2] - pt_strip[index], yb = pt_strip[p2 + 1] - pt_strip[index + 1], zb = pt_strip[p2 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
				}
				if (j < STRIP_LENGTH - 1) {
					xa = pt_strip[p2] - pt_strip[index], ya = pt_strip[p2 + 1] - pt_strip[index + 1], za = pt_strip[p2 + 2] - pt_strip[index + 2];
					xb = pt_strip[p1] - pt_strip[index], yb = pt_strip[p1 + 1] - pt_strip[index + 1], zb = pt_strip[p1 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
				}
			}
			if (normalizeF(&pt_normal[index], &pt_normal[index], 3))
				// printf("%d\t%d\n", index / 3 / STRIP_LENGTH, (index / 3) % STRIP_LENGTH);

			index += 3;
		}
	}

	//Calculate vertex_data[] according to pt_strip[], and normal_data[] according to pt_normal[]
	int pt;
	for (int c = 0; c<(STRIP_COUNT - 1); c++)
	{
		for (int l = 0; l<2 * STRIP_LENGTH; l++)
		{
			if (l % 2 == 1) {
				pt = c*STRIP_LENGTH + l / 2;
			}
			else {
				pt = c*STRIP_LENGTH + l / 2 + STRIP_LENGTH;
			}
			index = STRIP_LENGTH * 2 * c + l;
			for (int i = 0; i<3; i++) {
				vertex_data[index * 3 + i] = pt_strip[pt * 3 + i];
				normal_data[index * 3 + i] = pt_normal[pt * 3 + i];
			}
		}
	}
}

void GerstnerWater::initGL()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);

	names.diffuse_texture = initTexture("./Textures/water-texture-2.tga");
	glBindTexture(GL_TEXTURE0, names.diffuse_texture);

	names.normal_texture = initTexture("./Textures/water-texture-2-normal.tga");
	glBindTexture(GL_TEXTURE1, names.normal_texture);

}