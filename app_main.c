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
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5

#include "RTE_Components.h"
#include CMSIS_device_header

#include "sensor_drv.h"

#define SENSOR_EVENT_TOUT 5000

#define SENSOR_EVENTS    (SENSOR_EVENT_TEMP_DATA_AVAILABLE  | \
                          SENSOR_EVENT_HUM_DATA_AVAILABLE   | \
                          SENSOR_EVENT_PRESS_DATA_AVAILABLE | \
                          SENSOR_EVENT_ACC_DATA_AVAILABLE   | \
                          SENSOR_EVENT_GYRO_DATA_AVAILABLE  | \
                          SENSOR_EVENT_MAG_DATA_AVAILABLE)

uint32_t Interval[6];
int32_t  Scale[6];

int32_t Scale_Temp;
int32_t Scale_Acc;

osThreadId_t Th_Read;

void sensor_init   (void);
void sensor_deinit (void);

void Sensor_Event (uint32_t event) {
  /* Send event(s) to the processing thread */
  osThreadFlagsSet (Th_Read, event);
}

void read_sensors (void *arg) {
  uint32_t event;
  uint32_t ts;
  float fTemp;
  float fAxes[3];

  while (1U) {
    /* Wait until Sensor callback wake-up */
    event = osThreadFlagsWait (SENSOR_EVENTS, osFlagsWaitAny, SENSOR_EVENT_TOUT);

    if ((event & osFlagsError) == 0U) {
      /* Create a timestamp */
      ts = osKernelGetTickCount();

      if (event & SENSOR_EVENT_TEMP_DATA_AVAILABLE) {
        Sensor_EnvReadData (SENSOR_TYPE_TEMP, &fTemp);

        printf ("(%d ms) Temperature: %.1f\n", ts, fTemp);
      }

      if (event & SENSOR_EVENT_ACC_DATA_AVAILABLE) {
        Sensor_MotionReadData (SENSOR_TYPE_ACC, &fAxes[0], &fAxes[1], &fAxes[2]);

        printf ("(%d ms) Acceleration: %.5f, %.5f, %.5f\n", ts, fAxes[0], fAxes[1], fAxes[2]);
      }
    }
    else {
      /* SENSOR_EVENT_TOUT timeout expired or error */
      printf ("Sensor sampling stopped.\n");

      /* De-init sensor interface */
      sensor_deinit();

      /* Terminate this thread */
      osThreadTerminate(Th_Read);
    }
  }
}

void sensor_init (void) {
  uint32_t interval;
  int32_t scale;

  Sensor_Initialize (Sensor_Event);

  Sensor_QueryInterval (SENSOR_TYPE_TEMP, &Interval[SENSOR_TYPE_TEMP], 1U);
  Sensor_QueryInterval (SENSOR_TYPE_ACC,  &Interval[SENSOR_TYPE_ACC],  1U);

  Sensor_Enable (SENSOR_TYPE_TEMP);
  Sensor_Enable (SENSOR_TYPE_ACC);

  Sensor_QueryScale    (SENSOR_TYPE_TEMP, &Scale[SENSOR_TYPE_TEMP], 1U);
  Sensor_QueryScale    (SENSOR_TYPE_ACC,  &Scale[SENSOR_TYPE_ACC],  1U);

  //Sensor_SetScale (SENSOR_TYPE_TEMP, Scale[SENSOR_TYPE_TEMP]);
  //Sensor_SetScale (SENSOR_TYPE_ACC,  Scale[SENSOR_TYPE_ACC]);

  scale    = Sensor_GetScale   (SENSOR_TYPE_TEMP);
  interval = Sensor_GetInterval(SENSOR_TYPE_TEMP);
  printf ("Temperature: scale=%i, interval=%d\n", scale, interval);

  scale    = Sensor_GetScale   (SENSOR_TYPE_ACC);
  interval = Sensor_GetInterval(SENSOR_TYPE_ACC);
  printf ("Acceleration: scale=%i, interval=%d\n", scale, interval);
  printf ("\n\n");
}

void sensor_deinit (void) {

  Sensor_Disable (SENSOR_TYPE_TEMP);
  Sensor_Disable (SENSOR_TYPE_ACC);

  Sensor_Uninitialize();
}

/*---------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
static void app_main (void *argument) {
  osThreadAttr_t attr;

  /* Initialize sensor interface */
  sensor_init();

  attr.stack_size = 2048U;

  Th_Read = osThreadNew (read_sensors, NULL, &attr);
  if (Th_Read == NULL) {
    /* This should succeed */
    __BKPT(0);
  }

  osThreadFlagsWait (0x1, osFlagsWaitAll, osWaitForever);

  sensor_deinit();

  osThreadExit();
}

/*---------------------------------------------------------------------------
 * Application initialization
 *---------------------------------------------------------------------------*/
void app_initialize (void) {
  osThreadNew(app_main, NULL, NULL);
}
