/* -------------------------------------------------------------------------- 
 * Copyright (c) 2022 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    app_main.c
 *      Purpose: Example program
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5

#include "RTE_Components.h"
#include CMSIS_device_header

#include "sensor_drv.h"

extern int test_main(void);

uint32_t Evt;
void Sensor_Event (uint32_t event) {

  Evt = event;
}

int32_t Scale_In_Temp[4];
int32_t Scale_In_Acc[4];
int32_t Scale_Temp;
int32_t Scale_Acc;

uint32_t ODR_In_Temp[4];
uint32_t ODR_In_Acc[4];
uint32_t ODR_Temp;
uint32_t ODR_Acc;

int32_t Temp;
int32_t xyz[3];

void test_sensor (void) {

  Sensor_Initialize (Sensor_Event);

  Sensor_Enable (SENSOR_TYPE_TEMP);
  Sensor_Enable (SENSOR_TYPE_ACC);

  Sensor_QueryScale    (SENSOR_TYPE_TEMP, Scale_In_Temp, sizeof(Scale_In_Temp)/sizeof(Scale_In_Temp[0]));
  Sensor_SetScale      (SENSOR_TYPE_TEMP, Scale_In_Temp[0]);
  Sensor_GetScale      (SENSOR_TYPE_TEMP, &Scale_Temp);

  Sensor_QueryDataRate (SENSOR_TYPE_TEMP, ODR_In_Temp, sizeof(ODR_In_Temp)/sizeof(ODR_In_Temp[0]));
  Sensor_SetDataRate   (SENSOR_TYPE_TEMP, ODR_In_Temp[0]);
  Sensor_GetDataRate   (SENSOR_TYPE_TEMP, &ODR_Temp);

  Sensor_EnvReadData   (SENSOR_TYPE_TEMP, &Temp);

  Sensor_QueryScale    (SENSOR_TYPE_ACC, Scale_In_Acc, sizeof(Scale_In_Acc)/sizeof(Scale_In_Acc[0]));
  Sensor_SetScale      (SENSOR_TYPE_ACC, Scale_In_Acc[0]);
  Sensor_GetScale      (SENSOR_TYPE_ACC, &Scale_Acc);

  Sensor_QueryDataRate (SENSOR_TYPE_ACC, ODR_In_Acc, sizeof(ODR_In_Acc)/sizeof(ODR_In_Acc[0]));
  Sensor_SetDataRate   (SENSOR_TYPE_ACC, ODR_In_Acc[0]);
  Sensor_GetDataRate   (SENSOR_TYPE_ACC, &ODR_Acc);

  Sensor_MotionReadData (SENSOR_TYPE_ACC, &xyz[0], &xyz[1], &xyz[2]);

  Sensor_Disable (SENSOR_TYPE_TEMP);
  Sensor_Disable (SENSOR_TYPE_ACC);

  Sensor_Uninitialize ();

}

/*---------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
static void app_main (void *argument) {
  stdio_init();

  printf ("Test\n");

  osThreadExit();
}

/*---------------------------------------------------------------------------
 * Application initialization
 *---------------------------------------------------------------------------*/
void app_initialize (void) {
  osThreadNew(app_main, NULL, NULL);
}
