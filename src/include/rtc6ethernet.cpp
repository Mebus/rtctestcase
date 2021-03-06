﻿#include "rtc6ethernet.h"
#include "rtc6expl.h"
#include <stdio.h>

namespace sepwind
{

using namespace rtc6;


Rtc6Ethernet::Rtc6Ethernet(const char* ipaddress, double xCntPerMm, double yCntPerMm)
{
	_kfactor = 0.0;
	_xCntPerMm = xCntPerMm;
	_yCntPerMm = yCntPerMm;
	strcpy(_ipaddress, ipaddress);
}

Rtc6Ethernet::~Rtc6Ethernet()
{

}

bool	__stdcall	Rtc6Ethernet::initialize(double kfactor, char* ct5FileName)
{
	int error = RTC6open();
	if (0 != error)
	{
		fprintf(stderr, "fail to initialize the rtc6 library. error code = %d", error);
		return false;
	}
	
	INT result = eth_search_cards(	eth_convert_string_to_ip(_ipaddress), eth_convert_string_to_ip("255.255.255.0")	);
	switch (result)
	{
	case -2:		
		fprintf(stderr, "the entry cannot be made. at this index, already an rtc6 ethernet board is entered : %s", _ipaddress);
		return FALSE;
		break;
	case -1:		
		fprintf(stderr, "the entry cannot be made. at this index, already an rtc6 pci express board is entered : %s", _ipaddress);
		return FALSE;
		break;
	case 0:		
		fprintf(stderr, "the entry cannot be made. search no or card no is invalid : %s", _ipaddress);
		return FALSE;
		break;
	}
	
	result = eth_assign_card_ip(eth_convert_string_to_ip(_ipaddress), 1);
	switch (result)
	{
	case -2:		
		fprintf(stderr, "the entry cannot be made. at this index, already an rtc6 ethernet board is entered : %s", _ipaddress);
		return FALSE;
		break;
	case -1:		
		fprintf(stderr, "the entry cannot be made. at this index, already an rtc6 pci express board is entered : %s", _ipaddress);
		return FALSE;
		break;
	case 0:		
		fprintf(stderr, "the entry cannot be made. search no or card no is invalid : %s", _ipaddress);
		return FALSE;
		break;
	}
			
	init_rtc6_dll();
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
		fprintf(stderr, "fail to load the rtc6 program file :  error code = %d", error);
		return false;
	}
	UINT32 rtcVersion = get_rtc_version();
	fprintf(stdout, "card count : %d. dll, hex, firmware version : %d, %d, %d\r\n", \
		rtc6_count_cards(), get_dll_version(), get_hex_version(), rtcVersion & 0x0F);

	if (rtcVersion & 0x100)
		fprintf(stdout, "processing on the fly option enabled\r\n");

	if (rtcVersion & 0x200)
		fprintf(stdout, ("2nd scanhead option enabled\r\n"));

	if (rtcVersion & 0x400)
	{
		fprintf(stdout, ("3d option (varioscan) enabled\r\n"));
		_3d = TRUE;
	}
	else
		_3d = FALSE;

	// Rtc6는 laser 및 gate신호 레벨을 설정할수가 있다
	// active high 로 설정
	int sigLevel = (0x01 << 3) | (0x01 << 4);
	set_laser_control(sigLevel);

	_kfactor = kfactor;

	error = load_correction_file(
		ct5FileName,		// ctb
		1,	// table no (1 ~ 4)
		2	// 2d
	);

	if (0 != error)
	{
		fprintf(stderr, "fail to load the correction file :  error code = %d", error);
		return false;
	}

	if (_3d)
		select_cor_table(1, 1);	//1 correction file at primary / secondary head	
	else
		select_cor_table(1, 0);	//1 correction file at primary head

	set_standby(0, 0);

	set_laser_mode(0);	//co2 mode

	short ctrlMode = \
		0x01 << 0 +	//ext start enabled
		0x01 << 1; // ext stop enabled
	set_control_mode(ctrlMode);

	config_list(4000, 4000);

	return true;
}



}//namespace