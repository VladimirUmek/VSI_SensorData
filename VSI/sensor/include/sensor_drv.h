/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SENSOR_DRV_H__
#define SENSOR_DRV_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* Sensor Type */
#define SENSOR_TYPE_TEMP                  0U
#define SENSOR_TYPE_HUM                   1U
#define SENSOR_TYPE_PRESS                 2U
#define SENSOR_TYPE_ACC                   3U
#define SENSOR_TYPE_GYRO                  4U
#define SENSOR_TYPE_MAG                   5U

/* Sensor Event */
#define SENSOR_EVENT_TEMP_DATA_AVAILABLE  (1UL << SENSOR_TYPE_TEMP)
#define SENSOR_EVENT_HUM_DATA_AVAILABLE   (1UL << SENSOR_TYPE_HUM)
#define SENSOR_EVENT_PRESS_DATA_AVAILABLE (1UL << SENSOR_TYPE_PRESS)
#define SENSOR_EVENT_ACC_DATA_AVAILABLE   (1UL << SENSOR_TYPE_ACC)
#define SENSOR_EVENT_GYRO_DATA_AVAILABLE  (1UL << SENSOR_TYPE_GYRO)
#define SENSOR_EVENT_MAG_DATA_AVAILABLE   (1UL << SENSOR_TYPE_MAG)

/* Return Codes */
#define SENSOR_OK                         (0)  ///< Operation succeeded
#define SENSOR_ERROR                      (-1) ///< Unspecified error
#define SENSOR_BUSY                       (-2) ///< Sensor interface is busy
#define SENSOR_TIMEOUT                    (-3) ///< Timeout occurred
#define SENSOR_UNSUPPORTED                (-4) ///< Operation not supported
#define SENSOR_INVALID_PARAMETER          (-5) ///< Parameter error


/**
  \fn          Sensor_Event_t
  \brief       Sensor Interface Event callback function type: void (*Sensor_Event_t) (uint32_t event)
  \param[in]   event events notification mask
  \return      none
*/
typedef void (*Sensor_Event_t) (uint32_t event);

/**
  \fn          int32_t Sensor_Initialize (Sensor_Event_t cb_event)
  \brief       Initialize Sensor Interface.
  \param[in]   cb_event pointer to \ref Sensor_Event_t
  \return      return code
*/
int32_t Sensor_Initialize (Sensor_Event_t cb_event);

/**
  \fn          int32_t Sensor_Uninitialize (void)
  \brief       De-initialize Sensor Interface.
  \return      return code
*/
int32_t Sensor_Uninitialize (void);

/**
  \fn          int32_t Sensor_Enable (uint32_t type)
  \brief       Enable Sensor Interface.
  \return      return code
*/
int32_t Sensor_Enable (uint32_t type);

/**
  \fn          int32_t Sensor_Disable (uint32_t type)
  \brief       Disable Sensor Interface.
  \return      return code
*/
int32_t Sensor_Disable (uint32_t type);

/**
  \fn          int32_t Sensor_MotionReadData (uint32_t type, float *x, float *y, float *z)
  \brief       Read motion sensor data.
  \param[in]   type sensor type
  \param[in]   x    pointer to variable that stores x axis data
  \param[in]   y    pointer to variable that stores y axis data
  \param[in]   z    pointer to variable that stores z axis data
  \return      >=0 number of data items available to read
               < 0 return code
*/
int32_t Sensor_MotionReadData (uint32_t type, float *x, float *y, float *z);
//int32_t Sensor_ReadAxes (uint32_t type, float *x, float *y, float *z);

/**
  \fn          int32_t Sensor_EnvReadData (uint32_t type, float *data)
  \brief       Read environmental sensor data.
  \param[in]   type sensor type
  \param[in]   data pointer to data variable
  \return      >=0 number of data items available to read
               < 0 return code
*/
int32_t Sensor_EnvReadData (uint32_t type, float *data);
//int32_t Sensor_ReadData (uint32_t type, float *data);

/**
  \fn          int32_t Sensor_QueryInterval (uint32_t type, uint32_t period[], uint32_t len)
  \brief       Retrieve available sensor data sampling intervals
  \param[in]   type     sensor type
  \param[in]   interval pointer to array that stores sampling interval in microseconds
  \param[in]   len      interval array length
  \return      number of valid fields in data array
*/
int32_t Sensor_QueryInterval (uint32_t type, uint32_t interval[], uint32_t len);

/**
  \fn          uint32_t Sensor_GetInterval (uint32_t type)
  \brief       Get sensor sampling interval.
  \return      sensor sampling interval in microseconds
*/
uint32_t Sensor_GetInterval (uint32_t type);

/**
  \fn          int32_t Sensor_SetInterval (uint32_t type, uint32_t period)
  \brief       Set sensor sampling interval.
  \param[in]   interval sampling interval in microseconds
  \return      return code
*/
int32_t Sensor_SetInterval (uint32_t type, uint32_t interval);

/**
  \fn          int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len)
  \brief       Retrieve sensor sensitivity scale range.
  \param[in]   type sensor type
  \param[in]   data pointer to array of integers
  \param[in]   len  data array length
  \return      number of valid fields in data array
*/
int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len);

/**
  \fn          int32_t Sensor_GetScale (uint32_t type)
  \brief       Get sensor sensitivity scale.
  \return      sensitivity scale
*/
int32_t Sensor_GetScale (uint32_t type);

/**
  \fn          int32_t Sensor_SetScale (uint32_t type, int32_t data)
  \brief       Set sensor sensitivity scale.
  \return      return code
*/
int32_t Sensor_SetScale (uint32_t type, int32_t data);

#ifdef  __cplusplus
}
#endif

#endif /* SENSOR_DRV_H__ */
