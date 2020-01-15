#include "StarRing.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVarLengthArray>

#include <QtOpenGL/QtOpenGL>
#include <QtGui/qmatrix.h>
#include <time.h>


//#define DEBUG_FLAG

GLuint StarRing::skyboxShaderID = 0;
float StarRing::DEGREE_TO_RADIANT = 3.1415926f / 180.f;
float StarRing::RADIANT_TO_DEGREE = 180.f / 3.1415926f;

StarRing::StarRing(const QString &filePath, int s, Point3d p, ShaderType type)
	: m_fileName(QFileInfo(filePath).fileName())
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << file.errorString() << endl;
		return;
	}

	boundsMin = Point3d(1e9, 1e9, 1e9);
	boundsMax = Point3d(-1e9, -1e9, -1e9);
	scale = s;
	position = p;
	centerPoistion = QVector3D(0, 0, 0);

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString input = in.readLine();
		if (input.isEmpty() || input[0] == '#')
			continue;

		QTextStream ts(&input);
		QString id;
		ts >> id;
		if (id == "v") {
			Point3d p;
			for (int i = 0; i < 3; ++i) {
				ts >> p[i];
				boundsMin[i] = qMin(boundsMin[i], p[i]);
				boundsMax[i] = qMax(boundsMax[i], p[i]);
			}
			m_points << p;
		}
		else if (id == "f" || id == "fo") {
			QVarLengthArray<int, 4> p;

			while (!ts.atEnd()) {
				QString vertex;
				ts >> vertex;
				const int vertexIndex = vertex.split('/').value(0).toInt();
				if (vertexIndex)
					p.append(vertexIndex > 0 ? vertexIndex - 1 : m_points.size() + vertexIndex);
			}

			for (int i = 0; i < p.size(); ++i) {
				const int edgeA = p[i];
				const int edgeB = p[(i + 1) % p.size()];

				if (edgeA < edgeB)
					m_edgeIndices << edgeA << edgeB;
			}

			for (int i = 0; i < 3; ++i)
				m_pointIndices << p[i];

			if (p.size() == 4)
				for (int i = 0; i < 3; ++i)
					m_pointIndices << p[(i + 2) % 4];
		}
	}

	const Point3d bounds = boundsMax - boundsMin;
	const qreal scale = s / qMax(bounds.x, qMax(bounds.y, bounds.z));

	m_target_normals.resize(m_points.size());

	for (int i = 0; i < m_pointIndices.size(); i += 3) {
		const Point3d a = m_points.at(m_pointIndices.at(i));
		const Point3d b = m_points.at(m_pointIndices.at(i + 1));
		const Point3d c = m_points.at(m_pointIndices.at(i + 2));

		const Point3d normal = cross(b - a, c - a).normalize();

		for (int j = 0; j < 3; ++j)
			m_target_normals[m_pointIndices.at(i + j)] += normal;
	}

	for (int i = 0; i < m_target_normals.size(); ++i)
		m_target_normals[i] = m_target_normals[i].normalize();

	file.close();
	shadertype = type;
	texture = new QOpenGLTexture(QImage("./Textures/Rock-Texture-Surface.jpg"));
	Init();
}
StarRing::StarRing(StarRing* copyVersion)
{
	shadertype = copyVersion->shadertype;
	boundsMin = copyVersion->boundsMin;
	boundsMax = copyVersion->boundsMax;
	scale = copyVersion->scale;
	m_points = copyVersion->m_points;
	m_edgeIndices = copyVersion->m_edgeIndices;
	m_pointIndices = copyVersion->m_pointIndices;
	shaderProgram = copyVersion->shaderProgram;
	InitVAO();
	InitVBO();
}

