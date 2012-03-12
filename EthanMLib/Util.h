#pragma once
#include <QPointF>
#include <QLineF>
#include "CrashPoint.h"
#include "MCCell.h"
#include "ethanmlib_global.h"


ETHANMLIB_EXPORT int toLeft(const QLineF& line, const QPointF& p);
ETHANMLIB_EXPORT bool IsTwoLineIntersect(const QLineF& line1, const QLineF& line2);
ETHANMLIB_EXPORT bool IsLineIntersectRect(const QLineF& line, MCCell* c);

ETHANMLIB_EXPORT bool isSamePoint(const CrashPoint& p1, const CrashPoint& p2);