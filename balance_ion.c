// Copyright (c) Janghun LEE. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "balance_ion.h"

static const char *TAG = "balance ion";
static Ion *ions = NULL;
static size_t ion_num = 0;
static ION_MODE mode = UNKNOWN_MODE;
static bool power = false;

esp_err_t balance_ion_set_cali_raw(int index, int cali_raw);
esp_err_t balance_ion_set_cali_concentration(int index,
                                             double cali_concentration);
esp_err_t balance_ion_set_raw(int index, int raw);
esp_err_t balance_ion_set_concentration(int index, double concentration);

esp_err_t balance_ion_init(adc1_channel_t adc_channels[], size_t length) {
  if (length == 0) {
    return ESP_ERR_INVALID_ARG;
  }
  if (ions != NULL) {
    free(ions);
  }
  ion_num = length;
  ions = (Ion *)malloc(sizeof(Ion) * ion_num);
  for (int i = 0; i < ion_num; i++) {
    ions[i].adc_channel = adc_channels[i];
    balance_ion_set_cali_raw(i, -1);
    balance_ion_set_cali_concentration(i, -1);
    balance_ion_set_raw(i, -1);
    balance_ion_set_concentration(i, -1);
    adc1_config_channel_atten(ions[i].adc_channel, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "ion #%d is initalized.", i);
  }
  adc1_config_width(ADC_WIDTH_BIT_12);
  return ESP_OK;
}

esp_err_t balance_ion_set_cali_raw(int index, int cali_raw) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return ESP_ERR_INVALID_ARG;
  }
  ions[index].cali_raw = cali_raw;
  return ESP_OK;
}

int balance_ion_get_cali_raw(int index) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return -1;
  }
  return ions[index].cali_raw;
}

esp_err_t balance_ion_set_cali_concentration(int index,
                                             double cali_concentration) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return ESP_ERR_INVALID_ARG;
  }
  ions[index].cali_concentration = cali_concentration;
  return ESP_OK;
}

double balance_ion_get_cali_concentration(int index) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return -1;
  }
  return ions[index].cali_concentration;
}

esp_err_t balance_ion_set_raw(int index, int raw) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return ESP_ERR_INVALID_ARG;
  }
  ions[index].raw = raw;
  return ESP_OK;
}

int balance_ion_get_raw(int index) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return -1;
  }
  return ions[index].raw;
}

esp_err_t balance_ion_set_concentration(int index, double concentration) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return ESP_ERR_INVALID_ARG;
  }
  // TODO 내부 로직 추가해야함
  ions[index].concentration = concentration;
  return ESP_OK;
}

double balance_ion_get_concentration(int index) {
  if (index < 0 || ion_num <= index) {
    ESP_LOGE(TAG, "Index is Invaild. size : %d index: %d", ion_num, index);
    return -1;
  }
  // TODO 내부 로직 추가해야함
  return ions[index].concentration;
}

void balance_ion_check_ion(void) {
  ESP_LOGI(TAG, "mode : %s.",
           mode == CALI_MODE ? "calibration"
                             : mode == MEASURE_MODE ? "measure" : "unknown");

  for (int i = 0; i < ion_num; i++) {
    if (mode == CALI_MODE) {
      ions[i].cali_raw = adc1_get_raw(ions[i].adc_channel);
      ESP_LOGI(TAG, "#%d raw : %d cali concentration %f.", i, ions[i].cali_raw,
               ions[i].cali_concentration);
    } else if (mode == MEASURE_MODE) {
      ions[i].raw = adc1_get_raw(ions[i].adc_channel);
      ESP_LOGI(TAG, "#%d raw : %d.", i, ions[i].raw);
    }
  }
}
void balance_ion_add_message(time_t _time) {
  if (mode == UNKNOWN_MODE)
    return;
  char number[3];
  struct tm *time_info;
  time_info = localtime(&_time);
  char format_time[40];
  strftime(format_time, 40, "%FT%TZ", time_info);

  for (int i = 0; i < ion_num; i++) {
    sprintf(number, "%d", i);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "messageType", "ion");
    json_object_set_string(root_object, "time", format_time);
    json_object_set_string(root_object, "sensorID", number);
    json_object_set_string(root_object, "messageType", "ion");
    json_object_set_number(root_object, "mode", mode);
    if (mode == CALI_MODE) {
      json_object_set_number(root_object, "raw", balance_ion_get_cali_raw(i));
      json_object_set_number(root_object, "concentration",
                             balance_ion_get_cali_concentration(i));
    } else if (mode == MEASURE_MODE) {
      json_object_set_number(root_object, "raw", balance_ion_get_raw(i));
      json_object_set_number(root_object, "concentration",
                             balance_ion_get_concentration(i));
    }
    char *result = json_serialize_to_string(root_value);
    queue_message_add_message(result);
    json_value_free(root_value);
    free(result);
  }
}
void balance_ion_check_parse_from_json(const char *json,
                                       DEVICE_TWIN_STATE update_state) {
  JSON_Value *root_value = json_parse_string(json);
  JSON_Object *root_object = json_value_get_object(root_value);
  char name[50], number[10];
  if (update_state == UPDATE_PARTIAL) {
    sprintf(name, "ion");
  } else if (update_state == UPDATE_COMPLETE) {
    sprintf(name, "desired.ion");
  }
  JSON_Object *json_object_ion = json_object_dotget_object(root_object, name);
  if (json_object_get_value(json_object_ion, "mode") != NULL) {
    ION_MODE new_mode = json_object_get_number(json_object_ion, "mode");
    if (new_mode != mode) {
      mode = new_mode;
      // TODO 추가해라 그 뭐냐 그거 있잖아 메세지 보내는거
    }
  }
  if (json_object_get_value(json_object_ion, "power") != NULL) {
    bool new_power = json_object_get_boolean(json_object_ion, "power");
    if (new_power != power) {
      power = new_power;
      // TODO 추가해라 그 뭐냐 그거 있잖아 메세지 보내는거
    }
  }
  for (int i = 0; i < ion_num; i++) {
    memset(number, 0, sizeof(number));
    sprintf(number, "%d", i);
    if (json_object_get_value(json_object_ion, number) != NULL) {
      ions[i].cali_concentration =
          json_object_get_number(json_object_ion, number);
    }
  }
  json_value_free(root_value);
}

esp_err_t balance_ion_set_power(bool is_on) {
  power = is_on;
  return ESP_OK;
}

bool balance_ion_get_power(void) { return power; }