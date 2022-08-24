/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Sensor driver for B-U585I-IOT20A
 */

#include "sensor_drv.h"
#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"

// Sensor configuration
#define ACC_SENSITIVITY ISM330DHCX_ACC_SENSITIVITY_FS_4G
#define ACC_FULL_SCALE  ISM330DHCX_2g

const uint32_t temperature_interval[]   = {1000000, 142857, 83333};
const uint32_t humidity_interval[]      = {1000000, 142857, 83333};
const uint32_t pressure_interval[]      = {1000000, 100000, 40000, 20000, 13333, 10000, 5000};
const uint32_t accelerometer_interval[] = {38462, 19231, 9615, 4808, 2398, 1200, 600, 300, 150};
const uint32_t gyroscope_interval[]     = {38462, 19231, 9615, 4808, 2398, 1200, 600, 300, 150};
const uint32_t magnetometer_interval[]  = {100000, 50000, 20000, 10000};

/* Externals */
extern void *Env_Sensor_CompObj[ENV_SENSOR_INSTANCES_NBR];
extern void *Motion_Sensor_CompObj[MOTION_SENSOR_INSTANCES_NBR];

/**
  Error Code
*/
static int32_t Sensor_Error (int32_t err) {
  int32_t ret_err = SENSOR_ERROR;

  switch (err) {
    case BSP_ERROR_NONE:
      ret_err = SENSOR_OK;
      break;
    case BSP_ERROR_NO_INIT:
    case BSP_ERROR_PERIPH_FAILURE:
    case BSP_ERROR_COMPONENT_FAILURE:
    case BSP_ERROR_UNKNOWN_FAILURE:
    case BSP_ERROR_UNKNOWN_COMPONENT:
    case BSP_ERROR_BUS_FAILURE:
    case BSP_ERROR_CLOCK_FAILURE:
    case BSP_ERROR_MSP_FAILURE:
      ret_err = SENSOR_ERROR;
      break;
    case BSP_ERROR_BUSY:
      ret_err = SENSOR_BUSY;
      break;
    case BSP_ERROR_FEATURE_NOT_SUPPORTED:
      ret_err = SENSOR_UNSUPPORTED;
      break;
    case BSP_ERROR_WRONG_PARAM:
      ret_err = SENSOR_INVALID_PARAMETER;
      break;
  }
  return ret_err;
}

/**
  Initialize Sensor Interface
*/
int32_t Sensor_Initialize (Sensor_Event_t cb_event) {
  int32_t err;

  // Temperature sensor
  err = Sensor_Error(BSP_ENV_SENSOR_Init(0, ENV_TEMPERATURE));

  // Humidity sensor
  if (err == SENSOR_OK) {
    err = Sensor_Error(BSP_ENV_SENSOR_Init(0, ENV_HUMIDITY));
  }

  // Humidity Pressure
  if (err == SENSOR_OK) {
    err = Sensor_Error(BSP_ENV_SENSOR_Init(1, ENV_PRESSURE));
  }

  // Accelerometer
  if (err == SENSOR_OK) {
    err = Sensor_Error(BSP_MOTION_SENSOR_Init(0, MOTION_ACCELERO));
  }

  // Gyroscope
  if (err == SENSOR_OK) {
    err = Sensor_Error(BSP_MOTION_SENSOR_Init(0, MOTION_GYRO));
  }

  // Magnetometer
  if (err == SENSOR_OK) {
    err = Sensor_Error(BSP_MOTION_SENSOR_Init(1, MOTION_MAGNETO));
  }

  return err;
}

/**
  De-initialize Sensor Interface.
*/
int32_t Sensor_Uninitialize (void) {
  BSP_ENV_SENSOR_DeInit(0);
  BSP_ENV_SENSOR_DeInit(1);
  BSP_MOTION_SENSOR_DeInit(0);
  BSP_MOTION_SENSOR_DeInit(1);

  return SENSOR_OK;
}

/**
  Enable Sensor Interface.
*/
int32_t Sensor_Enable (uint32_t type) {
  int32_t err;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      err = Sensor_Error(BSP_ENV_SENSOR_Enable(0, ENV_TEMPERATURE));
      break;
    case SENSOR_TYPE_HUM:
      err = Sensor_Error(BSP_ENV_SENSOR_Enable(0, ENV_HUMIDITY));
      break;
    case SENSOR_TYPE_PRESS:
      err = Sensor_Error(BSP_ENV_SENSOR_Enable(1, ENV_PRESSURE));
      break;
    case SENSOR_TYPE_ACC:
      err = Sensor_Error(BSP_MOTION_SENSOR_Enable(0, MOTION_ACCELERO));
      break;
    case SENSOR_TYPE_GYRO:
      err = Sensor_Error(BSP_MOTION_SENSOR_Enable(0, MOTION_GYRO));
      break;
    case SENSOR_TYPE_MAG:
      err = Sensor_Error(BSP_MOTION_SENSOR_Enable(1, MOTION_MAGNETO));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  return err;
}

