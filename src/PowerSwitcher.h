#ifndef SRC_POWERSWITCHER
#define SRC_POWERSWITCHER

#include "Arduino.h"

class PowerSwitcher{
public:
    PowerSwitcher()
    {
    }

    void updateIgnitionState(bool ignition)
    {
        if (ignition)
        {
            lastChangeTime = millis();
            output = true;
        }
    }

    void updateBatteryVoltage(uint32_t voltageMillivolts) {
        if ((voltageMillivolts + VOLTAGE_DROP_THRESHOLD_MILLIVOLTS) < lastVoltageMillivolts)
        {
            lastChangeTime = millis();
            output = true;
        }
        lastVoltageMillivolts = voltageMillivolts;
    }

    bool calculatePowerState()
    {
        bool calculatedOutput = false;

        if (output) {
            calculatedOutput = (lastChangeTime + MAX_ON_TIME) > millis();
        } else {
            calculatedOutput = millis() > lastChangeTime + MAX_OFF_TIME;
        }

        if(output != calculatedOutput) {
            lastChangeTime = millis();
            output = calculatedOutput;
        }
        return output;
    }

private:
    bool output = true;
    unsigned long lastChangeTime;
    uint32_t lastVoltageMillivolts;

    const unsigned long MAX_ON_TIME = 2 * 1000;
    const unsigned long MAX_OFF_TIME = 5 * 1000;
    const uint32_t VOLTAGE_DROP_THRESHOLD_MILLIVOLTS = 100;
};

#endif    /* SRC_POWERSWITCHER */
