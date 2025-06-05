/*
 * alarming.h
 *
 *  Created on: Jun 3, 2025
 *      Author: Cineq
 */

#ifndef INC_ALARMING_H_
#define INC_ALARMING_H_

#include "stm32f4xx.h"

#define DLS_ALARM_START_STRING "ALARM START"
#define DLS_ALARM_STOP_STRING "ALARM STOP"
#define DLS_ALARM_STATION_NAME "Alarming        "

typedef uint8_t alarming_status_t;

enum alarming_state{
alarming_off,
alarming_mode_1,
alarming_mode_2,
alarming_mode_3
};

void Alarming_Init();
void Alarming_Manage();

void Alarming_Mode_1_On();
void Alarming_Mode_2_On();
void Alarming_Mode_3_On();
void Alarming_Off();

void Alarming_Mode_1_Start();
void Alarming_Mode_1_Stop();

void Alarming_Mode_2_Handle();
void Alarming_Mode_3_Handle();

alarming_status_t Alarming_Get_Status();

#endif /* INC_ALARMING_H_ */
