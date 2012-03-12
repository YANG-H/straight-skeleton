#pragma once;
#include <set>
#include <QPointF>
#include <vector>
using namespace std;
class MCCell
{
public:
	int index;
	QPointF leftup;
	float width;
	float height;
	set<int> points;
	vector<int> walls;
public:
	
};