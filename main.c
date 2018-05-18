/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define MOVING_AVERAGE_DATAPOINTS 10
#define SMOOTHED_ACCELERATION_SAVED_VALUES 10

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// Standard C Included Files
#include <string.h>
#include <math.h>
#include <stdio.h>

// SDK Included Files
#include "board.h"
#include "fsl_i2c_master_driver.h"

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

enum _subaddress_index_e
{
    Subaddress_Index_1 = 0x01,
    Subaddress_Index_2 = 0x02,
    Subaddress_Index_3 = 0x03,
    Subaddress_Index_4 = 0x04,
    Subaddress_Index_5 = 0x05,
    Subaddress_Index_6 = 0x06,
    Invalid_Subaddress_Index = 0x07,
    Max_Subaddress_Index
};

uint8_t i;
uint8_t cmdBuff[1] = {0xFF};
uint8_t sendBuff[1] = {0xFF};
uint8_t receiveBuff[2] = {0xFF, 0xFF};
i2c_master_state_t master;
i2c_status_t returnValue;
i2c_device_t slave =
{
    .address = 0x1D,
    .baudRate_kbps = 100
};

uint8_t number_of_steps = 0;
int8_t *accelerometer_data;
uint8_t accel_magnitude_array[MOVING_AVERAGE_DATAPOINTS] = {0};
uint8_t accel_magnitude_ptr = 0;
uint8_t accel_magnitude_smoothed_array[SMOOTHED_ACCELERATION_SAVED_VALUES] = {0};
uint8_t accel_magnitude_smoothed_ptr = 0;

static void LED_toggle_master(void){
    GPIO_DRV_TogglePinOutput(kGpioLED1);
}

void performSetup(){
	// Set up.
    hardware_init();
    dbg_uart_init();
    configure_i2c_pins(BOARD_I2C_COMM_INSTANCE);
    OSA_Init();
    GPIO_DRV_Init(0, ledPins);
    I2C_DRV_MasterInit(BOARD_I2C_COMM_INSTANCE, &master);

	// Confirm alive.
    printf("\r\n====== START OF TEST ======\r\n\r\n");

	// Flash lights.
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
    OSA_TimeDelay(500);
    LED_toggle_master();
}

void pollTheRegisters(){
	// Steps for repeated single-byte read.
    // 1. Write transaction beginning with start condition, slave address, and pointer address.
    // 2. Read transaction beginning with start condition, followed by slave address, and read 1 byte payload.
	for (i=Subaddress_Index_1; i<Invalid_Subaddress_Index; i++){
		cmdBuff[0] = i;
		returnValue = I2C_DRV_MasterReceiveDataBlocking(
									BOARD_I2C_COMM_INSTANCE, &slave,
									cmdBuff, 1, receiveBuff, sizeof(receiveBuff), 500);
        if (returnValue == kStatus_I2C_Success){
			printf("0x%02X,0x%02X\r\n", cmdBuff[0], receiveBuff[0]);
        }
		else{
            printf("0x%02x,ERROR\r\n", cmdBuff[0]);
        }
    }
}

int8_t checkForStep(){
	return 0;
}

void addStep(){
	number_of_steps++;
	printf("Step detected. Step count: %d.\r\n", number_of_steps);
}

void testLoop(){
	pollTheRegisters();
}

void loop(){
	// Code to get the acceleration from the accelerometer.
	int8_t accel_x;
	int8_t accel_y;
	int8_t accel_z;
	accel_x = accelerometer_data[1];
	accel_y = accelerometer_data[2];
	accel_z = accelerometer_data[3];

	// Code to manipulate acceleration parameters into suitable state for processing.
	uint16_t accel_magnitude_squared;
	uint8_t accel_magnitude_smoothed;
	uint8_t tempSum;
	uint8_t i;
	int8_t is_step;

	// Store into data array used for moving average calculation.
	accel_magnitude_squared = (uint16_t)((uint16_t)accel_x*(uint16_t)accel_x + (uint16_t)accel_y*(uint16_t)accel_y + (uint16_t)accel_z*(uint16_t)accel_z);
	accel_magnitude_array[accel_magnitude_ptr] = (uint8_t)sqrt(accel_magnitude_squared);
	accel_magnitude_ptr++;
	if (accel_magnitude_ptr == MOVING_AVERAGE_DATAPOINTS){
		accel_magnitude_ptr = 0;
	}
	
	// Calculate moving average and store into moving average data array.
	tempSum = 0;
	for (i = 0; i < MOVING_AVERAGE_DATAPOINTS; i++){
		tempSum = tempSum + accel_magnitude_array[i];
	}
	accel_magnitude_smoothed = tempSum / (uint8_t)MOVING_AVERAGE_DATAPOINTS;
	accel_magnitude_smoothed_array[accel_magnitude_smoothed_ptr] = accel_magnitude_smoothed;
	accel_magnitude_smoothed_ptr++;
	if (accel_magnitude_smoothed_ptr == SMOOTHED_ACCELERATION_SAVED_VALUES){
		accel_magnitude_smoothed_ptr = 0;
	}

	// Output moving average value. (Test purposes).
	printf("Test Point 1. Moving average value: %d.\r\n", accel_magnitude_smoothed);

	// Check for step.
	is_step = checkForStep();
	if (is_step == 1){
		addStep();
	}
}

int main(void){
    performSetup();
    while (1){
        testLoop();
    }
}