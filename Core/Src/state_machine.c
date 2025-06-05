/*
 * state_machine.c
 *
 *  Created on: Sep 15, 2022
 *      Author: Cineq
 */

#include "state_machine.h"
#include "debug_uart.h"
#include "display_management.h"
#include "Si468x.h"
#include "eeprom.h"
#include "touch.h"
#include "alarming.h"

system_state_t system_state = main_screen;
playing_state_t playing_state = playing;

state_change_flag_t state_change_flag = state_change_started_not_completed;

int srv_list_start_index = 0;

touch_coordinates_t touch_coordinates;

dab_digrad_status_t dab_digrad_status_to_display;
dab_management_t dab_management_to_display;
dab_service_t* services_list_to_display;
dab_ensemble_t* ensembles_list_to_display;
dab_audio_info_t dab_audio_info_to_display;

char* dls_label_to_display;

uint8_t scan_cancel_flag = 0;

uint8_t scan_complete_flag = 0;

uint16_t audio_info_timeout;

//alarming_status_t alarming_status_sm; //alarming status to be used by state machine

void state_machine()
{
	if(state_change_flag == state_change_completed)
	{
		touch_coordinates = Touch_read();
	}
	else
	{
		touch_coordinates.x = 0;
		touch_coordinates.y = 0;
	}

	dab_management_to_display = get_dab_management();
	dls_label_to_display = get_dls_label();
	Si468x_dab_get_time();

	//manage alarming functionality
	Alarming_Manage();

	switch(system_state)
	{
	case main_screen:

		if(state_change_flag == state_change_started_not_completed)
		{
			dab_management_to_display = get_dab_management();
			if(!dab_management_to_display.total_services)
			{
				playing_state = not_playing;
			}

			else
			{
				services_list_to_display = get_dab_service_list();
				ensembles_list_to_display = get_dab_ensemble_list();

				audio_info_timeout = 50;
				while(!dab_audio_info_to_display.audio_sample_rate)
				{
					dab_audio_info_to_display = Si468x_dab_get_audio_info();
					audio_info_timeout--;
					if(!audio_info_timeout)
					{
						break;
					}
				}
			}

			Display_main_screen_background();
			Display_main_screen_data(services_list_to_display, ensembles_list_to_display, dab_management_to_display, dab_audio_info_to_display);
			state_change_flag = state_change_completed;
		}

		else
		{
			if(playing_state == playing)
			{
				Si468x_dab_get_digital_service_data();
				if (Alarming_Get_Status() == alarming_mode_1)
				{
					if(strcmp(dls_label_to_display, DLS_ALARM_START_STRING))
					{
						Display_main_screen_dls(dls_label_to_display);
					}
				}
				else
				{
					Display_main_screen_dls(dls_label_to_display);
				}
			}
			else
			{
				Display_main_screen_empty();
			}
		}

		//left button handling (alarming)
		if(touch_coordinates.x >  5 && touch_coordinates.x < 157 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = alarming;
		}

		//right button handling (service list)
		if(touch_coordinates.x >  163 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = services_list_screen;
		}

		break;

	case services_list_screen:
		if(state_change_flag == state_change_started_not_completed)
		{
			services_list_to_display = get_dab_service_list();
//			srv_list_start_index = 0;
			Display_stations_list_background();
			Display_stations_list_data(srv_list_start_index, dab_management_to_display, services_list_to_display);
			state_change_flag = state_change_completed;
		}

		//left button handling (main)
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = main_screen;
		}

		//right button handling (signal info)
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = signal_info;
		}

		//up button handling
		if(touch_coordinates.x > 290 && touch_coordinates.x < 315 && touch_coordinates.y > 20 && touch_coordinates.y < 102)
		{
			if(dab_management_to_display.total_services)
			{
				srv_list_start_index--;
				if(srv_list_start_index > dab_management_to_display.total_services || srv_list_start_index < 0)
				{
					srv_list_start_index = dab_management_to_display.total_services - 1;
				}
				Display_stations_list_data(srv_list_start_index, dab_management_to_display, services_list_to_display);
			}
		}

		//down button handling
		if(touch_coordinates.x > 290 && touch_coordinates.x < 315 && touch_coordinates.y > 108 && touch_coordinates.y < 190)
		{
			if(dab_management_to_display.total_services)
			{
				srv_list_start_index++;
				if(srv_list_start_index >= dab_management_to_display.total_services)
				{
					srv_list_start_index = 0;
				}
				Display_stations_list_data(srv_list_start_index, dab_management_to_display, services_list_to_display);
			}
		}

		//Service 1 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 285 && touch_coordinates.y > 20 && touch_coordinates.y < 50)
		{
			if(dab_management_to_display.total_services)
			{
				state_change_flag = state_change_started_not_completed;
				play_station(srv_list_start_index);
				playing_state = playing;
				system_state = main_screen;
			}
		}

		//Service 2 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 285 && touch_coordinates.y > 55 && touch_coordinates.y < 85)
		{
			if(dab_management_to_display.total_services)
			{
				state_change_flag = state_change_started_not_completed;

				if(srv_list_start_index + 1 >= dab_management_to_display.total_services)
				{
					play_station(srv_list_start_index + 1 - dab_management_to_display.total_services);
				}
				else
				{
					play_station(srv_list_start_index + 1);
				}

				playing_state = playing;
				system_state = main_screen;
			}
		}

		//Service 3 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 285 && touch_coordinates.y > 90 && touch_coordinates.y < 120)
		{
			if(dab_management_to_display.total_services)
			{
				state_change_flag = state_change_started_not_completed;

				if(srv_list_start_index + 2 >= dab_management_to_display.total_services)
				{
					play_station(srv_list_start_index + 2 - dab_management_to_display.total_services);
				}
				else
				{
					play_station(srv_list_start_index + 2);
				}

				playing_state = playing;
				system_state = main_screen;
			}
		}

		//Service 4 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 285 && touch_coordinates.y > 125 && touch_coordinates.y < 155)
		{
			if(dab_management_to_display.total_services)
			{
				state_change_flag = state_change_started_not_completed;

				if(srv_list_start_index + 3 >= dab_management_to_display.total_services)
				{
					play_station(srv_list_start_index + 3 - dab_management_to_display.total_services);
				}
				else
				{
					play_station(srv_list_start_index + 3);
				}

				playing_state = playing;
				system_state = main_screen;
			}
		}

		//Service 5 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 285 && touch_coordinates.y > 160 && touch_coordinates.y < 190)
		{
			if(dab_management_to_display.total_services)
			{
				state_change_flag = state_change_started_not_completed;

				if(srv_list_start_index + 4 >= dab_management_to_display.total_services)
				{
					play_station(srv_list_start_index + 4 - dab_management_to_display.total_services);
				}
				else
				{
					play_station(srv_list_start_index + 4);
				}

				playing_state = playing;
				system_state = main_screen;
			}
		}

		break;

	case signal_info:
		if(state_change_flag == state_change_started_not_completed)
		{
			Display_dab_digrad_status_background();
			state_change_flag = state_change_completed;
		}
		dab_digrad_status_to_display = Si468x_dab_digrad_status();
		Display_dab_digrad_status_data(dab_digrad_status_to_display);

		//left button handling (services list)
		if(touch_coordinates.x >  5 && touch_coordinates.x < 157 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = services_list_screen;
		}

		//right button handling (settings)
		if(touch_coordinates.x >  163 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = settings;
		}
		break;

	case scanning:
		if(state_change_flag == state_change_started_not_completed)
		{
			playing_state = not_playing;
			Display_scanning_screen_background();
			state_change_flag = state_change_completed;
			scan_complete_flag = Si468x_dab_full_scan();
			if(scan_complete_flag)
			{
//				dab_management_to_display = get_dab_management();
//				Display_scanning_screen_data(dab_digrad_status_to_display, dab_management_to_display);
				Display_scanning_screen_complete();
				scan_complete_flag = 0;
			}
		}
