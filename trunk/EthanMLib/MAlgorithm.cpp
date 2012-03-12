#include <stdio.h>
#include<vector>
#include<iostream>
#include <queue>
#include "Motorcycle.h"
#include "Segment2D.h"
#include "MAlgorithm.h"
using namespace std;
int CompareCrashEvent(const void * elem1, const void * elem2)
{
	CrashEvent* e1 = (CrashEvent*)elem1;
	CrashEvent* e2 = (CrashEvent*)elem2;
	double tmp = (e1->time - e2->time);
	if(tmp > 0)
		return 1;
	return -1;
}

//vector<Segment2D*>* getMotorcycleGraphBF(Motorcycle* motors, int num)
void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, std::vector<QLineF>& result)
{
	result.clear();
	//vector<Segment2D*> *result = new vector<Segment2D*>();
	int num = motors.size();
	CrashEvent* events = new CrashEvent[num * num];
	for(int i = 0; i < num; i++)
	{
		for(int j = 0; j < num; j++)
		{
			int index = i * num + j;
			if(i == j)
			{
				events[index].startMotor = i;
				events[index].endMotor = j;
				events[index].time = -1.0;
				continue;
			}

			events[index].startMotor = i;
			events[index].endMotor = j;
			events[index].time = motors[i].getCrashTime(motors[j], &events[index]);
		}
	}
	qsort(events, num * num, sizeof(CrashEvent), CompareCrashEvent);

	bool *hasCrashed = new bool[num];
	double *crashTime = new double[num];
	for(int i = 0; i < num; i++)
		hasCrashed[i] = false;

	for(int i = 0; i < num * num; i++)
	{
		CrashEvent *e = &events[i];
		if(e->time < 0)
			continue;
		if(hasCrashed[e->startMotor])
			continue;
		if(hasCrashed[e->endMotor] && e->othertime > crashTime[e->endMotor])
			continue;
		hasCrashed[e->startMotor] = true;
		crashTime[e->startMotor] = e->time;

		/*Segment2D* s = new Segment2D();
		s->startPoint = motors[e->startMotor].position;
		s->endPoint.x = motors[e->startMotor].position.x + motors[e->startMotor].speed.x * e->time;
		s->endPoint.y = motors[e->startMotor].position.y + motors[e->startMotor].speed.y * e->time;*/
		QLineF s(motors[e->startMotor].position, 
			motors[e->startMotor].position + (motors[e->startMotor].speed * e->time).toPointF());

		//result->push_back(s);
		result.push_back(s);
	}
	for(int i = 0; i < num; i++)
	{
		if(hasCrashed[i])
			continue;

		/*Segment2D* s = new Segment2D();
		s->startPoint = motors[i].position;
		s->endPoint.x = motors[i].position.x + motors[i].speed.x * 10000;
		s->endPoint.y = motors[i].position.y + motors[i].speed.y * 10000;*/
		QLineF s(motors[i].position, motors[i].position + (motors[i].speed * 10000).toPointF());

		//result->push_back(s);
		result.push_back(s);
	}

	delete[] events;
	delete[] hasCrashed;
	delete[] crashTime;
	//return result;
}
void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<QLineF>& result)
{
	result.clear();
	//vector<Segment2D*> *result = new vector<Segment2D*>();
	int num = motors.size();
	int wallNum = walls.size();
	CrashEvent* events = new CrashEvent[num * (num + wallNum)];
	int index = 0;
	for(int i = 0; i < num; i++)
	{
		for(int j = 0; j < num; j++, index++)
		{			
			if(i == j)
			{
				events[index].startMotor = i;
				events[index].endMotor = j;
				events[index].time = -1.0;
				continue;
			}
			events[index].isCrashToWall = false;
			events[index].startMotor = i;
			events[index].endMotor = j;
			events[index].time = motors[i].getCrashTime(motors[j], &events[index]);
		}
	}
	for(int i = 0; i < num; i++)
	{
		for(int j = 0; j < wallNum; j++, index++)
		{			
			if(i == j)
			{
				events[index].startMotor = i;
				events[index].endMotor = j;
				events[index].time = -1.0;
				continue;
			}
			events[index].isCrashToWall = true;
			events[index].startMotor = i;
			events[index].endMotor = j;
			events[index].time = motors[i].getCrashTime(walls[j], &events[index]);
		}
	}
	qsort(events, num * (num + wallNum), sizeof(CrashEvent), CompareCrashEvent);

	bool *hasCrashed = new bool[num];
	double *crashTime = new double[num];
	for(int i = 0; i < num; i++)
		hasCrashed[i] = false;

	for(int i = 0; i < num * num; i++)
	{
		CrashEvent *e = &events[i];
		if(e->time < 0)
			continue;
		if(!e->isCrashToWall)
		{
			if(hasCrashed[e->startMotor])
				continue;
			if(hasCrashed[e->endMotor] && e->othertime > crashTime[e->endMotor])
				continue;
		}
		else
		{
			if(hasCrashed[e->startMotor])
				continue;
		}
		hasCrashed[e->startMotor] = true;
		crashTime[e->startMotor] = e->time;

		/*Segment2D* s = new Segment2D();
		s->startPoint = motors[e->startMotor].position;
		s->endPoint.x = motors[e->startMotor].position.x + motors[e->startMotor].speed.x * e->time;
		s->endPoint.y = motors[e->startMotor].position.y + motors[e->startMotor].speed.y * e->time;*/
		QLineF s(motors[e->startMotor].position, 
			motors[e->startMotor].position + (motors[e->startMotor].speed * e->time).toPointF());

		//result->push_back(s);
		result.push_back(s);
	}
	for(int i = 0; i < num; i++)
	{
		if(hasCrashed[i])
			continue;

		/*Segment2D* s = new Segment2D();
		s->startPoint = motors[i].position;
		s->endPoint.x = motors[i].position.x + motors[i].speed.x * 10000;
		s->endPoint.y = motors[i].position.y + motors[i].speed.y * 10000;*/
		QLineF s(motors[i].position, motors[i].position + (motors[i].speed * 10000).toPointF());

		//result->push_back(s);
		result.push_back(s);
	}

	delete[] events;
	delete[] hasCrashed;
	delete[] crashTime;
}


