/**
 * @file WifiConnect.h
 * @author Gustice
 * @brief WiFi Module that can either setup an access-point or connect to an access-point.
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"
#include "tcpip_adapter.h"
#include "ParamRepo.h"

  typedef enum
  {
    /// Creates own Network, to enable AdHoc setup-connection
    InitAsAp,
    // Connects to an Access-Point
    ConnectToAp,
  } eWifiType;

  /**
 * @brief Setup WiFi Connection
 * 
 * @param connect Defines mode of station
 * @param config Defines Wifi-Configuration
 * @return esp_err_t 
 */
  esp_err_t Wifi_Connect(eWifiType connect, WifiConfig_t *config);

#ifdef __cplusplus
}
#endif
