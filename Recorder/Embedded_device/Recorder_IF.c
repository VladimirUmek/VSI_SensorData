/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Recorder
 */

#include "Recorder.h"
#include "Recorder_IF.h"

#include "Driver_WiFi.h"
#include "iot_socket.h"

/* Configuration */
#define SSID        ""
#define PASS        ""
#define SECURITY    ARM_WIFI_SECURITY_WPA2

#define IP   {192, 168, 1, 59}
#define PORT  5000

extern ARM_DRIVER_WIFI Driver_WiFi0;
static ARM_DRIVER_WIFI * wifi = &Driver_WiFi0;

static int32_t socket;

/**
  Send data 

  Transfer interface is IotSocket (could be something else)
*/
int32_t Recorder_IF_Send (const uint8_t * data, uint32_t sz) {
  int32_t err;
  uint8_t ip4[4] = IP;

  if (iotSocketSendTo(socket, data, sz, ip4, 4, PORT) >= 0) {
    err = REC_OK;
  }

  return err;
}

/**
  Initialize transfer interface 

  Transfer interface is IotSocket (could be something else)
*/
int32_t Recorder_IF_Initialize (void) {
  int32_t err;

  // WiFi initialize and connect to AP (wifi could be placed in main)
  ARM_WIFI_CONFIG_t config = {
                               SSID,
                               PASS,
                               SECURITY,
                               0U, 0U, 0U, 0
                             };
  wifi->Initialize(NULL);
  wifi->PowerControl(ARM_POWER_FULL);
  wifi->Activate (0, &config);
  while (wifi->IsConnected() == 0);

  // UDP Socket
  socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);

  // TCP Socket
  //socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_STREAM, IOT_SOCKET_IPPROTO_TCP);

  if (socket < 0) {
    err = REC_ERROR;
  }
//  if (err == 0) {
//    iotSocketConnect(socket, ip4, 4, PORT);
//  }
  
  return err;
}

