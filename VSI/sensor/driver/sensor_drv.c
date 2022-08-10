/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 */

#include <stddef.h>
#include "sensor_drv.h"
#include "sensor_vsi.h"
#include "arm_vsi.h"

#include "RTE_Components.h"
#include CMSIS_device_header

#define VSI               ARM_VSI0              /* VSI instance          */
#define VSI_IRQn          ARM_VSI0_IRQn         /* VSI interrupt number  */
#define VSI_Handler       ARM_VSI0_Handler      /* VSI interrupt handler */

#define CTRL              Regs[0]
#define STATUS            Regs[1]
#define CLK_DIV           Regs[2]
#define SELECT            Regs[3]
#define SCALE             Regs[4]
#define ODR               Regs[5]
#define FIFO_CNT          Regs[6]
#define FIFO              Regs[7]
#define FIFO_MOTION_X     Regs[8]
#define FIFO_MOTION_Y     Regs[9]
#define FIFO_MOTION_Z     Regs[10]


#define TIME_BETWEEN_SAMPLES_US(sampling_freq) (1000000 / (sampling_freq - 1))


/* Event Callback */
static Sensor_Event_t CB_Event = NULL;

/* VSI interrupt handler */
void VSI_Handler (void) {
  uint32_t status;
  uint32_t evt;

  VSI->IRQ.Clear = 1U;
  __DSB();
  __ISB();

  status = VSI->STATUS;

  evt = 0U;

  if (status & SENSOR_STATUS_FIFO_NE_TEMP) {
    evt = SENSOR_EVENT_TEMP_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_HUM) {
    evt = SENSOR_EVENT_HUM_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_PRESS) {
    evt = SENSOR_EVENT_PRESS_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_ACC) {
    evt = SENSOR_EVENT_ACC_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_GYRO) {
    evt = SENSOR_EVENT_GYRO_DATA_AVAILABLE;
  }
  if (status & SENSOR_STATUS_FIFO_NE_MAG) {
    evt = SENSOR_EVENT_MAG_DATA_AVAILABLE;
  }

  if (CB_Event != NULL) {
    CB_Event(evt);
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

//static uint32_t GetTimerInterval {
//}


int32_t Sensor_Initialize (Sensor_Event_t cb_event) {

  CB_Event = cb_event;

  /* Initialize VSI peripheral */
  VSI->Timer.Control = 0U;
  VSI->DMA.Control   = 0U;
  VSI->IRQ.Clear     = 0x00000001U;
  VSI->IRQ.Enable    = 0x00000001U;

  /* Initialize user registers */
  VSI->CTRL = 0U;

  /* Enable VSI interrupts */
  //NVIC_EnableIRQ(VSI_IRQn);
  NVIC->ISER[(((uint32_t)VSI_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)VSI_IRQn) & 0x1FUL));
  __DSB();
  __ISB();

//  Initialized = 1U;

  return 0;//_OK;
}

int32_t Sensor_Uninitialize (void) {

  /* Disable VSI interrupts */
  //NVIC_DisableIRQ(VSI_IRQn);
  NVIC->ICER[(((uint32_t)VSI_IRQn)  >> 5UL)] = (uint32_t)(1UL << (((uint32_t)VSI_IRQn)  & 0x1FUL));
  __DSB();
  __ISB();

  /* De-initialize VSI output */
  VSI->Timer.Control = 0U;
  VSI->DMA.Control   = 0U;
  VSI->IRQ.Clear     = 0x00000001U;
  VSI->IRQ.Enable    = 0x00000000U;
  
  /* Deinitialize user registers */
  VSI->CTRL = 0U;

  CB_Event = NULL;

//  Initialized = 0U;

  return 0; //_OK;
}

int32_t Sensor_Enable (uint32_t type) {
  uint32_t ctrl;
  uint32_t div;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  ctrl = (1UL << type);

  /* Enable sensor */
  VSI->CTRL |= ctrl;

  /* Re-evaluate peripheral clock divider */
  div = VSI->CLK_DIV;

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

  ctrl = VSI->CTRL & ~(1UL << type);

  /* Disable sensor */
  VSI->CTRL = ctrl;

  return SENSOR_OK;
}


int32_t Sensor_MotionReadData (uint32_t type, int32_t *x, int32_t *y, int32_t *z) {
  uint32_t num;

  /* Select requested sensor */
  VSI->SELECT = type;
  
  /* Read number of samples available in FIFO */
  num = VSI->FIFO_CNT;

  if (num > 0U) {
    /* Read FIFO */
    *x = VSI->FIFO_MOTION_X;
    *y = VSI->FIFO_MOTION_Y;
    *z = VSI->FIFO_MOTION_Z;

    /* Decrement number of samples available */
    num--;
  }

  /* Return number of samples available to read */
  return (num);
}


int32_t Sensor_EnvReadData (uint32_t type, int32_t *data) {
  uint32_t num;

  /* Select requested sensor */
  VSI->SELECT = type;
  
  /* Read number of samples available in FIFO */
  num = VSI->FIFO_CNT;

  if (num > 0U) {
    /* Read FIFO */
    *data = VSI->FIFO;

    /* Decrement number of samples available */
    num--;
  }

  /* Return number of samples available to read */
  return (num);
}


int32_t Sensor_QueryScale (uint32_t type, int32_t data[], uint32_t len) {

  if ((IsTypeValid(type) == 0U) || (data == NULL) || (len == 0U)) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current full scale setting */
  data[0] = VSI->SCALE;

  //busy = 0U;

  return (1U);
}


int32_t Sensor_GetScale (uint32_t type, int32_t *data) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current full scale setting */
  *data = VSI->SCALE;

  //busy = 0U;

  return (SENSOR_OK);
}


int32_t Sensor_SetScale (uint32_t type, int32_t data) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* Set full scale setting */
  VSI->SCALE = data;

  //busy = 0U;

  return (SENSOR_OK);
}


int32_t Sensor_QueryDataRate (uint32_t type, uint32_t data[], uint32_t len) {

  if ((IsTypeValid(type) == 0U) || (data == NULL) || (len == 0U)) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* This VSI driver implements single output data rate setting */
  data[0] = VSI->ODR;

  //busy = 0U;

  return (1U);
}


int32_t Sensor_GetDataRate (uint32_t type, uint32_t *rate) {

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* Retrieve current output data rate setting */
  *rate = VSI->ODR;

  //busy = 0U;

  return (SENSOR_OK);
}


int32_t Sensor_SetDataRate (uint32_t type, uint32_t rate) {
  int32_t scale;

  if (IsTypeValid(type) == 0U) {
    return (SENSOR_INVALID_PARAMETER);
  }

  //busy = 1U;

  /* Select sensor */
  VSI->SELECT = type;

  /* Set output data rate setting */
  VSI->ODR = rate;

  //busy = 0U;

  return (SENSOR_OK);
}
