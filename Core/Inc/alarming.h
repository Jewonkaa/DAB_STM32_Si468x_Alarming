/*
 * alarming.h
 *
 *  Created on: Jun 3, 2025
 *      Author: Cineq
 */

#ifndef INC_ALARMING_H_
#define INC_ALARMING_H_

#include "stm32f4xx.h"

typedef uint8_t alarming_status_t;

enum alarming_state{
alarming_mode_1,
alarming_mode_2,
alarming_mode_3,
alarming_off
};


void Alarming_Mode_1_On();
void Alarming_Mode_2_On();
void Alarming_Mode_3_On();
void Alarming_Off();

alarming_status_t Alarming_Get_Status();

#endif /* INC_ALARMING_H_ */
