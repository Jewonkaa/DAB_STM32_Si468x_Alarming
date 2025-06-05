/*
 * state_machine.h
 *
 *  Created on: Sep 15, 2022
 *      Author: Cineq
 */

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

#include "stm32f4xx.h"

typedef uint8_t system_state_t, playing_state_t, state_change_flag_t;

enum system_state{
main_screen,
services_list_screen,
scanning,
signal_info,
settings,
alarming
};

enum play_state{
	not_playing,
	playing
};

enum state_change_info{
	state_change_started_not_completed,
	state_change_completed
};

void state_machine();

uint8_t get_scan_cancel_flag();

system_state_t get_system_state();
void set_system_state(system_state_t system_state_to_set);

playing_state_t get_playing_state();
void set_playing_state(playing_state_t playing_state_to_set);

state_change_flag_t get_state_change_flag();
void set_state_change_flag(state_change_flag_t state_change_flag_to_set);



#endif /* INC_STATE_MACHINE_H_ */
