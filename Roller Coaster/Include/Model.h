#ifndef MODEL_H
#define MODEL_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include <math.h>

#include "point3d.h"

class Model
{
public:
	Model() {}
	Model(const QString &filePath, int s, Point3d p);

	void render(bool wireframe = false, bool normals = false) const;

	QString fileName() const { return m_fileName; }
	int faces() const { return m_pointIndices.size() / 3; }
	int edges() const { return m_edgeIndices.size() / 2; }
	int points() const { return m_points.size(); }

	void updatePosition(Point3d);
	void updateRotation(float, Point3d);
private:
	Point3d boundsMin;
	Point3d boundsMax;
	int scale;
	QString m_fileName;

	Point3d position;
	Point3d normal;

	QVector<Point3d> m_points;
	QVector<Point3d> m_normals;
	QVector<Point3d> m_target_points;
	QVector<Point3d> m_target_normals;
	QVector<int> m_edgeIndices;
	QVector<int> m_pointIndices;
};

#endif