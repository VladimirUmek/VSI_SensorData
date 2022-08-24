/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef RECORDER_H__
#define RECORDER_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* Return Codes */
#define REC_OK                         (0)  ///< Operation succeeded
#define REC_ERROR                      (-1) ///< Unspecified error

/**
  \fn          int32_t Recorder_Initialize (const uint8_t * cfg_str, uint32_t len)
  \brief       Initialize Recorder
  \param[in]   cfg_str configuration string
  \param[in]   len     string length
  \return      Return code
*/
int32_t Recorder_Initialize (const uint8_t * cfg_str, uint32_t len);

/**
  \fn          int32_t Recorder_Write (uint8_t * data, uint32_t sz)
  \brief       Send data via recorder
  \return      Return code
*/
int32_t Recorder_Write (uint8_t * data, uint32_t sz);


#ifdef  __cplusplus
}
#endif

#endif /* RECORDER_H__ */