/**
  Disable Sensor Interface.
*/
int32_t Sensor_Disable (uint32_t type) {
  int32_t err;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      err = Sensor_Error(BSP_ENV_SENSOR_Disable(0, ENV_TEMPERATURE));
      break;
    case SENSOR_TYPE_HUM:
      err = Sensor_Error(BSP_ENV_SENSOR_Disable(0, ENV_HUMIDITY));
      break;
    case SENSOR_TYPE_PRESS:
      err = Sensor_Error(BSP_ENV_SENSOR_Disable(1, ENV_PRESSURE));
      break;
    case SENSOR_TYPE_ACC:
      err = Sensor_Error(BSP_MOTION_SENSOR_Disable(0, MOTION_ACCELERO));
      break;
    case SENSOR_TYPE_GYRO:
      err = Sensor_Error(BSP_MOTION_SENSOR_Disable(0, MOTION_GYRO));
      break;
    case SENSOR_TYPE_MAG:
      err = Sensor_Error(BSP_MOTION_SENSOR_Disable(1, MOTION_MAGNETO));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  return err;
}

/**
  Read motion sensor data.
*/
int32_t Sensor_MotionReadData (uint32_t type, float *x, float *y, float *z) {
  int32_t err = 0;
  BSP_MOTION_SENSOR_Axes_t axes;

  switch (type) {
    case SENSOR_TYPE_ACC:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetAxes(0, MOTION_ACCELERO, &axes));
      break;
    case SENSOR_TYPE_GYRO:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetAxes(0, MOTION_GYRO, &axes));
      break;
    case SENSOR_TYPE_MAG:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetAxes(1, MOTION_MAGNETO, &axes));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  if (err == SENSOR_OK) {
    *x = (float)axes.xval;
    *y = (float)axes.yval;
    *z = (float)axes.zval;
  }

  return err;

}

/**
  Read environmental sensor data.
*/
int32_t Sensor_EnvReadData (uint32_t type, float *data) {
  int32_t err = 0;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      err = Sensor_Error(BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, data));
      break;
    case SENSOR_TYPE_HUM:
      err = Sensor_Error(BSP_ENV_SENSOR_GetValue(0, ENV_HUMIDITY, data));
      break;
    case SENSOR_TYPE_PRESS:
      err = Sensor_Error(BSP_ENV_SENSOR_GetValue(1, ENV_PRESSURE, data));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  return err;
}

/**
  Retrive sensor status.
*/
int32_t Sensor_GetStatus (uint32_t type) {
  int32_t status = 0;
  int32_t ret;
  uint8_t stat;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      ret = HTS221_TEMP_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
      if (stat & (1U)) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      break;
    case SENSOR_TYPE_HUM:
      ret = HTS221_HUM_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
      if (stat & (2U)) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      break;
    case SENSOR_TYPE_PRESS:
      ret = LPS22HH_PRESS_Get_DRDY_Status(Env_Sensor_CompObj[1], &stat);
      if (stat &  1U) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      if (stat & (1U << 4)) {
        status |= SENSOR_STATUS_DATA_OVERRUN;
      }
      break;
    case SENSOR_TYPE_ACC:
      ret = ISM330DHCX_ACC_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
      if (stat & 1U) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      break;
    case SENSOR_TYPE_GYRO:
      ret = ISM330DHCX_GYRO_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
      if (stat & 2U) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      break;
    case SENSOR_TYPE_MAG:
      ret = IIS2MDC_MAG_Get_DRDY_Status(Motion_Sensor_CompObj[1], &stat);
      if (stat & (0x0FU)) {
        status |= SENSOR_STATUS_DATA_AVAILABLE;
      }
      if (stat & (0xF0U)) {
        status |= SENSOR_STATUS_DATA_OVERRUN;
      }
      break;
    default:
      status = SENSOR_INVALID_PARAMETER;
  }
  if (ret != 0) {
    status = SENSOR_ERROR;
  }

  return status;
}

