﻿#include "rtc5.h"
#include "rtc5expl.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

namespace sepwind
{

using namespace rtc5;


Rtc5::Rtc5()
{
	_kfactor = 0.0;
}

Rtc5::~Rtc5()
{

}

bool	__stdcall	Rtc5::initialize(double kfactor, char* lpszCtbFileName)
{
	int error = RTC5open();
	if (0 != error)
	{
		fprintf(stderr, "fail to initialize the Rtc5 library. error code = %d", error);
		return false;
	}

	init_rtc5_dll();

	error = get_last_error();
	if (0 != error)
	{
		// 에러 리셋
		reset_error(error);
	}

	// program file load
	error = load_program_file(NULL);
	if (0 != error)
	{
		fprintf(stderr, "fail to load the Rtc5 program file :  error code = %d", error);
		return false;
	}

	// rtc5는 laser 및 gate신호 레벨을 설정할수가 있다
	// active high 로 설정
	int siglevel = (0x01 << 3) | (0x01 << 4);
	set_laser_control(siglevel);

	_kfactor = kfactor;


	error = load_correction_file(
		lpszCtbFileName,		// ctb
		1,	// table no (1 ~ 4)
		2	// 2d
	);
	if (0 != error)
	{
		fprintf(stderr, "fail to load the correction file :  error code = %d", error);
		return false;
	}

	select_cor_table(1, 0);	//1 correction file at primary head

	set_standby(0, 0);


	set_laser_mode(0);	//co2 mode

	short ctrlMode = \
		0x01 << 0 +	//ext start enabled
		0x01 << 1; // ext stop enabled
	set_control_mode(ctrlMode);

	return true;
}

bool __stdcall	Rtc5::listBegin()
{
	set_start_list(1);//list 1
	return true;
}

bool __stdcall	Rtc5::listTiming(double frequency, double pulsewidth)
{
	double period = 1.0f / frequency * (double)1.0e6;	//usec
	double halfperiod = period / 2.0f;

	set_laser_timing(
		halfperiod * 64,	//half period (us)
		pulsewidth * 64,
		0,
		0);	// timebase 1/64 usec	
	return true;
}

bool __stdcall	Rtc5::listDelay(double on, double off, double jump, double mark, double polygon)
{
	set_scanner_delays(
		(jump / 10.0f),
		(mark / 10.0f),
		(polygon / 10.0f)
	);
	// unit: 10 usec

	return true;
}

bool __stdcall	Rtc5::listSpeed(double jump, double mark)
{
	double jump_bitpermsec = (double)(jump / 1.0e3 * _kfactor);
	double mark_bitpermsec = (double)(mark / 1.0e3 * _kfactor);

	set_jump_speed(jump_bitpermsec);
	set_mark_speed(mark_bitpermsec);
	return true;
}

bool __stdcall	Rtc5::listJump(double x, double y)
{
	int xbits = x * _kfactor;
	int ybits = y * _kfactor;
	jump_abs(xbits, ybits);
	return true;
}

bool __stdcall	Rtc5::listMark(double x, double y)
{
	int xbits = x * _kfactor;
	int ybits = y * _kfactor;
	mark_abs(xbits, ybits);
	return true;
}

bool __stdcall	Rtc5::listArc(double cx, double cy, double sweepAngle)
{
	int cxbits = cx * _kfactor;
	int cybits = cy * _kfactor;
	arc_abs(cxbits, cybits, -sweepAngle);
	return true;
}

bool	__stdcall Rtc5::listOn(double msec)
{
	double remind_msec = msec;
	while (remind_msec > 1000)
	{
		laser_on_list(1000 * 1000 / 10);
		remind_msec -= 1000;
	}

	laser_on_list(remind_msec * 1000 / 10);
	return TRUE;

}

bool	__stdcall	Rtc5::listOff()
{
	laser_signal_off_list();
	return true;
}


bool __stdcall	Rtc5::listEnd()
{
	set_end_of_list();
	return TRUE;
}

bool __stdcall Rtc5::listExecute(bool wait)
{
	execute_list(1);	//list 1

	if (wait)
	{
		unsigned int busy(0), position(0);
		do {
			::Sleep(50);
			get_status(&busy, &position);
		} while (busy);
	}
	return true;
}


}//namespace