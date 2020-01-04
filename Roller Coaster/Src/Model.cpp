#include "model.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVarLengthArray>

#include <QtOpenGL/QtOpenGL>

bool Model::firstInitShader = false;
QOpenGLShader*  Model::vertexShader = NULL;
QOpenGLShader*  Model::geometryShader = NULL;
QOpenGLShader*  Model::fragmentShader = NULL;
void DimensionTransformation(GLfloat source[], GLfloat target[][4])
{
	//for uniform value, transfer 1 dimension to 2 dimension
	int i = 0;
	for (int j = 0; j < 4; j++)
		for (int k = 0; k < 4; k++)
		{
			target[j][k] = source[i];
			i++;
		}
}

Model::Model(const QString &filePath, int s, Point3d p)
	: m_fileName(QFileInfo(filePath).fileName())
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
		return;

	boundsMin = Point3d( 1e9, 1e9, 1e9);
	boundsMax = Point3d(-1e9,-1e9,-1e9);
	scale = s;
	position = p;

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
		} else if (id == "f" || id == "fo") {
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
		const Point3d b = m_points.at(m_pointIndices.at(i+1));
		const Point3d c = m_points.at(m_pointIndices.at(i+2));

		const Point3d normal = cross(b - a, c - a).normalize();

		for (int j = 0; j < 3; ++j)
			m_target_normals[m_pointIndices.at(i + j)] += normal;
	}

	for (int i = 0; i < m_target_normals.size(); ++i)
		m_target_normals[i] = m_target_normals[i].normalize();

	Init();
}

void Model::render(bool wireframe, bool normals)
{
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	if (wireframe) {
		glVertexPointer(3, GL_FLOAT, 0, (float *)m_points.data());
		glDrawElements(GL_LINES, m_edgeIndices.size(), GL_UNSIGNED_INT, m_edgeIndices.data());
	} else {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glShadeModel(GL_SMOOTH);

		Point3d bounds = boundsMax - boundsMin;
		Point3d boundedContant = (boundsMin + bounds * 0.5);
		GLfloat boundScale = this->scale / qMax(bounds.x, qMax(bounds.y, bounds.z));
		GLfloat P[4][4];
		GLfloat MV[4][4];
		GLfloat ModelViewMatix[16], ProjectionMatrix[16];
		QVector3D inputPos = { position.x, position.y, position.z };
		QVector3D inputRot = { rotation.x, rotation.y, rotation.z };
		QVector3D inputConst = { boundedContant.x,boundedContant.y,boundedContant.z };
		glGetFloatv(GL_MODELVIEW_MATRIX, ModelViewMatix);
		glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrix);
		DimensionTransformation(ProjectionMatrix, P);
		DimensionTransformation(ModelViewMatix, MV);


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

		vvbo.bind();
		shaderProgram->enableAttributeArray(0);
		shaderProgram->setAttributeArray(0, GL_FLOAT, 0, 3, NULL);
		vvbo.release();

		glDrawElements(GL_TRIANGLES, m_pointIndices.size(), GL_UNSIGNED_INT, m_pointIndices.data());
		shaderProgram->disableAttributeArray(0);
		vao.release();
		shaderProgram->release();

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}

	if (normals) {
		QVector<Point3d> normals;
		for (int i = 0; i < m_target_normals.size(); ++i)
			normals << m_points.at(i) << (m_points.at(i) + m_target_normals.at(i) * 0.02f);
		glVertexPointer(3, GL_FLOAT, 0, (float *)normals.data());
		glDrawArrays(GL_LINES, 0, normals.size());
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_DEPTH_TEST);
}

void Model::updateRotation(Point3d rotationDir, Point3d p)
{
	if (!(p == position && rotationDir == rotation))
	{
		/*const Point3d bounds = boundsMax - boundsMin;
		const qreal scale = this->scale / qMax(bounds.x, qMax(bounds.y, bounds.z));*/

		/*for (int i = 0; i < m_points.size(); ++i)
			m_target_points[i] = (m_points[i].m(rotationDir.x, rotationDir.y, rotationDir.z) + p - (boundsMin + bounds * 0.5)) * scale;

		m_target_normals.resize(m_points.size());*/
		//for (int i = 0; i < m_pointIndices.size(); i += 3) {
		//	const Point3d a = m_points.at(m_pointIndices.at(i));
		//	const Point3d b = m_points.at(m_pointIndices.at(i + 1));
		//	const Point3d c = m_points.at(m_pointIndices.at(i + 2));

		//	const Point3d normal = cross(b - a, c - a).normalize();

		//	for (int j = 0; j < 3; ++j)
		//		m_target_normals[m_pointIndices.at(i + j)] = normal.normalize();

		//}

		position = p;
		this->rotation = rotationDir;
	}
}

void Model::Init()
{
	InitShader("./Shader/Model.vs", "./Shader/Model.fs", "./Shader/Model.gs");
	InitVAO();
	InitVBO();
}
void Model::InitVAO()
{
	// Create Vertex Array Object
	vao.create();
	// Bind the VAO so it is the current active VAO
	vao.bind();
}
void Model::InitVBO()
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
void Model::InitShader(QString vertexShaderPath, QString fragmentShaderPath, QString geomoetryShaderPath)
{

	// Create Shader
	shaderProgram = new QOpenGLShaderProgram();
	if (!firstInitShader)
	{
		firstInitShader = true;
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
	}
	else 
	{
		shaderProgram->addShader(vertexShader);
		shaderProgram->addShader(geometryShader);
		shaderProgram->addShader(fragmentShader);
	}

	
	shaderProgram->link();
}