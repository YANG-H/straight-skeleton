#include "MAlgorithm.h"
#include "CrashEvent.h"
#include "MCCell.h"
#include <queue>
#include <map>
#include "Util.h"
using namespace std;
priority_queue<MCEvent> MCQ;
struct TmpEle
{
	int index;
	double time;
	friend bool operator < (struct TmpEle a, struct TmpEle b)
	{
		return a.time > b.time;
	}
};
vector<TmpEle> *motorPoints = NULL;
MCCell* Cells = NULL;
bool* isMotorCrashed = NULL;
float* crashTime = NULL;
int motorNum;
int CellSize;
float minX, minY, maxX, maxY;
float offset = 10.0;
float cwidth;
float cheight;
int originPointNum;
int* PointIndex = NULL;
priority_queue<MCEvent>* MotorHash = NULL;
// 判断点在有向直线的左侧还是右侧.
// 返回值:-1: 点在线段左侧; 0: 点在线段上; 1: 点在线段右侧

int locateCell(float x, float y)
{
	int i = (int)((x - minX + offset / 2) / cwidth);
	int j = (int)((y - minY + offset / 2) / cheight);
	return ((j * CellSize) + i);
}
int locateCell(QPointF p)
{
	return locateCell(p.x(), p.y());
}
void reducePoints(std::vector<CrashPoint>& points)
{
	if(PointIndex != NULL)
		delete[] PointIndex;
	originPointNum = points.size();
	PointIndex = new int[originPointNum];
	vector<CrashPoint> newPoints;
	for(int i = 0; i < motorNum; i++)
	{
		PointIndex[i] = i;
		newPoints.push_back(points[i]);		
	}
	for(int i = motorNum; i < originPointNum; i++)
	{
		int j;
		for(j = 0; j < newPoints.size(); j++)
		{
			if(isSamePoint(points[i], newPoints[j]))
			{
				PointIndex[i] = j;
				break;
			}
		}
		if(j == newPoints.size())
		{
			newPoints.push_back(points[i]);
			PointIndex[i] = newPoints.size() - 1;
		}
	}
	points.clear();
	for(int i = 0; i < newPoints.size(); i++)
		points.push_back(newPoints[i]);
}
void clearMC(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<struct MotorLines>& lines)
{
	
	if(motorPoints != NULL)
		delete[] motorPoints;
	if(crashTime != NULL)
		delete[] crashTime;
	if(MotorHash != NULL)
		delete[] MotorHash;
	MotorHash = new priority_queue<MCEvent>[motors.size()];
	crashTime = new float[motors.size()];
	motorPoints = new vector<TmpEle>[motors.size()];
	if(isMotorCrashed != NULL)
		delete[] isMotorCrashed;
	isMotorCrashed = new bool[motors.size()];

	while(!MCQ.empty())
		MCQ.pop();
	if(Cells != NULL)
		delete[] Cells;

	motorNum = motors.size();
	points.clear();
	lines.clear();
	minY = minX = FLT_MAX;
	maxX = maxY = FLT_MIN;
	for(int i = 0; i < motors.size(); i++)
	{
		if(motors[i].position.x() < minX)
			minX = motors[i].position.x();
		if(motors[i].position.x() > maxX)
			maxX = motors[i].position.x();
		if(motors[i].position.y() < minY)
			minY = motors[i].position.y();
		if(motors[i].position.y() > maxY)
			maxY = motors[i].position.y();	
		isMotorCrashed[i] = false;
		TmpEle e;
		e.index = i;
		e.time = 0;
		motorPoints[i].push_back(e);
	}
	for(int i = 0; i < walls.size(); i++)
	{
		if(walls[i].p1().x() < minX)
			minX = walls[i].p1().x();
		if(walls[i].p1().x() > maxX)
			maxX = walls[i].p1().x();
		if(walls[i].p1().y() < minY)
			minY = walls[i].p1().y();
		if(walls[i].p1().y() > maxY)
			maxY = walls[i].p1().y();

		if(walls[i].p2().x() < minX)
			minX = walls[i].p2().x();
		if(walls[i].p2().x() > maxX)
			maxX = walls[i].p2().x();
		if(walls[i].p2().y() < minY)
			minY = walls[i].p2().y();
		if(walls[i].p2().y() > maxY)
			maxY = walls[i].p2().y();
	}

	CellSize = (int)(sqrt((double)motorNum));
	cwidth = (maxX - minX + offset) / CellSize;
	cheight = (maxY - minY + offset) / CellSize;
	Cells = new MCCell[CellSize * CellSize];

	for(int i = 0; i < CellSize * CellSize; i++)
	{
		Cells[i].width = cwidth;
		Cells[i].height = cheight;
		Cells[i].leftup.setX(i % CellSize * cwidth + minX - offset / 2);
		Cells[i].leftup.setY(i / CellSize * cheight + minY - offset / 2);
		Cells[i].points.clear();		
		Cells[i].index = i;
		for(int j = 0; j < walls.size(); j++)
		{
			if(IsLineIntersectRect(walls[j], &(Cells[i])))
			{
				Cells[i].walls.push_back(j);
			}
		}
	}
}
void insertMC(Motorcycle m)
{
	struct MCEvent e;
	e.type = MCSWITCH_EVENT;
	e.key = m.startTime;

	e.s = new struct SwitchEvent;
	//int x = (int)((m.position.x() - minX + offset / 2) / cwidth);
//	int y = (int)((m.position.y() - minY + offset / 2) / cheight);

	e.s->c = &(Cells[locateCell(m.position)]);
	e.id = m.id;

	MCQ.push(e);
}
void handleSwitchEvent(MCEvent e, const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls)
{
	if(e.type != MCSWITCH_EVENT)
		return;
	if(isMotorCrashed[e.id])
		return;
	
	e.s->c->points.insert(e.id);
	/*if(CellIndex[e.id] >= 0)
	{
	set<int>::iterator iter = (Cells[CellIndex[e.id]].points).find(e.id);
	Cells[CellIndex[e.id]].points.erase(iter);
	}*/
	//insert a new switch event
	Motorcycle m = motors[e.id];
	MCCell *c = e.s->c;
	float timeXR = (c->leftup.x() + c->width - m.position.x()) / m.speed.x();
	float XR = m.position.y() + m.speed.y() * timeXR;
	float timeXL = (c->leftup.x() - m.position.x()) / m.speed.x();
	float XL = m.position.y() + m.speed.y() * timeXL;
	float timeYU = (c->leftup.y() - m.position.y()) / m.speed.y();
	float YU = m.position.x() + m.speed.x() * timeYU;
	float timeYD = (c->leftup.y() + c->height - m.position.y()) / m.speed.y();
	float YD = m.position.x() + m.speed.x() * timeYD;
	if(timeXL > e.key && XL >= c->leftup.y() && XL <= c->leftup.y() + c->height)
	{
		if(c->index % CellSize > 0)
		{
			struct MCEvent e;
			e.type = MCSWITCH_EVENT;
			e.key = timeXL;

			e.s = new struct SwitchEvent;			

			e.s->c = &(Cells[c->index - 1]);
			e.id = m.id;
			//e.s->c->points.insert(m.id);
			MCQ.push(e);
		}
	}
	else if(timeXR > e.key && XR >= c->leftup.y() && XR <= c->leftup.y() + c->height)
	{
		if(c->index % CellSize < CellSize - 1)
		{
			struct MCEvent e;
			e.type = MCSWITCH_EVENT;
			e.key = timeXR;

			e.s = new struct SwitchEvent;			

			e.s->c = &(Cells[c->index + 1]);
			e.id = m.id;
			//e.s->c->points.insert(m.id);
			MCQ.push(e);
		}
	}
	else if(timeYU > e.key && YU >= c->leftup.x() && YU <= c->leftup.x() + c->width)
	{
		if(c->index / CellSize > 0)
		{
			struct MCEvent e;
			e.type = MCSWITCH_EVENT;
			e.key = timeYU;

			e.s = new struct SwitchEvent;			

			e.s->c = &(Cells[c->index - CellSize]);
			e.id = m.id;
			//Cells[c->index - CellSize].points.insert(m.id);
			MCQ.push(e);
		}
	}
	else if(timeYD > e.key && YD >= c->leftup.x() && YD <= c->leftup.x() + c->width)
	{
		if(c->index / CellSize < CellSize - 1)
		{
			struct MCEvent e;
			e.type = MCSWITCH_EVENT;
			e.key = timeYD;

			e.s = new struct SwitchEvent;			

			e.s->c = &(Cells[c->index + CellSize]);
			e.id = m.id;
			//Cells[c->index + CellSize].points.insert(m.id);
			MCQ.push(e);
		}
	}

	//check crash event
	float minCrashTime = FLT_MAX;
	CrashEvent* bestCrash = NULL;
	while(!MotorHash->empty())
		MotorHash->pop();
	for(set<int>::iterator iter = c->points.begin(); iter != c->points.end(); iter++)
	{
		if(*iter == m.id)
			continue;
		CrashEvent e;
		e.isCrashToWall = false;
		e.startMotor = m.id;
		e.endMotor = *iter;
		e.time = m.getCrashTime(motors[*iter], &e);
		if(e.time > 0)
		{
			float x = m.position.x() + m.speed.x() * e.time;
			float y = m.position.y() + m.speed.y() * e.time;
			if(x >= c->leftup.x() && x <= c->leftup.x() + c->width
				&& y >= c->leftup.y() && y <= c->leftup.y() + c->height)
			{
				
				bestCrash = new CrashEvent;
				//minCrashTime = e.time;
				bestCrash->isCrashToWall = e.isCrashToWall;
				bestCrash->startMotor = e.startMotor;
				bestCrash->endMotor = e.endMotor;
				bestCrash->time = e.time;
				bestCrash->othertime = e.othertime;
				struct MCEvent e;
				e.type = MCCRASH_ENVENT;
				e.key = bestCrash->time;

				e.c = bestCrash;


				//e.s->c = c;
				e.id = m.id;
				MCQ.push(e);
			}
		}
		CrashEvent *otherE = new CrashEvent;
		otherE->time = motors[*iter].getCrashTime(m, otherE);
		if(otherE->time > 0)
		{
			float x = motors[*iter].position.x() + motors[*iter].speed.x() * otherE->time;
			float y = motors[*iter].position.y() + motors[*iter].speed.y() * otherE->time;
			if(x >= c->leftup.x() && x <= c->leftup.x() + c->width
				&& y >= c->leftup.y() && y <= c->leftup.y() + c->height)
			{

				otherE->isCrashToWall = false;
				otherE->startMotor = *iter;
				otherE->endMotor = m.id;
				struct MCEvent e;
				e.type = MCCRASH_ENVENT;
				e.key = otherE->time;

				e.c = otherE;


				e.id = *iter;
				MCQ.push(e);
			}
			else
			{
				delete otherE;
			}
		}
		else
			delete otherE;
	}
	float minWallCrashTime = FLT_MAX;
	CrashEvent *bestWallCrash = NULL;
	for(int i = 0; i < c->walls.size(); i++)
	{
		CrashEvent e;
		e.isCrashToWall = true;
		e.startMotor = m.id;
		e.endMotor = c->walls[i];
		e.time = m.getCrashTime(walls[c->walls[i]], &e);
		if(e.time > 0 && e.time < minWallCrashTime)
		{
			float x = m.position.x() + m.speed.x() * e.time;
			float y = m.position.y() + m.speed.y() * e.time;
			if(x >= c->leftup.x() && x <= c->leftup.x() + c->width
				&& y >= c->leftup.y() && y <= c->leftup.y() + c->height)
			{
				minWallCrashTime = e.time;
				if(bestWallCrash == NULL)
					bestWallCrash = new CrashEvent;
				bestWallCrash->isCrashToWall = e.isCrashToWall;
				bestWallCrash->startMotor = e.startMotor;
				bestWallCrash->endMotor = e.endMotor;
				bestWallCrash->time = e.time;
				bestWallCrash->othertime = e.othertime;
			}
		}
	}
	/*
	if(bestCrash != NULL)
	{
		struct MCEvent e;
		e.type = MCCRASH_ENVENT;
		e.key = bestCrash->time;

		e.c = bestCrash;


		//e.s->c = c;
		e.id = m.id;

		MCQ.push(e);
	}*/
	if(bestWallCrash != NULL)
	{
		struct MCEvent e;
		e.type = MCCRASH_ENVENT;
		e.key = bestWallCrash->time;

		e.c = bestWallCrash;


		//e.s->c = c;
		e.id = m.id;

		MCQ.push(e);
	}
}
void handleCrashEvent(MCEvent e, const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,std::vector<CrashPoint>& points)
{
	if(e.type != MCCRASH_ENVENT)
		return;
	if(isMotorCrashed[e.id])
		return;
	if(!e.c->isCrashToWall && isMotorCrashed[e.c->endMotor] && e.c->othertime > crashTime[e.c->endMotor])
		return;
	crashTime[e.id] = e.key;

	TmpEle te;
	te.index = points.size();
	te.time = e.key;
	motorPoints[e.id].push_back(te);

	isMotorCrashed[e.id] = true;
	/*if(CellIndex[e.id] >= 0)
	{
	set<int>::iterator iter = (Cells[CellIndex[e.id]].points).find(e.id);
	Cells[CellIndex[e.id]].points.erase(iter);
	}*/
	CrashPoint p;
	p.p = motors[e.id].position + (motors[e.id].speed * e.key).toPointF();
	p.time = e.key;
	if(e.c->isCrashToWall)
	{
		p.type = WallVertice;
		p.wallNo = e.c->endMotor;
	}
	else
	{
		TmpEle nte;
		te.index = points.size();
		te.time = e.c->othertime;
		motorPoints[e.c->endMotor].push_back(te);

		p.type = MovingVertice;
		p.wallNo = -1;
	}
	points.push_back(p);
}
void GetMotorcycleGraph(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	/*OUT*/ std::vector<CrashPoint>& points, std::vector<struct MotorLines>& lines)
{
	clearMC(motors, walls, points, lines);
	for(int i = 0; i < motors.size(); i++)
	{
		points.push_back(CrashPoint(motors[i].position, ReflexVertice, -1));

	}

	for(int i = 0; i < motors.size(); i++)
	{
		insertMC(motors[i]);
	}
	while(!MCQ.empty())
	{
		MCEvent e = MCQ.top();
		MCQ.pop();
		switch(e.type)
		{
		case  MCSWITCH_EVENT:
			handleSwitchEvent(e, motors, walls);
			break;
		case MCCRASH_ENVENT:
			handleCrashEvent(e, motors, walls, points);
			break;
		}
	}
	reducePoints(points);
	for(int i = 0; i < motors.size(); i++)
	{
		TmpEle p = motorPoints[i][0];
		for(int j = 1; j < motorPoints[i].size(); j++)
		{
			TmpEle q = motorPoints[i][j];
			MotorLines line;
			if(p.time - q.time > EPSINON || p.time - q.time < EPSINON)
			{
				line.start = PointIndex[p.index];
				line.end = PointIndex[q.index];
				line.id = i;
				lines.push_back(line);
			}
			
			p = q;
		}
		if(!isMotorCrashed[i])
		{
			CrashPoint cp;
			cp.type = InfiniteVertice;
			cp.p = motors[i].position + (motors[i].speed.normalized() * 10000).toPointF();
			cp.wallNo = -1;
			
			points.push_back(cp);
			MotorLines line;
			line.start = PointIndex[p.index];
			line.end = points.size() - 1;
			line.id = i;
			lines.push_back(line);
		}
	}
}