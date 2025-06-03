/*
 * alarming.c
 *
 *  Created on: Jun 3, 2025
 *      Author: Cineq
 */
#include "alarming.h"
#include "leds.h"

static alarming_status_t alarming_status;

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

alarming_status_t Alarming_Get_Status()
{
	return alarming_status;
}
