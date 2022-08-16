/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#include <stddef.h>
#include "sensor_drv.h"
#include "sensor_vsi.h"
#include "arm_vsi.h"

#include "RTE_Components.h"
#include CMSIS_device_header

/* Sensor units

|       |       Unit      | Alternative Unit | Conversion              | NXP Sensor Fusion | STMicro X-CUBE-MEMS
| TEMP  | degrees Celsius | Kelvin           | 1[degC] == 274.15[K]    | degC              | degC
| HUM   | % (percentage)  |                  |                         | %                 | %
| PRESS | hPa             | kPa              | 1[hPa] == 0.1[kPa]      |                   | Pa
| ACC   | G (gravity)     | m/s2             | 1[m/s2] == 9.8066[G]    | G                 | mG
| GYRO  | dps (deg/s)     | rad/s            | 1[dps] == 0.0175[rad/s] | dps (deg/s)       | mdps
| MAG   | uT (micro Tesla)| Gauss            | 1[uT] == 0.01[G]        | uT                | mGauss

  See: https://www.iana.org/assignments/senml/senml.xhtml
  See: https://au.mathworks.com/help/supportpkg/beagleboneblue/ref/mpu9250.html#mw_767456e9-c318-4f60-ae79-3413342658e0
  See: https://sst.semiconductor-digest.com/2010/11/introduction-to-mems-gyroscopes/
*/

#define VSI               ARM_VSI0              /* VSI instance          */
#define VSI_IRQn          ARM_VSI0_IRQn         /* VSI interrupt number  */
#define VSI_Handler       ARM_VSI0_Handler      /* VSI interrupt handler */

#define STATUS            Regs[0]
#define INTERVAL          Regs[1]
#define SELECT            Regs[2]
#define ENABLE            Regs[3]
#define SCALE             Regs[4]
#define ODR               Regs[5]
#define FIFO_CNT          Regs[6]
#define FIFO              Regs[7]

/* Number of sensors implemented using VSI peripheral */
#define SENSOR_COUNT      6

/* Event Callback */
static Sensor_Event_t CB_Event = NULL;

/* VSI interrupt handler */
void VSI_Handler (void) {
  uint32_t status;
  uint32_t event;

  VSI->IRQ.Clear = 1U;
  __DSB();
  __ISB();

  status = VSI->STATUS;

  event = 0U;

  if (status & SENSOR_STATUS_FIFO_NE_TEMP) {
    event |= SENSOR_EVENT_TEMP_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_HUM) {
    event |= SENSOR_EVENT_HUM_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_PRESS) {
    event |= SENSOR_EVENT_PRESS_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_ACC) {
    event |= SENSOR_EVENT_ACC_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_GYRO) {
    event |= SENSOR_EVENT_GYRO_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_MAG) {
    event |= SENSOR_EVENT_MAG_DATA_AVAILABLE;
  }

  if ((CB_Event != NULL) && (event != 0U)) {
    CB_Event(event);
  }
}

static uint32_t IsTypeValid (uint32_t type) {

  if ((type == SENSOR_TYPE_TEMP) || (type == SENSOR_TYPE_HUM)  || (type == SENSOR_TYPE_PRESS) ||
      (type == SENSOR_TYPE_ACC)  || (type == SENSOR_TYPE_GYRO) || (type == SENSOR_TYPE_MAG))  {
    return (1U);
  } else {
    /* None of the above */
    return (0U);
  }
}

static uint32_t IsTypeEnv (uint32_t type) {

  if ((type == SENSOR_TYPE_TEMP) || (type == SENSOR_TYPE_HUM)  || (type == SENSOR_TYPE_PRESS)) {
    return (1U);
  } else {
    /* Not environmental sensor */
    return (0U);
  }
}


int32_t Sensor_Initialize (Sensor_Event_t cb_event) {
  uint32_t id;

  CB_Event = cb_event;

  /* Initialize VSI peripheral */
  VSI->Timer.Control = 0U;
  VSI->DMA.Control   = 0U;
  VSI->IRQ.Clear     = 0x00000001U;
  VSI->IRQ.Enable    = 0x00000001U;

  /* Disable sensors */
  for (id = 0U; id < SENSOR_COUNT; id++) {
    VSI->SELECT = id;
    VSI->ENABLE = 0U;
  }

  /* Enable VSI interrupts */
  NVIC->ISER[(((uint32_t)VSI_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)VSI_IRQn) & 0x1FUL));
  // NVIC_EnableIRQ(VSI_IRQn);
  __DSB();
  __ISB();

  return SENSOR_OK;
}

