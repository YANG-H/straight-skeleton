#include "Util.h"
#include "MCConstants.h"
int toLeft(QLineF line, QPointF p)
{
	QPointF p1 = line.p1() - p;
	QPointF p2 = line.p2() - p;

	float nRet = p1.x() * p2.y() - p1.y() * p2.x();
	if (nRet < EPSINON && nRet > -EPSINON)
		return 0;
	else if (nRet > 0)
		return 1;
	else if (nRet < 0)
		return -1;
	return 0;
}
bool IsTwoLineIntersect(QLineF line1, QLineF line2)
{
	int nLine1Start = toLeft(line2, line1.p1());
	int nLine1End = toLeft(line2, line1.p2());
	if (nLine1Start * nLine1End > 0)
		return false;
	int nLine2Start = toLeft(line1, line2.p1());
	int nLine2End = toLeft(line1, line2.p2());
	if (nLine2Start * nLine2End > 0)
		return false;
	return true;
}
bool IsLineIntersectRect(QLineF line, MCCell* c)
{
	if(line.p1().x() >= c->leftup.x() && line.p1().x() <= c->leftup.x() + c->width
		&& line.p1().y() >= c->leftup.y() && line.p1().y() <= c->leftup.x() + c->height)
		return true;
	if(line.p2().x() >= c->leftup.x() && line.p2().x() <= c->leftup.x() + c->width
		&& line.p2().y() >= c->leftup.y() && line.p2().y() <= c->leftup.y() + c->height)
		return true;
	if(IsTwoLineIntersect(line, QLineF(c->leftup, QPointF(c->leftup.x() + c->width, c->leftup.y()))))
		return true;
	if(IsTwoLineIntersect(line, QLineF(c->leftup, QPointF(c->leftup.x(), c->leftup.y() + c->height))))
		return true;
	if(IsTwoLineIntersect(line, QLineF(QPointF(c->leftup.x() + c->width, c->leftup.y() + c->height), QPointF(c->leftup.x() + c->width, c->leftup.y()))))
		return true;
	if(IsTwoLineIntersect(line, QLineF(QPointF(c->leftup.x() + c->width, c->leftup.y() + c->height), QPointF(c->leftup.x(), c->leftup.y() + c->height))))
		return true;
	return false;
}

bool isSamePoint(CrashPoint p1, CrashPoint p2)
{
	if(p1.p.x() - p2.p.x() < EPSINON && p1.p.x() - p2.p.x() > -EPSINON
		&& p1.p.y() - p2.p.y() < EPSINON && p1.p.y() - p2.p.y() > -EPSINON)
		return true;
	return false;
}