#pragma once

#include <vector>

#include <QVector2D>
#include <QPointF>
#include <QLineF>

#include "Segment2D.h"
#include "Motorcycle.h"
#include "CrashPoint.h"

#include "ethanmlib_global.h"

using namespace std;

// refactored
//vector<Segment2D*>* getMotorcycleGraphBF(Motorcycle* motors, int num);

ETHANMLIB_EXPORT void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, 
	/*OUT*/ std::vector<QLineF>& result);

ETHANMLIB_EXPORT void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<QLineF>& result);

ETHANMLIB_EXPORT void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<MotorLines>& lines);

ETHANMLIB_EXPORT void GetMotorcycleGraph(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<struct MotorLines>& lines);

