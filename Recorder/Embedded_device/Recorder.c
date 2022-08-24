/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Recorder
 */

#include "Recorder.h"
#include "Recorder_IF.h"
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2

#ifndef REC_THREAD_DELAY
#define REC_THREAD_DELAY    100
#endif
#ifndef REC_IF_FRAME_SZ
#define REC_IF_FRAME_SZ     52
#endif

#ifndef REC_BUFFER_SIZE
#define REC_BUFFER_SIZE     1024        // size must be 2^n
#endif

static uint8_t rec_if_frame_buffer[REC_IF_FRAME_SZ];

static uint8_t  rec_buffer[REC_BUFFER_SIZE];
static volatile uint32_t head;
static volatile uint32_t tail;

static osThreadId_t tid_thr_Recorder;   // Recorder Thread

/* Static helper functions */

/*
  Write data to circular buffer
 */
static int32_t write_buffer (uint8_t * data, uint32_t sz) {
  int32_t err = REC_OK;
  uint32_t i;

  if ((REC_BUFFER_SIZE - (head - tail)) >= sz) {
    // Space is available.
    // Copy the data to the buffer
    for (i = 0; i < sz; i++) {
      rec_buffer[head++ & (REC_BUFFER_SIZE - 1)] = data[i];
    }
  } else {
    // No space available;
    err = REC_ERROR;
  }
  return err;
}

/*
  Read data from circular buffer

  Return: sz: if requested amount of data available
          0:  not enough data available
 */
static uint32_t read_buffer (uint8_t * data, uint32_t sz) {
  uint32_t i, ret_sz = 0;

  if ((head - tail) >= sz) {
    ret_sz = sz;
    for (i = 0; i < sz; i++) {
      data[i] = rec_buffer[tail++ & (REC_BUFFER_SIZE - 1)];
    }
  }

  return ret_sz;
}

/**
  Transfer data from buffer
*/
__NO_RETURN static void thr_Recorder (void *arg) {

  while (1) {
    if (read_buffer(rec_if_frame_buffer, REC_IF_FRAME_SZ) == REC_IF_FRAME_SZ) {
      Recorder_IF_Send(rec_if_frame_buffer, REC_IF_FRAME_SZ);
    }
    osDelay(REC_THREAD_DELAY);
  }
}

/**
  Initialize Recorder
*/
int32_t Recorder_Initialize (const uint8_t * cfg_str, uint32_t len) {
  int32_t err = REC_OK;

  // Initialize interface
  Recorder_IF_Initialize();

  // Send first line (configuration line)
  Recorder_IF_Send(cfg_str, len - 1);

  // Create Recorder thread
  if (err == REC_OK) {
    tid_thr_Recorder = osThreadNew(thr_Recorder, NULL, NULL);
    if (tid_thr_Recorder == NULL) {
       err = REC_ERROR;
    }
  }

  return err;
}

/**
  Write data to Recorder buffer
*/
int32_t Recorder_Write (uint8_t * data, uint32_t sz) {
  return write_buffer(data, sz);
}
