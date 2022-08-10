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

#define SENSOR_TYPE_TEMP                  0
#define SENSOR_TYPE_HUM                   1
#define SENSOR_TYPE_PRESS                 2
#define SENSOR_TYPE_ACC                   3
#define SENSOR_TYPE_GYRO                  4
#define SENSOR_TYPE_MAG                   5

#define SENSOR_EVENT_TEMP_DATA_AVAILABLE  (1UL << SENSOR_TYPE_TEMP)
#define SENSOR_EVENT_HUM_DATA_AVAILABLE   (1UL << SENSOR_TYPE_HUM)
#define SENSOR_EVENT_PRESS_DATA_AVAILABLE (1UL << SENSOR_TYPE_PRESS)
#define SENSOR_EVENT_ACC_DATA_AVAILABLE   (1UL << SENSOR_TYPE_ACC)
#define SENSOR_EVENT_GYRO_DATA_AVAILABLE  (1UL << SENSOR_TYPE_GYRO)
#define SENSOR_EVENT_MAG_DATA_AVAILABLE   (1UL << SENSOR_TYPE_MAG)

#define SENSOR_CTRL_ENABLE_TEMP           (1UL << SENSOR_TYPE_TEMP)
#define SENSOR_CTRL_ENABLE_HUM            (1UL << SENSOR_TYPE_HUM)
#define SENSOR_CTRL_ENABLE_PRESS          (1UL << SENSOR_TYPE_PRESS)
#define SENSOR_CTRL_ENABLE_ACC            (1UL << SENSOR_TYPE_ACC)
#define SENSOR_CTRL_ENABLE_GYRO           (1UL << SENSOR_TYPE_GYRO)
#define SENSOR_CTRL_ENABLE_MAG            (1UL << SENSOR_TYPE_MAG)

/* Return Codes */
#define SENSOR_OK                         (0)  ///< Operation succeeded
#define SENSOR_ERROR                      (-1) ///< Unspecified error
#define SENSOR_BUSY                       (-2) ///< Sensor interface is busy
#define SENSOR_TIMEOUT                    (-3) ///< Timeout occurred
#define SENSOR_UNSUPPORTED                (-4) ///< Operation not supported
#define SENSOR_INVALID_PARAMETER          (-5) ///< Parameter error

#if 0
/* Sensor units

                  | Primary Unit    | Alternative Unit | Conversion                     | NXP Sensor Fusion
  Temperature:    | degrees Celsius | Kelvin           | 1[degC] == 274.15[K]           | degC
  Humidity:       | percentage      |                  |                                |
  Pressure:       | hPa             | kPa              |                                |
  Light:          | lx (lux)        |                  | 1[lx] == 1[lm/m2]              |
  Acceleration:   | G               | m/s2             | 1[m/s2] == 9.8066[G]           | G
  Angular rate:   | deg/s (dps)     | rad/s            | 1[dps] == 0.0175[rad/s]        | dps (deg/s)
  Magnetic field: | uT (uTesla)     | Gauss            | 1[uT] == 0.01[G] (1G == 100uT) | uT

  See: https://www.iana.org/assignments/senml/senml.xhtml
  See: https://au.mathworks.com/help/supportpkg/beagleboneblue/ref/mpu9250.html#mw_767456e9-c318-4f60-ae79-3413342658e0
  See: https://sst.semiconductor-digest.com/2010/11/introduction-to-mems-gyroscopes/
  
  Output Data Rate: f = 1/T  1Hz = 1s = 1000ms = 1000000us
*/
typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} SensorAxes_t;
#endif


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
  \fn          int32_t Sensor_MotionReadData (uint32_t type, int32_t *x, int32_t *y, int32_t *z)
  \brief       Read motion sensor data.
  \return      >=0 number of data items available to read
               < 0 return code
*/
int32_t Sensor_MotionReadData (uint32_t type, int32_t *x, int32_t *y, int32_t *z);

/**
  \fn          int32_t Sensor_EnvReadData (uint32_t type, int32_t *data)
  \brief       Read environmental sensor data.
  \return      >=0 number of data items available to read
               < 0 return code
*/
int32_t Sensor_EnvReadData (uint32_t type, int32_t *data);

/**
  \fn          int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len)
  \param[in]   type sensor type
  \param[in]   data pointer to array of integers
  \param[in]   len  data array length
  \brief       Retrieve sensor sensitivity scale range.
  \return      number of valid fields in data array
*/
int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len);

/**
  \fn          int32_t Sensor_GetScale (uint32_t type, int32_t *data)
  \brief       Get sensor sensitivity scale.
  \return      sensitivity scale
*/
int32_t Sensor_GetScale (uint32_t type, int32_t *data);

/**
  \fn          int32_t Sensor_SetScale (uint32_t type, int32_t data)
  \brief       Set sensor sensitivity scale.
  \return      return code
*/
int32_t Sensor_SetScale (uint32_t type, int32_t data);

/**
  \fn          int32_t Sensor_QueryDataRate (uint32_t type, uint32_t data[], uint32_t len)
  \param[in]   type sensor type
  \param[in]   data pointer to array of integers
  \param[in]   len  data array length
  \brief       Retrieve available data rate sensor data rate range.
  \return      number of valid fields in data array
*/
int32_t Sensor_QueryDataRate (uint32_t type, uint32_t data[], uint32_t len);
int32_t Sensor_QueryInterval (uint32_t type, uint32_t period[], uint32_t len);

/**
  \fn          int32_t Sensor_GetDataRate (uint32_t type, uint32_t *data_rate)
  \brief       Get sensor output data rate.
  \return      otuput data rate in Hz
*/
int32_t Sensor_GetDataRate (uint32_t type, uint32_t *rate);
int32_t Sensor_GetInterval (uint32_t type, uint32_t *period);

/**
  \fn          int32_t Sensor_SetDataRate (uint32_t type, uint32_t data_rate)
  \brief       Set sensor output data rate.
  \return      return code
*/
int32_t Sensor_SetDataRate (uint32_t type, uint32_t rate);
int32_t Sensor_SetInterval (uint32_t type, uint32_t period);

#ifdef  __cplusplus
}
#endif

#endif /* SENSOR_DRV_H__ */