int32_t Sensor_Uninitialize (void) {
  uint32_t id;

  /* Disable VSI interrupts */
  NVIC->ICER[(((uint32_t)VSI_IRQn)  >> 5UL)] = (uint32_t)(1UL << (((uint32_t)VSI_IRQn)  & 0x1FUL));
  // NVIC_DisableIRQ(VSI_IRQn);
  __DSB();
  __ISB();

  /* De-initialize VSI output */
  VSI->Timer.Control = 0U;
  VSI->DMA.Control   = 0U;
  VSI->IRQ.Clear     = 0x00000001U;
  VSI->IRQ.Enable    = 0x00000000U;
  
  /* Disable sensors */
  for (id = 0U; id < SENSOR_COUNT; id++) {
    VSI->SELECT = id;
    VSI->ENABLE = 0U;
  }

  CB_Event = NULL;

  return SENSOR_OK;
}

int32_t Sensor_Enable (uint32_t type) {
  uint32_t ctrl;
  uint32_t div;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Enable sensor */
  VSI->SELECT = type;
  VSI->ENABLE = 1U;

  /* Re-evaluate peripheral clock divider */
  div = VSI->INTERVAL;

  if (VSI->Timer.Interval != div) {
    /* Re-configure peripheral timer (clock) */
    VSI->Timer.Interval = div;
    VSI->Timer.Control = ARM_VSI_Timer_Trig_IRQ_Msk |
                         ARM_VSI_Timer_Periodic_Msk |
                         ARM_VSI_Timer_Run_Msk;
  }

  return SENSOR_OK;
}

int32_t Sensor_Disable (uint32_t type) {
  uint32_t ctrl;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Disable sensor */
  VSI->SELECT = type;
  VSI->ENABLE = 0U;

  return SENSOR_OK;
}


int32_t Sensor_MotionReadData (uint32_t type, float *x, float *y, float *z) {
  uint32_t num;
  uint32_t scale;
  int32_t axes[3];

  /* Select requested sensor */
  VSI->SELECT = type;

  /* Read current scale setting */
  scale = VSI->SCALE;

  /* Read number of samples available in FIFO */
  num = VSI->FIFO_CNT;

  if (num > 0U) {
    /* Read FIFO */
    axes[0] = VSI->FIFO;
    axes[1] = VSI->FIFO;
    axes[2] = VSI->FIFO;

    *x = (float)axes[0] / scale;
    *y = (float)axes[1] / scale;
    *z = (float)axes[2] / scale;

    /* Decrease number of samples available */
    num -= 3U;
  }

  /* Return number of samples available to read */
  return (num);
}


int32_t Sensor_EnvReadData (uint32_t type, float *data) {
  uint32_t num;
  uint32_t scale;
  int32_t val;

  /* Select requested sensor */
  VSI->SELECT = type;

  /* Read current scale setting */
  scale = VSI->SCALE;

  /* Read number of samples available in FIFO */
  num = VSI->FIFO_CNT;

  if (num > 0U) {
    /* Read FIFO */
    val = VSI->FIFO;

    *data = (float)val / scale;

    /* Decrement number of samples available */
    num--;
  }

  /* Return number of samples available to read */
  return (num);
}


int32_t Sensor_QueryInterval (uint32_t type, uint32_t interval[], uint32_t len) {

  if ((IsTypeValid(type) == 0U) || (interval == NULL) || (len == 0U)) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* This VSI driver implements single sampling interval */
  interval[0] = VSI->ODR;

  return (1U);
}


uint32_t Sensor_GetInterval (uint32_t type) {
  uint32_t odr;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current sampling interval */
  odr = VSI->ODR;

  return (odr);
}


int32_t Sensor_SetInterval (uint32_t type, uint32_t interval) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* Set sampling interval */
  VSI->ODR = interval;

  return (SENSOR_OK);
}

int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len) {

  if ((IsTypeValid(type) == 0U) || (data == NULL) || (len == 0U)) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current full scale setting */
  data[0] = VSI->SCALE;

  return (1U);
}


int32_t Sensor_GetScale (uint32_t type) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current full scale setting */
  scale = VSI->SCALE;

  return (scale);
}


int32_t Sensor_SetScale (uint32_t type, int32_t data) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  /* Select sensor */
  VSI->SELECT = type;

  /* Set full scale setting */
  VSI->SCALE = data;

  return (SENSOR_OK);
}
