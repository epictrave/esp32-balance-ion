// Copyright (c) Janghun LEE. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef BALANCE_ION_H
#define BALANCE_ION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_log.h"
#include "stdlib.h"
#include "time.h"

#include "device_twin_state.h"
#include "parson.h"
#include "queue_message.h"

typedef enum { CALI_MODE, MEASURE_MODE, UNKNOWN_MODE } ION_MODE;

typedef struct ION_TAG {
  adc1_channel_t adc_channel;
  int cali_raw;
  double cali_concentration;
  int raw;
  double concentration;
} Ion;

esp_err_t balance_ion_init(adc1_channel_t adc_channels[], size_t length);
void balance_ion_check_ion(void);
void balance_ion_add_message(time_t _time);

void balance_ion_check_parse_from_json(const char *json,
                                       DEVICE_TWIN_STATE update_state);

esp_err_t balance_ion_set_power(bool is_on);
bool balance_ion_get_power(void);

#ifdef __cplusplus
}
#endif

#endif /* BALANCE_ION_H */