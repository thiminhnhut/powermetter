#include <Arduino.h>
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiSupportESP8266.h>
#include <ControlDigitalOutput.h>
#include <RandomLib.h>

#ifdef PZEM004T
#include <PZEM004T.h>
HardwareSerial hwserial(UART0);
PZEM004T pzem(&hwserial);
IPAddress ip(192, 168, 1, 1);
#endif

LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiSupportESP8266 wifiSupport(&debug_port, LED_BUILTIN, LOW);
ControlDigitalOutput buzzer(PIN_BUZZER, ACTIVE_BUZZER);
RandomLib randomLib;
BlynkTimer timer;

#define PIN_LCD_VIRTUAL V0
#define PIN_CURRENT_SETUP V1
#define PIN_CURRENT_ALRAM V2
// char auth[] = "3319436c3aa0422a99a14802dd51fef7";
char auth[] = "c5185cf2159f472abef8115bb8f4bb25";

WidgetLCD lcdVirtual(PIN_LCD_VIRTUAL);

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
void displayLCDVirtual(PowerMetter powerMetter);

void setup() {

    buzzer.off();

#ifdef PZEM004T
    hwserial.swap();
#endif

    debug_port.begin(BAUD_RATE);

#ifdef PZEM004T
    debug_port.println("Define PZEM004T");
#else
    debug_port.println("Not define PZEM004T");
#endif

#ifdef PZEM004T
    // Connect PZEM module
    bool pzemrdy = false;
    while (!pzemrdy) {
        debug_port.println("Connecting to PZEM...");
        pzemrdy = pzem.setAddress(ip);
        delay(1000);
    }
#endif

    // Wait for connect wifi wifi smart config
    if (!wifiSupport.isSmartConfig(TIMEOUT)) {
        ESP.restart();
    }

    lcd.begin();
    lcd.backlight();
    lcd.clear();

    // Connect Blynk server
    Blynk.config(auth);

    // clear lcdVirtual
    lcdVirtual.clear();

    timer.setInterval(TIME_UPDATE_DATA, updatePowerMetter);
}

void loop() {
    Blynk.run();
    timer.run();
}

#ifdef PZEM004T
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
#else
PowerMetter getPowerMetter() {
    PowerMetter powerMetter;

    powerMetter.voltage = randomLib.getNumberFloat(210.0, 250.0);

    powerMetter.current = randomLib.getNumberFloat(0.0, 5.0);

    powerMetter.power = randomLib.getNumberFloat(0.0, 1000.0);

    powerMetter.energy = randomLib.getNumberFloat(0.0, 1000.0);

    return powerMetter;
}
#endif

void updatePowerMetter() {
    PowerMetter powerMetter = getPowerMetter();

    if (!checkCurrentValid(powerMetter.current)) {
        buzzer.on();
        updateBuzzerAlarm(true);
    } else {
        buzzer.off();
        updateBuzzerAlarm(false);
    }

    displayLCDVirtual(powerMetter);
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

void displayLCDVirtual(PowerMetter powerMetter) {
    String line1 = String(powerMetter.current) + "A-" + String(powerMetter.voltage) + "V";
    String line2 = String(powerMetter.power) + "J-" + String(powerMetter.energy) + "W";

    lcdVirtual.clear();
    lcdVirtual.print(0, 0, line1);
    lcdVirtual.print(0, 1, line2);
}

void displayLCD(PowerMetter powerMetter) {
    String line1 = String(powerMetter.current) + "A-" + String(powerMetter.voltage) + "V";
    String line2 = String(powerMetter.power) + "J-" + String(powerMetter.energy) + "W";

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
    debug_port.println(current);
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
