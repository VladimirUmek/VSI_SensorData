/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef RECORDER_IF_H__
#define RECORDER_IF_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
  \fn          int32_t Recorder_IF_Initialize (void)
  \brief       Initialize Recorder Interface
  \return      Return code
*/
int32_t Recorder_IF_Initialize (void);

/**
  \fn          int32_t Recorder_IF_Send (cosnt uint8_t * data, uint32_t sz)
  \brief       Send data
  \return      Return code
*/
int32_t Recorder_IF_Send (const uint8_t * data, uint32_t sz);


#ifdef  __cplusplus
}
#endif

#endif /* RECORDER_IF_H__ */