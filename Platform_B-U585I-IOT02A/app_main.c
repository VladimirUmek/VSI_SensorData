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
#include <string.h>

#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5

#include "RTE_Components.h"
#include CMSIS_device_header

#include "sensor_drv.h"

/* Configuration */
#define SENSOR_TEPM_ENABLED      1
#define SENSOR_HUM_ENABLED       1
#define SENSOR_PRESS_ENABLED     1
#define SENSOR_ACC_ENABLED       1
#define SENSOR_GYRO_ENABLED      1
#define SENSOR_MAG_ENABLED       1

#define SENSOR_EVENT_TOUT        5000

#ifndef SENSOR_EVENT_ENABLED
#define SENSOR_EVENT_ENABLED     0
#endif

#define SENSOR_EVENTS           (SENSOR_EVENT_TEMP_DATA_AVAILABLE  | \
                                 SENSOR_EVENT_HUM_DATA_AVAILABLE   | \
                                 SENSOR_EVENT_PRESS_DATA_AVAILABLE | \
                                 SENSOR_EVENT_ACC_DATA_AVAILABLE   | \
                                 SENSOR_EVENT_GYRO_DATA_AVAILABLE  | \
                                 SENSOR_EVENT_MAG_DATA_AVAILABLE)

/* Enable/Disable Recorder */
#define RECORDER_ENABLED         1
#if RECORDER_ENABLED
#include "Recorder.h"

/* Recorder Configuration string (first line in CSV)*/
#define REC_CFG_STR_TIMESTAMP    "timestamp,"
#if SENSOR_TEPM_ENABLED
  #define REC_CFG_STR_TEMP       "temp,"
#else
  #define REC_CFG_STR_TEMP       ""
#endif
#if SENSOR_HUM_ENABLED
  #define REC_CFG_STR_HUM        "hum,"
#else
  #define REC_CFG_STR_HUM        ""
#endif
#if SENSOR_PRESS_ENABLED
  #define REC_CFG_STR_PRESS      "press,"
#else
  #define REC_CFG_STR_PRESS      ""
#endif
#if SENSOR_ACC_ENABLED
  #define REC_CFG_STR_ACC        "acc_x,acc_y,acc_z,"
#else
  #define REC_CFG_STR_ACC        ""
#endif
#if SENSOR_GYRO_ENABLED
  #define REC_CFG_STR_GYRO       "gyro_x,gyro_y,gyro_z,"
#else
  #define REC_CFG_STR_GYRO       ""
#endif
#if SENSOR_MAG_ENABLED
  #define REC_CFG_STR_MAG        "mag_x,mag_y,mag_z,"
#else
  #define REC_CFG_STR_MAG        ""
#endif
#endif

static uint32_t Interval[6];
static int32_t  Scale[6];

static int32_t Scale_Temp;
static int32_t Scale_Acc;

static osThreadId_t Th_Read;

static void sensor_init   (void);
static void sensor_deinit (void);

#if SENSOR_EVENT_ENABLED
static void Sensor_Event (uint32_t event) {
  /* Send event(s) to the processing thread */
  osThreadFlagsSet (Th_Read, event);
}
#else
static uint32_t get_event (void) {
  uint32_t event = 0;

#if SENSOR_TEPM_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_TEMP) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_TEMP_DATA_AVAILABLE;
  }
#endif
#if SENSOR_HUM_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_HUM) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_HUM_DATA_AVAILABLE;
  }
#endif
#if SENSOR_PRESS_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_PRESS) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_PRESS_DATA_AVAILABLE;
  }
#endif
#if SENSOR_ACC_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_ACC) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_ACC_DATA_AVAILABLE;
  }
#endif
#if SENSOR_GYRO_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_GYRO) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_GYRO_DATA_AVAILABLE;
  }
#endif
#if SENSOR_MAG_ENABLED
  if (Sensor_GetStatus(SENSOR_TYPE_MAG) == SENSOR_STATUS_DATA_AVAILABLE) {
    event |= SENSOR_EVENT_MAG_DATA_AVAILABLE;
  }
#endif
  return (event);
}
#endif

