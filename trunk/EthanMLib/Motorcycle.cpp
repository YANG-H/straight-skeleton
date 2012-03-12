#include "Motorcycle.h"
#include<math.h>
#include "Util.h"
Motorcycle::Motorcycle()
{

}

Motorcycle::Motorcycle(const QPointF& p, const QVector2D& v, double time)
	: position(p), speed(v), startTime(time)
{}
Motorcycle::Motorcycle(const QPointF& p, const QVector2D& v, double time, int mid)
	: position(p), speed(v), startTime(time), id(mid)
{
	this->id = mid;
}

Motorcycle::Motorcycle(const Motorcycle& m)
	: position(m.position), speed(m.speed), startTime(m.startTime), id(m.id), leftPoint(m.leftPoint), rightPoint(m.rightPoint)
{}

Motorcycle& Motorcycle::operator = (const Motorcycle& m)
{
	Motorcycle::Motorcycle(m);
	return *this;
}

double Motorcycle::getCrashTime(const Motorcycle& m, CrashEvent* e) const
{
	/*Vector2D* n;
	if(m->speed.x < EPSINON && m->speed.x > - EPSINON)
	n = new Vector2D(1, 0);
	else if(m->speed.y < EPSINON && m->speed.y > - EPSINON)
	n = new Vector2D(0, 1);
	else
	{
	double tx = -1.0 / m->speed.x;
	double ty = 1.0 / m->speed.y;
	double ntx = tx / sqrt(tx * tx + ty * ty);
	double nty = ty / sqrt(tx * tx + ty * ty);
	n = new Vector2D(ntx, nty);
	}
	double ld = m->position.x * n->x + m->position.y * n->y;
	double dis = n->x * position.x + n->y * position.y;
	double td = n->x *(position.x + speed.x) + n->y * (position.y * speed.y);
	double k = dis / (dis - td);
	Point2D *p = new Point2D(speed.x, speed.y);
	p->x *= k;
	p->y *= k;
	p->x += position.x;
	p->y += position.y;
	double t1 = (p->x - position.x) / speed.x;
	double t2 = (p->x - m->position.x) / m->speed.x;
	if(t1 < t2)
	return -1;
	e->othertime = t2;
	return t1;*/
	/*if(m->speed.x < EPSINON && m->speed.x > - EPSINON)
	{
	if(speed.x < EPSINON && speed.x > -EPSINON)
	return -1.0;
	double t1 = (m->position.x - position.x) / speed.x;
	double t2 = speed.y * t1 / m->speed.y;
	e->othertime = t2;
	return t1;
	}
	if(m->speed.y < EPSINON && m->speed.y > - EPSINON)
	{
	if(speed.y < EPSINON && speed.y > -EPSINON)
	return -1.0;

	double t1 = (m->position.y - position.y) / speed.y;
	double t2 = speed.x * t1 / m->speed.x;
	e->othertime = t1;
	return t1;
	}
	double k1 = - this->speed.x / m->speed.x;
	double k2 = - this->speed.y / m->speed.y;

	if(k1 - k2 < EPSINON && k1 - k2 > -EPSINON)
	return -1.0;

	double c = (m->position.x - position.x) / m->speed.x
	- (m->position.y - position.y) / m->speed.y;
	double t1 = c / (k2 - k1);
	double t2 = - (m->position.x - position.x - speed.x * t1) / m->speed.x;
	if(t1 < t2)
	return -1.0;
	if(t1 < EPSINON)
	return -1.0;
	if(t2 < EPSINON)
	return -1.0;
	e->othertime = t2;
	return t1;*/
	double t1, t2;
	double c1 = m.position.x() - position.x();
	double c2 = m.position.y() - position.y();
	if(speed.x() < EPSINON && speed.x() > - EPSINON)
	{
		if(speed.y() < EPSINON && speed.y() > -EPSINON)
		{
			return -1.0;
		}
		else if(m.speed.x() < EPSINON && m.speed.x() > - EPSINON)
		{
			if(c1 > EPSINON || c1 < -EPSINON)
				return -1.0;
			if(m.speed.y() < EPSINON && m.speed.y() > -EPSINON)
				return -1.0;
			t1 = c2 / speed.y();
			t2 = - c2 / m.speed.y();
			if(t1 * t2 < 0)
			{
				if(t1 < 0)
					t1 = 0;
				else
					t2 = 0;
			}
			else if(t1 > 0 && t2 > 0)
			{
				t2 = t1 = c2 / (speed.y() - m.speed.y());
			}
		}
		else
		{
			t2 = - c1 / m.speed.x();
			t1 = (c2 + t2 * m.speed.y()) / speed.y();
		}
	}
	else
	{

		double coe = speed.y() * m.speed.x() - speed.x() * m.speed.y();
		t2 = (c2 * speed.x() - speed.y() * c1) / coe;
		t1 = (c1 + m.speed.x() * t2) / speed.x();
	}

	if(t1 < t2)
		t1 = -1.0;
	if(t1 < 0 || t2 < 0)
		t1 = -1.0;
	e->othertime = t2;
	return t1;
}
double Motorcycle::getCrashTime(const QLineF& line, CrashEvent* e) const
{
	if(position.x() - line.p1().x() < EPSINON && position.x() - line.p1().x() > -EPSINON
		&& position.y() - line.p1().y() < EPSINON && position.y() - line.p1().y() > -EPSINON)
		return -1.0;
	if(position.x() - line.p2().x() < EPSINON && position.x() - line.p2().x() > -EPSINON
		&& position.y() - line.p2().y() < EPSINON && position.y() - line.p2().y() > -EPSINON)
		return -1.0;
	Motorcycle m(line.p1(), QVector2D(line.p2() - line.p1()), 0);
	double time = getCrashTime(m, e);
	if(time < 0)
	{
		m.position = line.p2();
		m.speed = -m.speed;
		time = getCrashTime(m, e);
		if(time < 0)
			return -1.0;
	}
	
	QLineF line2(position, position + (speed * (time + 1.0)).toPointF());
	if(!IsTwoLineIntersect(line, line2))
		return -1.0;
	
	return time;
}