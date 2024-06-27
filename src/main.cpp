#include "Pinout.h"
#include "PowerSwitcher.h"
#include <WiFi.h>

String serialInputBuffer = "";
PowerSwitcher powerSwitcher;

void lightSleep(uint32_t time_in_ms)
{
  esp_sleep_enable_timer_wakeup(time_in_ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

bool lastPowerState = false;
void turnOutputPower(bool val)
{
  if (lastPowerState != val)
  {
    gpio_hold_dis(GPIO_NUM_10);
    digitalWrite(FED_ENABLE_PIN, val);
    gpio_hold_en(GPIO_NUM_10);
  }
  lastPowerState = val;
}

bool hasIgnition()
{
  return digitalRead(IGNITION_INPUT_PIN);
}

uint32_t readBatteryVoltageMilliVolts()
{
  uint64_t adc_mv = 0;
  for (int i = 0; i < 1000; i++)
  {
    adc_mv += analogReadMilliVolts(SUPPLY_ADC_PIN);
  }
  adc_mv = adc_mv / 1000;
  uint32_t divider = 11;
  return adc_mv * divider;
}

void processCommand(String cmd)
{
  if (cmd.startsWith("help()"))
  {
    Serial.println("help()");
    Serial.println("reset()");
    Serial.println("adc()");
    delay(2000);
  }
  if (cmd.startsWith("reset()"))
  {
    Serial.println("System restart by user.");
    delay(2000);
    ESP.restart();
  }
  if (cmd.startsWith("adc()"))
  {
  Serial.printf(">ADC_mv:%d\n", readBatteryVoltageMilliVolts());
  }
  if (cmd.startsWith("on()"))
  {
    turnOutputPower(1);
  }
  if (cmd.startsWith("off()"))
  {
    turnOutputPower(0);
  }
  if (cmd.startsWith("sleep()"))
  {
    lightSleep(2000);
  }
  if (cmd.startsWith("xxx"))
  {
    Serial.printf("freq: %d\n", getCpuFrequencyMhz());
  }
}

void readSerialInput()
{
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\r' || c == '\n')
    {
      processCommand(serialInputBuffer);
      serialInputBuffer = "";
    }
    else if (c == 0)
    { // Do not handle binary data, just ASCII. Some old java code sends null at the end of the message.
      continue;
    }
    else
    {
      serialInputBuffer += c;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(FED_ENABLE_PIN, OUTPUT);
  digitalWrite(FED_ENABLE_PIN, HIGH);
  pinMode(IGNITION_INPUT_PIN, INPUT);
  pinMode(SUPPLY_ADC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // WiFi.disconnect(true); // Disconnect from the network
  // WiFi.mode(WIFI_OFF);   // Switch WiFi off
  // setCpuFrequencyMhz(80);
}

void loop()
{
  readSerialInput();
  delay(1);
  /////////////////////////////////////////////////////////////////////

  powerSwitcher.updateIgnitionState(hasIgnition());
  powerSwitcher.updateBatteryVoltage(readBatteryVoltageMilliVolts());

  bool state = powerSwitcher.calculatePowerState();
  turnOutputPower(state);

  lightSleep(500);


  // digitalWrite(LED_PIN, LOW);
  // delay(20);
  // digitalWrite(LED_PIN, HIGH);
  // delay(20);

  // if(Serial.availableForWrite())
  // {
  //   //delay(1);
  //   // Serial.printf(">ADC:%d\n", adc);
  // }
}
