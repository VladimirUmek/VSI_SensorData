/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#include <stdint.h>
#include <stddef.h>

/* VSI Timer Control Definitions for Timer.Control register */

#define SID_TEMP  0
#define SID_HUM   1
#define SID_PRESS 2
#define SID_ACC   3
#define SID_GYRO  4
#define SID_MAG   5

#define SENSOR_STATUS_FIFO_NE_TEMP    (1 << SID_TEMP )
#define SENSOR_STATUS_FIFO_NE_HUM     (1 << SID_HUM  )
#define SENSOR_STATUS_FIFO_NE_PRESS   (1 << SID_PRESS)
#define SENSOR_STATUS_FIFO_NE_ACC     (1 << SID_ACC  )
#define SENSOR_STATUS_FIFO_NE_GYRO    (1 << SID_GYRO )
#define SENSOR_STATUS_FIFO_NE_MAG     (1 << SID_MAG  )