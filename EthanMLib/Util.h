#pragma once
#include<QPointF>
#include <QLineF>
#include "CrashPoint.h"
#include "MCCell.h"
int toLeft(QLineF line, QPointF p);
bool IsTwoLineIntersect(QLineF line1, QLineF line2);
bool IsLineIntersectRect(QLineF line, MCCell* c);

bool isSamePoint(CrashPoint p1, CrashPoint p2);