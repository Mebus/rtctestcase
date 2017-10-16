#ifndef RTC5_H
#define RTC5_H


#include "rtc.h"


namespace sepwind
{

class Rtc5 : public Rtc
{

public:
	virtual bool	__stdcall	initialize(double kfactor, char* ctbFileName);	/// bit/mm , ctb or ct5(rtc5)
	virtual bool	__stdcall	listBegin();	/// opening internal buffers
	virtual bool	__stdcall	listTiming(double frequency, double pulseWidth);	/// hz, usec
	virtual bool	__stdcall	listDelay(double on, double off, double jump, double mark, double polygon); //usec
	virtual bool	__stdcall	listSpeed(double jump, double mark); /// mm /sec
	virtual bool	__stdcall	listJump(double x, double y); /// mm
	virtual bool	__stdcall	listMark(double x, double y);	/// mm
	virtual bool	__stdcall	listArc(double cx, double cy, double sweepAngle); /// mm, mm, positive angle is ccw, 
	virtual bool	__stdcall	listOn(double msec);
	virtual bool	__stdcall	listOff();
	virtual bool	__stdcall	listEnd();	/// closing internal buffers
	virtual bool	__stdcall	listExecute(bool wait);	/// blocking until finish

private:
	double _kfactor;

public:
	Rtc5();
	~Rtc5();
};

}//namespace

#endif