#pragma once

#include <QPointF>
#include <QVector2D>
#include <QLineF>

#include "ethanmlib_global.h"

//#include "Point2D.h"
//#include "Vector2D.h"
#include "CrashEvent.h"
#include "MCConstants.h"
class ETHANMLIB_EXPORT Motorcycle
{
public:
	int id;
	QPointF position;
	QVector2D speed;
	double startTime;
	int crashIndex;
	QPointF leftPoint;
	QPointF rightPoint;
public:
	Motorcycle();
	Motorcycle(const QPointF& p, const QVector2D& v, double time);
	Motorcycle(const QPointF& p, const QVector2D& v, double time, int id);
	Motorcycle(const Motorcycle& m);

	Motorcycle& operator = (const Motorcycle& m);

public:
	double getCrashTime(const Motorcycle& m, CrashEvent* e) const;
	double getCrashTime(const QLineF& line, CrashEvent* e) const;
	void setID(int id)
	{
		this->id = id;
	}
};