static void read_sensors (void *arg) {
  uint32_t event;
  uint32_t ts;
#if SENSOR_TEPM_ENABLED
  float fTemp;
#endif
#if SENSOR_HUM_ENABLED
  float fHum;
#endif
#if SENSOR_PRESS_ENABLED
  float fPress;
#endif
#if SENSOR_ACC_ENABLED
  float fAccAxes[3];
#endif
#if SENSOR_GYRO_ENABLED
  float fGyroAxes[3];
#endif
#if SENSOR_MAG_ENABLED
  float fMagAxes[3];
#endif

#if RECORDER_ENABLED
  float  RecData[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t  RecDataCnt;
#endif

  while (1U) {
#if SENSOR_EVENT_ENABLED
    /* Wait until Sensor callback wake-up */
    event = osThreadFlagsWait (SENSOR_EVENTS, osFlagsWaitAny, SENSOR_EVENT_TOUT);
#else
    while (1) {
      osDelay (10);
      event = get_event();
      if (event != 0U) {
        break;
      }
    }
#endif

    if ((event & osFlagsError) == 0U) {
      /* Create a timestamp */
      ts = osKernelGetTickCount();
#if RECORDER_ENABLED
      RecDataCnt = 0;
      RecData[RecDataCnt++] = ts;
#endif


#if SENSOR_TEPM_ENABLED
      if (event & SENSOR_EVENT_TEMP_DATA_AVAILABLE) {
        Sensor_EnvReadData (SENSOR_TYPE_TEMP, &fTemp);

        printf ("(%d ms) Temperature: %.1f\n", ts, fTemp);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fTemp;
#endif
#endif

#if SENSOR_HUM_ENABLED
      if (event & SENSOR_EVENT_HUM_DATA_AVAILABLE) {
        Sensor_EnvReadData (SENSOR_TYPE_HUM, &fHum);

        printf ("(%d ms) Humiditty: %.1f\n", ts, fHum);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fHum;
#endif
#endif

#if SENSOR_PRESS_ENABLED
      if (event & SENSOR_EVENT_PRESS_DATA_AVAILABLE) {
        Sensor_EnvReadData (SENSOR_TYPE_PRESS, &fPress);

        printf ("(%d ms) Pressure: %.1f\n", ts, fPress);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fPress;
#endif
#endif

#if SENSOR_ACC_ENABLED
      if (event & SENSOR_EVENT_ACC_DATA_AVAILABLE) {
        Sensor_MotionReadData (SENSOR_TYPE_ACC, &fAccAxes[0], &fAccAxes[1], &fAccAxes[2]);

        printf ("(%d ms) Acceleration: %.5f, %.5f, %.5f\n", ts, fAccAxes[0], fAccAxes[1], fAccAxes[2]);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fAccAxes[0];
      RecData[RecDataCnt++] = fAccAxes[1];
      RecData[RecDataCnt++] = fAccAxes[2];
#endif
#endif

#if SENSOR_GYRO_ENABLED
      if (event & SENSOR_EVENT_GYRO_DATA_AVAILABLE) {
        Sensor_MotionReadData (SENSOR_TYPE_GYRO, &fGyroAxes[0], &fGyroAxes[1], &fGyroAxes[2]);

        printf ("(%d ms) Gyroscope: %.5f, %.5f, %.5f\n", ts, fGyroAxes[0], fGyroAxes[1], fGyroAxes[2]);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fGyroAxes[0];
      RecData[RecDataCnt++] = fGyroAxes[1];
      RecData[RecDataCnt++] = fGyroAxes[2];
#endif
#endif

#if SENSOR_MAG_ENABLED
      if (event & SENSOR_EVENT_MAG_DATA_AVAILABLE) {
        Sensor_MotionReadData (SENSOR_TYPE_MAG, &fMagAxes[0], &fMagAxes[1], &fMagAxes[2]);

        printf ("(%d ms) Magnetometer: %.5f, %.5f, %.5f\n", ts, fMagAxes[0], fMagAxes[1], fMagAxes[2]);
      }
#if RECORDER_ENABLED
      RecData[RecDataCnt++] = fMagAxes[0];
      RecData[RecDataCnt++] = fMagAxes[1];
      RecData[RecDataCnt++] = fMagAxes[2];
#endif
#endif
#if RECORDER_ENABLED
      Recorder_Write((uint8_t *)RecData, RecDataCnt*4);
#endif
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

static void sensor_init (void) {
#if SENSOR_EVENT_ENABLED
  Sensor_Initialize (Sensor_Event);
#else
  Sensor_Initialize (NULL);
#endif

#if SENSOR_TEPM_ENABLED
    Sensor_Enable (SENSOR_TYPE_TEMP);
    printf ("Temperature: enabled.\n");
#endif
#if SENSOR_HUM_ENABLED
    Sensor_Enable (SENSOR_TYPE_HUM);
    printf ("Humidity: enabled.\n");
#endif
#if SENSOR_PRESS_ENABLED
    Sensor_Enable (SENSOR_TYPE_PRESS);
    printf ("Pressure: enabled.\n");
#endif
#if SENSOR_ACC_ENABLED
    Sensor_Enable (SENSOR_TYPE_ACC);
    printf ("Accelerometer: enabled.\n");
#endif
#if SENSOR_GYRO_ENABLED
    Sensor_Enable (SENSOR_TYPE_GYRO);
    printf ("Gyroscope: enabled.\n");
#endif
#if SENSOR_MAG_ENABLED
    Sensor_Enable (SENSOR_TYPE_MAG);
    printf ("Magnetometer: enabled.\n");
#endif
}

void sensor_deinit (void) {

#if SENSOR_TEPM_ENABLED
    Sensor_Disable (SENSOR_TYPE_TEMP);
    printf ("Temperature: disabled.\n");
#endif
#if SENSOR_HUM_ENABLED
    Sensor_Disable (SENSOR_TYPE_HUM);
    printf ("Humidity: disabled.\n");
#endif
#if SENSOR_PRESS_ENABLED
    Sensor_Disable (SENSOR_TYPE_PRESS);
    printf ("Pressure: disabled.\n");
#endif
#if SENSOR_ACC_ENABLED
    Sensor_Disable (SENSOR_TYPE_ACC);
    printf ("Accelerometer: disabled.\n");
#endif
#if SENSOR_GYRO_ENABLED
    Sensor_Disable (SENSOR_TYPE_GYRO);
    printf ("Gyroscope: disabled.\n");
#endif
#if SENSOR_MAG_ENABLED
    Sensor_Disable (SENSOR_TYPE_MAG);
    printf ("Magnetometer: disabled.\n");
#endif

  Sensor_Uninitialize();
}

/*---------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
static void app_main (void *argument) {
  osThreadAttr_t attr;

#if RECORDER_ENABLED
  /* Initialize recorder */
  const char rec_cfg_data[256];

  /* Generate configuration line: first line of the CSV */
  snprintf((char *)rec_cfg_data, sizeof(rec_cfg_data), "%s%s%s%s%s%s%s\r\n", REC_CFG_STR_TIMESTAMP, REC_CFG_STR_TEMP, REC_CFG_STR_HUM,
                                                                             REC_CFG_STR_PRESS, REC_CFG_STR_ACC, REC_CFG_STR_GYRO, REC_CFG_STR_MAG);
  Recorder_Initialize((uint8_t *)rec_cfg_data, strlen((const char *)rec_cfg_data));
#endif

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
