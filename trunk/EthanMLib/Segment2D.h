#pragma once
#include "Point2D.h"

// use QLineF instead // yanghao

class Segment2D
{
public:
	Point2D startPoint;
	Point2D endPoint;
public:
	Segment2D();
	Segment2D(Point2D s, Point2D e);
};
