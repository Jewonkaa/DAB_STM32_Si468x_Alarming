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
#include "debug_uart.h"

static char itoa_buffer[80];											//buffer using to send debug informations by UART, it is necessary for itoa function

static alarming_status_t alarming_status;

static dab_management_t dab_management_for_alarming;
static dab_service_t* services_list_for_alarming;

static dab_events_t dab_events_for_alarming;
static dab_announcement_info_t dab_announcement_info_for_alarming;
static dab_announcement_support_info_t dab_announcement_support_info_for_alarming;

static char* dls_label_for_alarming;
static uint8_t station_index_before_alarm = 0;
static uint8_t alarm_mode_1_started = 0;
static uint8_t alarm_mode_2_started = 0;

static uint32_t announcement_service_id = 0;


void Alarming_Init()
{
	Alarming_Off();
}

void Alarming_Manage()
{
	dls_label_for_alarming = get_dls_label();

	switch(alarming_status)
	{
	case alarming_mode_1: //alarming by DLS
		if(!strcmp(dls_label_for_alarming, DLS_ALARM_START_STRING)) //strcmp returns 0 if both strings are the same
		{
			Alarming_Mode_1_Start();
		}
		if(!strcmp(dls_label_for_alarming, DLS_ALARM_STOP_STRING)) //strcmp returns 0 if both strings are the same
		{
			Alarming_Mode_1_Stop();
		}
		break;


	case alarming_mode_2: //alarming by Announcement FIG0/18

		dab_events_for_alarming = Si468x_dab_get_event_status();

		if(dab_events_for_alarming.anno_int)
		{
			dab_announcement_info_for_alarming = Si468x_dab_get_announcement_info();
			if(dab_announcement_info_for_alarming.anno_stat) //announcement just started
			{
				Alarming_Mode_2_Start();
				//HAL_Delay(10000);
			}
			else //announcement just stopped
			{
				Alarming_Mode_2_Stop();
				//HAL_Delay(10000);
			}
		}

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

void Alarming_Mode_2_Start()
{
	if(!alarm_mode_2_started)
	{
		uint8_t alarming_station_index = 0;
		char* tmp_name;
		uint8_t alarming_station_found = 0;

		//get DAB stations related data to be able to use that
		dab_management_for_alarming = get_dab_management();
		services_list_for_alarming = get_dab_service_list();

		send_debug_msg("ALARMING BY FIG0/18 ANNOUNCEMENT STARTED - NEW EVENT!", CRLF_SEND);
		send_debug_msg("ALARM SOURCE SERVICE ID: ", CRLF_NO_SEND);
		announcement_service_id = dab_announcement_info_for_alarming.id_1;
		send_debug_msg(itoa(announcement_service_id, itoa_buffer, 10), CRLF_SEND);
		if(announcement_service_id) //if announcement service ID is valid
		{
			//find alarming station with specific service ID
			for (uint8_t i = 0; i < dab_management_for_alarming.total_services; i++)
			{
				if(services_list_for_alarming[i].service_id == announcement_service_id)
				{
					alarming_station_found = 1;
					alarming_station_index = i;
					break;
				}
			}
		}
		else
		{
			send_debug_msg("ERROR - SOURCE SERVICE ID INVALID!", CRLF_SEND);
		}


		if(alarming_station_found)
		{
			station_index_before_alarm = dab_management_for_alarming.actual_station;
			set_state_change_flag(state_change_started_not_completed); //inform state machine about planned state change
			play_station(alarming_station_index); //switch to special alarming station
			set_playing_state(playing); //set state machine to playing state
			set_system_state(main_screen); //set state machine to main screen
			Display_main_screen_alarm();
			alarm_mode_2_started = 1;
			send_debug_msg("ANNOUNCEMENT ALARM STARTED PROPERLY.", CRLF_SEND);
		}
		else //if specific station not found
		{
			LEDs_Blue_On();
			LEDs_Green_On();
			LEDs_Orange_On();
			LEDs_Red_On();
			send_debug_msg("ERROR! SOURCE SERVICE ID NOT FOUND IN STATIONS LIST!", CRLF_SEND);
		}

	}

}

void Alarming_Mode_2_Stop()
{
	if(alarm_mode_2_started)
	{
		set_state_change_flag(state_change_started_not_completed); //inform state machine about planned state change
		play_station(station_index_before_alarm); //switch to special alarming station
		set_playing_state(playing); //set state machine to playing state
		set_system_state(main_screen); //set state machine to main screen
		alarm_mode_2_started = 0;
		send_debug_msg("ALARMING BY FIG0/18 ANNOUNCEMENT STOPPED!", CRLF_SEND);
	}
}

void Alarming_Mode_3_Handle()
{

}

alarming_status_t Alarming_Get_Status()
{
	return alarming_status;
}