struct TmpEle
{
	int index;
	double time;
	friend bool operator < (struct TmpEle a, struct TmpEle b)
	{
		return a.time > b.time;
	}
};

void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<struct MotorLines>& lines)
{
	points.clear();
	lines.clear();
	priority_queue<TmpEle> *pq = new priority_queue<TmpEle>[motors.size()];
	for(int i = 0; i < motors.size(); i++)
	{
		points.push_back(CrashPoint(motors[i].position, ReflexVertice, -1));
		TmpEle e;
		e.index = i;
		e.time = motors[i].startTime;
		pq[i].push(e);
	}
	//vector<Segment2D*> *result = new vector<Segment2D*>();

	int num = motors.size();
	int wallNum = walls.size();
	CrashEvent* events = new CrashEvent[num * (num + wallNum)];
	int index = 0;
	for(int i = 0; i < num; i++)
	{
		for(int j = 0; j < num; j++, index++)
		{			
			if(i == j)
			{
				events[index].startMotor = i;
				events[index].endMotor = j;
				events[index].time = -1.0;
				continue;
			}
			events[index].isCrashToWall = false;
			events[index].startMotor = i;
			events[index].endMotor = j;
			events[index].time = motors[i].getCrashTime(motors[j], &events[index]);
		}
	}
	for(int i = 0; i < num; i++)
	{
		for(int j = 0; j < wallNum; j++, index++)
		{			

			events[index].isCrashToWall = true;
			events[index].startMotor = i;
			events[index].endMotor = j;
			events[index].time = motors[i].getCrashTime(walls[j], &events[index]);
		}
	}
	qsort(events, num * (num + wallNum), sizeof(CrashEvent), CompareCrashEvent);

	bool *hasCrashed = new bool[num];
	double *crashTime = new double[num];
	int *crashIndex = new int[num];
	for(int i = 0; i < num; i++)
		hasCrashed[i] = false;

	for(int i = 0; i < num * (num + wallNum); i++)
	{
		CrashEvent *e = &events[i];
		if(e->time < 0)
			continue;
		if(!e->isCrashToWall)
		{
			if(hasCrashed[e->startMotor])
				continue;
			if(hasCrashed[e->endMotor] && e->othertime > crashTime[e->endMotor])
				continue;
		}
		else
		{
			if(hasCrashed[e->startMotor])
				continue;
		}
		hasCrashed[e->startMotor] = true;
		crashTime[e->startMotor] = e->time;
		if(!e->isCrashToWall)
		{
			if(e->time - e->othertime < EPSINON && e->time - e->othertime > -EPSINON)
			{
				if(hasCrashed[e->endMotor])
				{

					struct MotorLines line;
					line.start = e->startMotor;
					line.end = crashIndex[e->endMotor];
					crashIndex[e->startMotor] = line.end;
					//lines.push_back(line);
					TmpEle e1;
					e1.index = crashIndex[e->endMotor];
					e1.time = e->time;
					pq[e->startMotor].push(e1);
				}
				else
				{
					points.push_back(CrashPoint(motors[e->startMotor].position + (motors[e->startMotor].speed * e->time).toPointF(), MovingVertice, -1));
					struct MotorLines line;
					line.start = e->startMotor;
					line.end = points.size() - 1;
					crashIndex[e->startMotor] = line.end;
					//lines.push_back(line);

					TmpEle e1;
					e1.index = points.size() - 1;
					e1.time = e->time;
					pq[e->startMotor].push(e1);

				}
			}
			else
			{
				points.push_back(CrashPoint(motors[e->startMotor].position + (motors[e->startMotor].speed * e->time).toPointF(), MovingVertice, -1));
				struct MotorLines line;
				line.start = e->startMotor;
				line.end = points.size() - 1;
				crashIndex[e->startMotor] = line.end;
				//lines.push_back(line);
				TmpEle e1;
				e1.index = points.size() - 1;
				e1.time = e->time;
				pq[e->startMotor].push(e1);
				TmpEle e2;
				e2.index = points.size() - 1;
				e2.time = e->othertime;
				pq[e->endMotor].push(e2);
			}
		}
		else
		{
			points.push_back(CrashPoint(motors[e->startMotor].position + (motors[e->startMotor].speed * e->time).toPointF(), WallVertice, e->endMotor));
			struct MotorLines line;
			line.start = e->startMotor;
			line.end = points.size() - 1;
			//line.time = e->time;
			crashIndex[e->startMotor] = line.end;
			//lines.push_back(line);
			TmpEle e1;
			e1.index = points.size() - 1;
			e1.time = e->time;
			pq[e->startMotor].push(e1);


		}

		/*Segment2D* s = new Segment2D();
		s->startPoint = motors[e->startMotor].position;
		s->endPoint.x = motors[e->startMotor].position.x + motors[e->startMotor].speed.x * e->time;
		s->endPoint.y = motors[e->startMotor].position.y + motors[e->startMotor].speed.y * e->time;*/


		//result->push_back(s);
		//result.push_back(s);
	}
	/*for(int i = 0; i < num; i++)
	{
	if(hasCrashed[i])
	continue;

	/ *Segment2D* s = new Segment2D();
	s->startPoint = motors[i].position;
	s->endPoint.x = motors[i].position.x + motors[i].speed.x * 10000;
	s->endPoint.y = motors[i].position.y + motors[i].speed.y * 10000;* /

	points.push_back(CrashPoint(motors[i].position + (motors[i].speed * 10000).toPointF(), WallVertice, ));
	int line[2];
	line[0] = i;
	line[1] = points.size() - 1;
	crashIndex[i] = line[1];
	lines.push_back(line);
	//result->push_back(s);
	//result.push_back(s);
	}*/
	for(int i = 0; i < motors.size(); i++)
	{
		TmpEle p = pq[i].top();
		pq[i].pop();
		while(!pq[i].empty())
		{
			TmpEle q = pq[i].top();
			pq[i].pop();
			struct MotorLines line;
			line.start = p.index;
			line.end = q.index;
			lines.push_back(line);
			p = q;
		}
		if(!hasCrashed[i])
		{
			CrashPoint cp;
			cp.type = InfiniteVertice;
			cp.p = motors[i].position + (motors[i].speed.normalized() * 10000).toPointF();
			cp.wallNo = -1;
			points.push_back(cp);
			MotorLines line;
			line.start = p.index;
			line.end = points.size() - 1;
			lines.push_back(line);
		}
	}
	delete[] events;
	delete[] hasCrashed;
	delete[] crashTime;
}