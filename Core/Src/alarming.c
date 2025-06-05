/*
 * alarming.c
 *
 *  Created on: Jun 3, 2025
 *      Author: Cineq
 */
#include "alarming.h"
#include "leds.h"
#include "state_machine.h"
#include "Si468x.h"
#include <string.h>
#include "display_management.h"

static alarming_status_t alarming_status;

static dab_management_t dab_management_for_alarming;
static dab_service_t* services_list_for_alarming;
static char* dls_label_for_alarming;
static uint8_t station_index_before_alarm;
static uint8_t alarm_mode_1_started = 0;

void Alarming_Init()
{
	Alarming_Off();
}

void Alarming_Manage()
{
	dls_label_for_alarming = get_dls_label();

	switch(alarming_status)
	{
	case alarming_mode_1:
		if(!strcmp(dls_label_for_alarming, DLS_ALARM_START_STRING)) //strcmp returns 0 if both strings are the same
		{
			Alarming_Mode_1_Start();
		}
		if(!strcmp(dls_label_for_alarming, DLS_ALARM_STOP_STRING)) //strcmp returns 0 if both strings are the same
		{
			Alarming_Mode_1_Stop();
		}
		break;


	case alarming_mode_2:
		break;

	case alarming_mode_3:
		break;

	case alarming_off:
		break;

	default:
		break;
	}

}

void Alarming_Mode_1_On()
{
	alarming_status = alarming_mode_1;
	LEDs_Red_On();
	LEDs_Green_Off();
	LEDs_Blue_Off();
	LEDs_Orange_Off();

}

void Alarming_Mode_2_On()
{
	alarming_status = alarming_mode_2;
	LEDs_Red_Off();
	LEDs_Green_Off();
	LEDs_Blue_On();
	LEDs_Orange_Off();

}

void Alarming_Mode_3_On()
{
	alarming_status = alarming_mode_3;
	LEDs_Red_Off();
	LEDs_Green_Off();
	LEDs_Blue_Off();
	LEDs_Orange_On();

}

void Alarming_Off()
{
	alarming_status = alarming_off;
	LEDs_Red_Off();
	LEDs_Green_On();
	LEDs_Blue_Off();
	LEDs_Orange_Off();

}

void Alarming_Mode_1_Start() //alarming mode using DLS - what needs to be done after receiving alarm start info
{
	if(!alarm_mode_1_started)
	{
		uint8_t alarming_station_index = 0;
		char* tmp_name;
		uint8_t alarming_station_found = 0;

		//get DAB stations related data to be able to use that
		dab_management_for_alarming = get_dab_management();
		services_list_for_alarming = get_dab_service_list();

		//Display_main_screen_debug(services_list_for_alarming[3].name);

		//find alarming station with specific name (Alarming)
		if(dab_management_for_alarming.total_services) //check if stations list is not empty
		{
			for(uint8_t i = 0; i < dab_management_for_alarming.total_services; i++)
			{
				tmp_name = services_list_for_alarming[i].name;

				if(!strcmp(tmp_name, DLS_ALARM_STATION_NAME)) //strcmp returns 0 if both strings are the same
				{
					alarming_station_found = 1;
					alarming_station_index = i;
					break;
				}
			}
		}

		if(alarming_station_found)
		{
			station_index_before_alarm = dab_management_for_alarming.actual_station;
			set_state_change_flag(state_change_started_not_completed); //inform state machine about planned state change
			play_station(alarming_station_index); //switch to special alarming station
			set_playing_state(playing); //set state machine to playing state
			set_system_state(main_screen); //set state machine to main screen
			Display_main_screen_alarm();

		}
		else //if specific station not found
		{
			LEDs_Blue_On();
			LEDs_Green_On();
			LEDs_Orange_On();
			LEDs_Red_On();
		}
		alarm_mode_1_started = 1;
	}
}

void Alarming_Mode_1_Stop() //alarming mode using DLS - what needs to be done after receiving alarm stop info
{
	if(alarm_mode_1_started)
	{
		set_state_change_flag(state_change_started_not_completed); //inform state machine about planned state change
		play_station(station_index_before_alarm); //switch to special alarming station
		set_playing_state(playing); //set state machine to playing state
		set_system_state(main_screen); //set state machine to main screen
		alarm_mode_1_started = 0;
	}
}

void Alarming_Mode_2_Handle()
{

}

void Alarming_Mode_3_Handle()
{

}

alarming_status_t Alarming_Get_Status()
{
	return alarming_status;
}