//		dab_digrad_status_to_display = Si468x_dab_digrad_status();
//		Display_scanning_screen_data(dab_digrad_status_to_display);

		//cancel button handling
		if(touch_coordinates.x >  5 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			scan_cancel_flag = 1;
			state_change_flag = state_change_started_not_completed;
			system_state = settings;
		}

		break;

	case settings:
		if(state_change_flag == state_change_started_not_completed)
		{
			Display_settings_screen_background();
			scan_cancel_flag = 0;
			state_change_flag = state_change_completed;
		}

		Display_settings_screen_data(dab_management_to_display);

		//vol+ button handling
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 60 && touch_coordinates.y < 100)
		{
			dab_management_to_display.audio_volume += 2;
			if(dab_management_to_display.audio_volume > 63)
			{
				dab_management_to_display.audio_volume = 63;
			}
			Si468x_set_audio_volume(dab_management_to_display.audio_volume);
		}

		//vol- button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 60 && touch_coordinates.y < 100)
		{
			dab_management_to_display.audio_volume -= 2;
			if((dab_management_to_display.audio_volume < 0) || (dab_management_to_display.audio_volume > 63))
			{
				dab_management_to_display.audio_volume = 0;
			}
			Si468x_set_audio_volume(dab_management_to_display.audio_volume);
		}

		//backlight+ button handling
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 105 && touch_coordinates.y < 145)
		{

		}

		//backlight- button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 105 && touch_coordinates.y < 145)
		{

		}

		//scanning button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 150 && touch_coordinates.y < 190)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = scanning;
		}

		//touch calibration button handling
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 150 && touch_coordinates.y < 190)
		{

		}

		//left button handling (signal info)
		if(touch_coordinates.x >  5 && touch_coordinates.x < 157 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = signal_info;
		}

		//right button handling (alarming)
		if(touch_coordinates.x >  163 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = alarming;
		}


		break;

	case alarming:
		if(state_change_flag == state_change_started_not_completed)
		{
			Display_alarming_screen_background();
			//scan_cancel_flag = 0;
			state_change_flag = state_change_completed;
		}

		//Display_settings_screen_data(dab_management_to_display);

		//Alarming by DLS button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 43 && touch_coordinates.y < 114)
		{
			Alarming_Mode_1_On();
		}

		//Alarming by ...1 button handling
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 43 && touch_coordinates.y < 114)
		{
			Alarming_Mode_2_On();
		}

		//Alarming by ...2 button handling
		if(touch_coordinates.x > 5 && touch_coordinates.x < 157 && touch_coordinates.y > 119 && touch_coordinates.y < 190)
		{
			Alarming_Mode_3_On();
		}

		//No alarming - normal mode button handling
		if(touch_coordinates.x > 163 && touch_coordinates.x < 315 && touch_coordinates.y > 119 && touch_coordinates.y < 190)
		{
			Alarming_Off();
		}

		//left button handling (settings)
		if(touch_coordinates.x >  5 && touch_coordinates.x < 157 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = settings;
		}

		//right button handling (main screen)
		if(touch_coordinates.x >  163 && touch_coordinates.x < 315 && touch_coordinates.y > 195 && touch_coordinates.y < 235)
		{
			state_change_flag = state_change_started_not_completed;
			system_state = main_screen;
		}


		break;

	default:
		break;
	}
}

uint8_t get_scan_cancel_flag()
{
	return scan_cancel_flag;
}

system_state_t get_system_state()
{
	return system_state;
}

void set_system_state(system_state_t system_state_to_set)
{
	system_state = system_state_to_set;
}

playing_state_t get_playing_state()
{
	return playing_state;
}

void set_playing_state(playing_state_t playing_state_to_set)
{
	playing_state = playing_state_to_set;
}

state_change_flag_t get_state_change_flag()
{
	return state_change_flag;
}

void set_state_change_flag(state_change_flag_t state_change_flag_to_set)
{
	state_change_flag = state_change_flag_to_set;
}
