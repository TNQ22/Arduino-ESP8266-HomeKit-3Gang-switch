/*
    Created on: 2020-11-19
        Author: SYNC_BYTE
*/

#include <Arduino.h>
#include "wifi_info.h"
#include "ESPButton.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define BUTTON_01 5//D1
#define BUTTON_02 4//D2
#define BUTTON_03 0//D3

#define LIGHT_01 2//D4
#define LIGHT_02 14//D5
#define LIGHT_03 12//D6

void setup() {
  Serial.begin(115200);
  init_lights_and_buttons();
  wifi_connect(); // in wifi_info.h
  my_homekit_setup();
}

void loop() {
  my_homekit_loop();
  ESPButton.loop();
}


// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;

extern "C" homekit_characteristic_t cha_switch_on1;
extern "C" homekit_characteristic_t cha_switch_on2;
extern "C" homekit_characteristic_t cha_switch_on3;

//Called when the switch value is changed by iOS Home APP
void cha_switch_on1_setter(const homekit_value_t value) {
  cha_switch_on1.value.bool_value = value.bool_value;  //sync the value
  LOG_D("Switch1: %s", value.bool_value ? "ON" : "OFF");
  digitalWrite(LIGHT_01, value.bool_value);
}

void cha_switch_on2_setter(const homekit_value_t value) {
  cha_switch_on2.value.bool_value = value.bool_value;  //sync the value
  LOG_D("Switch2: %s", value.bool_value ? "ON" : "OFF");
  digitalWrite(LIGHT_02, value.bool_value);
}

void cha_switch_on3_setter(const homekit_value_t value) {
  cha_switch_on3.value.bool_value = value.bool_value;  //sync the value
  LOG_D("Switch3: %s", value.bool_value ? "ON" : "OFF");
  digitalWrite(LIGHT_03, value.bool_value);
}

void button_pressed(uint8_t id, ESPButtonEvent event) {
  LOG_D("Button %d Event: %s", id, ESPButton.getButtonEventDescription(event));

  //report the switch value to HomeKit if it is changed (e.g. by a physical button)
  if (event == ESPBUTTONEVENT_SINGLECLICK) {
    invert_light(id);
  } else if (event == ESPBUTTONEVENT_LONGCLICK) {
    //any button longpressed will launch the device setup access point
    wifi_setup_ap();
  }
}

void invert_light(int button) {
  switch (button) {
    case 1:
      cha_switch_on1.value.bool_value = !cha_switch_on1.value.bool_value;
      digitalWrite(LIGHT_01, cha_switch_on1.value.bool_value);
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
      break;
    case 2:
      cha_switch_on2.value.bool_value = !cha_switch_on2.value.bool_value;
      digitalWrite(LIGHT_02, cha_switch_on2.value.bool_value);
      homekit_characteristic_notify(&cha_switch_on2, cha_switch_on2.value);
      break;
    case 3:
      cha_switch_on3.value.bool_value = !cha_switch_on3.value.bool_value;
      digitalWrite(LIGHT_03, cha_switch_on3.value.bool_value);
      homekit_characteristic_notify(&cha_switch_on3, cha_switch_on3.value);
      break;
  }
}

void init_lights_and_buttons() {
  pinMode(LIGHT_01, OUTPUT);
  pinMode(LIGHT_02, OUTPUT);
  pinMode(LIGHT_03, OUTPUT);
  digitalWrite(LIGHT_01, LOW);
  digitalWrite(LIGHT_02, LOW);
  digitalWrite(LIGHT_03, LOW);

  pinMode(BUTTON_01, INPUT_PULLUP);
  pinMode(BUTTON_02, INPUT_PULLUP);
  pinMode(BUTTON_03, INPUT_PULLUP);
  ESPButton.add(1, BUTTON_01, LOW, false, true);
  ESPButton.add(2, BUTTON_02, LOW, false, true);
  ESPButton.add(3, BUTTON_03, LOW, false, true);

  ESPButton.setCallback(button_pressed);
  ESPButton.begin();
}

extern "C" void identify_switch_1(homekit_value_t _value) {
  invert_light(1);
  printf("identify switch 1\n");
}

extern "C" void identify_switch_2(homekit_value_t _value) {
  invert_light(2);
  printf("identify switch 2\n");
}

extern "C" void identify_switch_3(homekit_value_t _value) {
  invert_light(3);
  printf("identify switch 3\n");
}

extern "C" void identify_accessory(homekit_value_t _value) {
  invert_light(1);
  invert_light(2);
  invert_light(3);
  printf("identify accessory\n");
}

void my_homekit_setup() {
  //Add the .setter function to get the switch-event sent from iOS Home APP.
  cha_switch_on1.setter = cha_switch_on1_setter;
  cha_switch_on2.setter = cha_switch_on2_setter;
  cha_switch_on3.setter = cha_switch_on3_setter;
  arduino_homekit_setup(&config);
}

static uint32_t next_heap_millis = 0;
void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}