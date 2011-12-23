#pragma once
#include "Segment2D.h"
#include "Motorcycle.h"
#include <vector>
#include "CrashPoint.h"
using namespace std;

// refactored
//vector<Segment2D*>* getMotorcycleGraphBF(Motorcycle* motors, int num);

void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, 
	/*OUT*/ std::vector<QLineF>& result);

void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<QLineF>& result);

void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<MotorLines>& lines);

void GetMotorcycleGraph(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<struct MotorLines>& lines);