void StarRing::render(GLfloat P[][4], GLfloat MV[][4], bool wireframe, bool normals)
{
	glEnable(GL_DEPTH_TEST);
	/*glEnableClientState(GL_VERTEX_ARRAY);*/
	if (wireframe) {
		glVertexPointer(3, GL_FLOAT, 0, (float *)m_points.data());
		glDrawElements(GL_LINES, m_edgeIndices.size(), GL_UNSIGNED_INT, m_edgeIndices.data());
	}
	else {
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
		shaderProgram->setUniformValue("center", centerPoistion);

		/*if (shadertype == REFLECTION || shadertype == REFRACTION)
			shaderProgram->setUniformValue("cameraPos", eyePosition);*/

		vvbo.bind();
		shaderProgram->enableAttributeArray(0);
		shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);
		/*if (shadertype == REFLECTION || shadertype == REFRACTION)
			glBindTexture(GL_TEXTURE_CUBE_MAP, StarRing::skyboxShaderID);*/
		vvbo.release();
		GLuint textureID = texture->textureId();
		for (int i = 0; i < amount; i++)
		{
			GLfloat inst[4][4] =
			{
				{modelMatrices[i][0][0],modelMatrices[i][0][1],modelMatrices[i][0][2],modelMatrices[i][0][3]},
				{modelMatrices[i][1][0],modelMatrices[i][1][1],modelMatrices[i][1][2],modelMatrices[i][1][3]},
				{modelMatrices[i][2][0],modelMatrices[i][2][1],modelMatrices[i][2][2],modelMatrices[i][2][3]},
				{modelMatrices[i][3][0],modelMatrices[i][3][1],modelMatrices[i][3][2],modelMatrices[i][3][3]},
			};
			glBindTexture(GL_TEXTURE_2D, textureID);
			shaderProgram->setUniformValue("instanceMatrix", inst);
			glDrawElements(GL_TRIANGLES, m_pointIndices.size(), GL_UNSIGNED_INT, m_pointIndices.data());
		}
		shaderProgram->disableAttributeArray(0);
		vao.release();
		shaderProgram->release();

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);

#ifdef DEBUG_FLAG
		if (shadertype == TRAIN)
		{
			glBegin(GL_LINES);
			for (int i = 0; i < 3; i++)
			{
				glVertex3f(position.x, position.y, position.z);
				Point3d targetPoint = position + Point3d(rotMatrix[0][i], rotMatrix[1][i], rotMatrix[2][i]) * 15.f;
				glVertex3f(targetPoint.x, targetPoint.y, targetPoint.z);
			}
			glEnd();
		}
#endif // DEBUG_FLAG

	}

	if (normals) {
		QVector<Point3d> normals;
		for (int i = 0; i < m_target_normals.size(); ++i)
			normals << m_points.at(i) << (m_points.at(i) + m_target_normals.at(i) * 0.02f);
		glVertexPointer(3, GL_FLOAT, 0, (float *)normals.data());
		glDrawArrays(GL_LINES, 0, normals.size());
	}
	/*glDisableClientState(GL_VERTEX_ARRAY);*/
	glDisable(GL_DEPTH_TEST);
}

void StarRing::updateRotation(Point3d f_rotationDir, Point3d f_position)
{
	rotation = f_rotationDir;
	position = f_position;
}

void StarRing::updateRotation(Point3d f_position, Point3d f_tangent, Point3d f_normal)
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

void StarRing::Init()
{
	InitShader("./Shader/StarRing.vs", "./Shader/StarRing.fs", "./Shader/StarRing.gs");
	InitVAO();
	InitVBO();
	
	amount = 10000;
	modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(clock()); // 初始化随机种子    
	float radius = 180.0;
	float offset =9.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model;
		// 1. 位移：分布在半径为 'radius' 的圆形上，偏移的范围是 [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 1.2f; // 让行星带的高度比x和z的宽度要小
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. 缩放：在 0.05 和 0.25f 之间缩放
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. 旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. 添加到矩阵的数组中
		modelMatrices[i] = model;
	}
}
void StarRing::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
void StarRing::InitVBO()
{
	// Create Buffer for position
	vvbo.create();
	// Bind the buffer so that it is the current active buffer.
	vvbo.bind();
	// Since we will never change the data that we are about to pass the Buffer, we will say that the Usage Pattern is StaticDraw
	vvbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	// Allocate and initialize the information
	vvbo.allocate(m_points.constData(), m_points.size() * sizeof(Point3d));
}
void StarRing::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
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