/**
  Retrieve available sensor data sampling intervals
*/
int32_t Sensor_QueryInterval (uint32_t type, uint32_t interval[], uint32_t len) {
  int32_t err = SENSOR_OK;
  uint32_t buf_sz = len * 4;
  uint32_t *data_rate_buffer;
  uint32_t data_rate_buffer_size;

  memset(interval, 0, buf_sz);

  switch (type) {
    case SENSOR_TYPE_TEMP:
      data_rate_buffer      = (uint32_t *)temperature_interval;
      data_rate_buffer_size = sizeof(temperature_interval);
      break;
    case SENSOR_TYPE_HUM:
      data_rate_buffer      = (uint32_t *)humidity_interval;
      data_rate_buffer_size = sizeof(humidity_interval);
      break;
    case SENSOR_TYPE_PRESS:
      data_rate_buffer      = (uint32_t *)pressure_interval;
      data_rate_buffer_size = sizeof(pressure_interval);
      break;
    case SENSOR_TYPE_ACC:
      data_rate_buffer      = (uint32_t *)accelerometer_interval;
      data_rate_buffer_size = sizeof(accelerometer_interval);
      break;
    case SENSOR_TYPE_GYRO:
      data_rate_buffer      = (uint32_t *)gyroscope_interval;
      data_rate_buffer_size = sizeof(gyroscope_interval);
      break;
    case SENSOR_TYPE_MAG:
      data_rate_buffer      = (uint32_t *)magnetometer_interval;
      data_rate_buffer_size = sizeof(magnetometer_interval);
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  if (err == SENSOR_OK) {
    if (data_rate_buffer_size < buf_sz) {
      buf_sz = data_rate_buffer_size;
    }
    memcpy(interval, data_rate_buffer, buf_sz);
  }

  return err;
}

/**
  Get sensor sampling interval.
*/
uint32_t Sensor_GetInterval (uint32_t type) {
  int32_t  err;
  uint32_t interval = 0U;
  float    data_rate;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      err = Sensor_Error(BSP_ENV_SENSOR_GetOutputDataRate(0, ENV_TEMPERATURE, &data_rate));
      break;
    case SENSOR_TYPE_HUM:
      err = Sensor_Error(BSP_ENV_SENSOR_GetOutputDataRate(0, ENV_HUMIDITY, &data_rate));
      break;
    case SENSOR_TYPE_PRESS:
      err = Sensor_Error(BSP_ENV_SENSOR_GetOutputDataRate(1, ENV_PRESSURE, &data_rate));
      break;
    case SENSOR_TYPE_ACC:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetOutputDataRate(0, MOTION_ACCELERO, &data_rate));
      break;
    case SENSOR_TYPE_GYRO:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetOutputDataRate(0, MOTION_GYRO, &data_rate));
      break;
    case SENSOR_TYPE_MAG:
      err = Sensor_Error(BSP_MOTION_SENSOR_GetOutputDataRate(1, MOTION_MAGNETO, &data_rate));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  if (err == SENSOR_OK) {
    interval = (uint32_t)data_rate;
  }

  return interval;
}

/**
  Set sensor sampling interval.
*/
int32_t Sensor_SetInterval (uint32_t type, uint32_t interval) {
  int32_t err;
  float  data_rate = ((1.0 / interval) + 0.5) * 1000000;

  switch (type) {
    case SENSOR_TYPE_TEMP:
      err = Sensor_Error(BSP_ENV_SENSOR_SetOutputDataRate(0, ENV_TEMPERATURE, data_rate));
      break;
    case SENSOR_TYPE_HUM:
      err = Sensor_Error(BSP_ENV_SENSOR_SetOutputDataRate(0, ENV_HUMIDITY, data_rate));
      break;
    case SENSOR_TYPE_PRESS:
      err = Sensor_Error(BSP_ENV_SENSOR_SetOutputDataRate(1, ENV_PRESSURE, data_rate));
      break;
    case SENSOR_TYPE_ACC:
      err = Sensor_Error(BSP_MOTION_SENSOR_SetOutputDataRate(0, MOTION_ACCELERO, data_rate));
      break;
    case SENSOR_TYPE_GYRO:
      err = Sensor_Error(BSP_MOTION_SENSOR_SetOutputDataRate(0, MOTION_GYRO, data_rate));
      break;
    case SENSOR_TYPE_MAG:
      err = Sensor_Error(BSP_MOTION_SENSOR_SetOutputDataRate(0, MOTION_MAGNETO, data_rate));
      break;
    default:
      err = SENSOR_INVALID_PARAMETER;
  }

  return err;
}

/**
  Retrieve sensor sensitivity scale range.
*/
int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len) {
  return SENSOR_UNSUPPORTED;
}

/**
  Get sensor sensitivity scale.
*/
int32_t Sensor_GetScale (uint32_t type) {
  return SENSOR_UNSUPPORTED;
}

/**
  Set sensor sensitivity scale.
*/
int32_t Sensor_SetScale (uint32_t type, int32_t data) {
  return SENSOR_UNSUPPORTED;
}
