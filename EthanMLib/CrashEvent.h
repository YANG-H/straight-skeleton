#pragma once
#include "Point2D.h"
#include "MCCell.h"
struct CrashEvent
{
public:
	int startMotor;
	int endMotor;
	double time;
	double othertime;
	bool isCrashToWall;
	friend bool operator<(CrashEvent a, CrashEvent b)
	{
		return a.time > b.time;
	}
};
struct SwitchEvent
{
	MCCell* c;
};
enum MCEventType{MCSWITCH_EVENT, MCCRASH_ENVENT};
struct MCEvent
{
	enum MCEventType type;
	struct CrashEvent *c;
	struct SwitchEvent *s;
	int id;
	double key;
	friend bool operator<(MCEvent a, MCEvent b)
	{
		return a.key > b.key;
	}
};
