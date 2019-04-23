#include <Arduino.h>
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiSupportESP8266.h>
#include <ControlDigitalOutput.h>
#include <RandomLib.h>

#include <PZEM004T.h>
PZEM004T pzem(&Serial);
IPAddress ip(192,168,1,1);

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiSupportESP8266 wifiSupport(LED_BUILTIN, LOW);
ControlDigitalOutput buzzer(PIN_BUZZER, ACTIVE_BUZZER);
RandomLib randomLib;
BlynkTimer timer;

#define PIN_GAUGE_CURRENT V0
#define PIN_GAUGE_VOLTAGE V3
#define PIN_CURRENT_SETUP V1
#define PIN_CURRENT_ALRAM V2
// char auth[] = "3319436c3aa0422a99a14802dd51fef7";
char auth[] = "c5185cf2159f472abef8115bb8f4bb25";

struct PowerMetter {
    float voltage;
    float current;
    float power;
    float energy;
};

PowerMetter getPowerMetter();
void updatePowerMetter();

float maxCurrent;
void setMaxCurrent(float _current);
float getMaxCurrent();
bool checkCurrentValid(float _current);

void updateBuzzerAlarm(bool isAlarm);

void displayLCD(PowerMetter powerMetter);
void displayGaugeVirtual(PowerMetter powerMetter);

void setup() {
    buzzer.off();

    lcd.begin();
    lcd.backlight();
    lcd.clear();

    lcd.print("Init PZEM");
    pzem.setAddress(ip);

    lcd.clear();
    lcd.print("Connect Wifi");
    // Wait for connect wifi wifi smart config
    if (!wifiSupport.isSmartConfig(TIMEOUT)) {
        ESP.restart();
    }
    // wifiSupport.isConnected("Thi Nhut", "thiminhnhut", TIMEOUT);

    lcd.clear();
    lcd.print("Connect Blynk");

    // Connect Blynk server
    Blynk.config(auth);

    timer.setInterval(TIME_UPDATE_DATA, updatePowerMetter);

    lcd.print("Finish");
    lcd.clear();
}

void loop() {
    Blynk.run();
    timer.run();
}

PowerMetter getPowerMetter() {
    PowerMetter powerMetter;

    powerMetter.voltage = pzem.voltage(ip);
    if (powerMetter.voltage < 0.0) {
        powerMetter.voltage = 0.0;
    }

    powerMetter.current = pzem.current(ip);
    if (powerMetter.current < 0.0) {
        powerMetter.current = 0.0;
    }

    powerMetter.power = pzem.power(ip);
    if (powerMetter.power < 0.0) {
        powerMetter.power = 0.0;
    }

    powerMetter.energy = pzem.energy(ip);
    if (powerMetter.energy < 0.0) {
        powerMetter.energy = 0.0;
    }

    return powerMetter;
}

void updatePowerMetter() {
    PowerMetter powerMetter = getPowerMetter();

    if (!checkCurrentValid(powerMetter.current)) {
        buzzer.on();
        updateBuzzerAlarm(true);
    } else {
        buzzer.off();
        updateBuzzerAlarm(false);
    }

    displayGaugeVirtual(powerMetter);
    displayLCD(powerMetter);
}

void setMaxCurrent(float _current) {
    maxCurrent = _current;
}

float getMaxCurrent() {
    return maxCurrent;
}

bool checkCurrentValid(float _current) {
    return _current <= getMaxCurrent();
}

void updateBuzzerAlarm(bool isAlarm) {
    Blynk.virtualWrite(PIN_CURRENT_ALRAM, isAlarm);
}

void displayGaugeVirtual(PowerMetter powerMetter) {
    Blynk.virtualWrite(PIN_GAUGE_CURRENT, powerMetter.current);
    Blynk.virtualWrite(PIN_GAUGE_VOLTAGE, powerMetter.voltage);
}

void displayLCD(PowerMetter powerMetter) {
    String line1 = "Dien ap:" + String(powerMetter.voltage) + "V";
    String line2 = "Dong dien:" + String(powerMetter.current) + "A";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

BLYNK_WRITE(PIN_CURRENT_SETUP) {
    float current = param.asFloat();
    // debug_port.println(current);
    setMaxCurrent(current);
}

BLYNK_WRITE(PIN_CURRENT_ALRAM) {
    bool isAlarm = param.asInt();
    if (isAlarm) {
        buzzer.on();
    } else {
        buzzer.off();
    }
}
