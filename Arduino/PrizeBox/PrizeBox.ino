#include <Arduino.h>

#include <MPR121.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Pin definitions.

const uint8_t boardSelectA = D19;
const uint8_t boardSelectB = D18;

const uint8_t interruptPin = D6;

const uint8_t mosfetAPin = D3;
const uint8_t mosfetBPin = D2;
const uint8_t mosfetCPin = D1;
const uint8_t mosfetDPin = D0;

const uint8_t buzzerPin = D22;

// Data types.

const byte SYSEX_HEADER = 0xf0;
const byte SYSEX_VENDOR = 0x7d;
const byte SYSEX_FOOTER = 0xf7;

enum MessageType
{
    KEY_PRESS_0 = 0x00,
    KEY_PRESS_1 = 0x01,
    KEY_PRESS_2 = 0x02,
    KEY_PRESS_3 = 0x03,
    KEY_PRESS_4 = 0x04,
    KEY_PRESS_5 = 0x05,
    KEY_PRESS_6 = 0x06,
    KEY_PRESS_7 = 0x07,
    KEY_PRESS_8 = 0x08,
    KEY_PRESS_9 = 0x09,

    KEY_PRESS_CANCEL = 0x0a,
    KEY_PRESS_ENTER = 0x0b,

    SET_FAILURE = 0x20,
    SET_STANDBY = 0x30,
    SET_SUCCESS = 0x40,

    TRIGGER_BUZZER = 0x50,
    TRIGGER_UNLOCK = 0x60,

    TRIGGER_RESET = 0x70,
};

int keymap[12] = {
    KEY_PRESS_1,
    KEY_PRESS_2,
    KEY_PRESS_3,
    KEY_PRESS_4,
    KEY_PRESS_5,
    KEY_PRESS_6,
    KEY_PRESS_7,
    KEY_PRESS_8,
    KEY_PRESS_9,
    KEY_PRESS_CANCEL,
    KEY_PRESS_0,
    KEY_PRESS_ENTER};

// MPR-121 Configuration.

TwoWire *wire_ptr = &Wire;
MPR121 mpr121;

const MPR121::DeviceAddress device_address = MPR121::ADDRESS_5A;

const bool fast_mode = true;

const uint8_t touch_threshold = 40;
const uint8_t release_threshold = 30;

const uint8_t touch_debounce = 1;
const uint8_t release_debounce = 1;

const MPR121::SamplePeriod sample_period = MPR121::SAMPLE_PERIOD_8MS;
const MPR121::BaselineTracking baseline_tracking = MPR121::BASELINE_TRACKING_INIT_10BIT;

const MPR121::ProximityMode proximity_mode = MPR121::PROXIMITY_MODE_DISABLED;

const uint8_t charge_discharge_current = 32;
const MPR121::ChargeDischargeTime charge_discharge_time = MPR121::CHARGE_DISCHARGE_TIME_1US;
const MPR121::FirstFilterIterations first_filter_iterations = MPR121::FIRST_FILTER_ITERATIONS_34;
const MPR121::SecondFilterIterations second_filter_iterations = MPR121::SECOND_FILTER_ITERATIONS_10;

// MIDI setup.

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// Global state.

uint16_t lastStatus = 0;
unsigned long lastBeepTime = 0;
unsigned long lastBeepDuration = 0;

void resetSensor()
{
    // TODO: MPR-121 register reset.
    // mpr121.writeRegister(0x80, 0x63);
    // delay(1);

    mpr121.setupSingleDevice(*wire_ptr,
                             device_address,
                             fast_mode);

    mpr121.setAllChannelsThresholds(touch_threshold,
                                    release_threshold);
    mpr121.setDebounce(device_address,
                       touch_debounce,
                       release_debounce);
    mpr121.setBaselineTracking(device_address,
                               baseline_tracking);
    mpr121.setChargeDischargeCurrent(device_address,
                                     charge_discharge_current);
    mpr121.setChargeDischargeTime(device_address,
                                  charge_discharge_time);
    mpr121.setFirstFilterIterations(device_address,
                                    first_filter_iterations);
    mpr121.setSecondFilterIterations(device_address,
                                     second_filter_iterations);
    mpr121.setSamplePeriod(device_address,
                           sample_period);

    mpr121.startAllChannels();
}

void triggerBuzzer(unsigned long duration)
{
    lastBeepTime = millis();
    lastBeepDuration = duration;
}

void handleSystemExclusive(byte *array, unsigned size)
{
    MessageType type = (MessageType)array[2];
    uint8_t value = array[3];

    switch (type)
    {
    case SET_FAILURE:
        digitalWrite(mosfetAPin, value);
        break;
    case SET_SUCCESS:
        digitalWrite(mosfetBPin, value);
        break;
    case SET_STANDBY:
        digitalWrite(mosfetCPin, value);
        break;
    case TRIGGER_BUZZER:
        triggerBuzzer(value * 10);
        break;

    case TRIGGER_UNLOCK:
        digitalWrite(mosfetDPin, HIGH);
        delay(100);
        digitalWrite(mosfetDPin, LOW);
        break;

    case TRIGGER_RESET:
        digitalWrite(mosfetAPin, HIGH);
        digitalWrite(mosfetBPin, HIGH);
        digitalWrite(mosfetCPin, HIGH);
        digitalWrite(mosfetDPin, HIGH);

        resetSensor();

        delay(1000);

        digitalWrite(mosfetAPin, LOW);
        digitalWrite(mosfetBPin, LOW);
        digitalWrite(mosfetCPin, LOW);
        digitalWrite(mosfetDPin, LOW);
        break;
    };
}

void setup()
{
    Serial.begin(115200);

    // Initialize USB.
    {
        int boardIndex = (digitalRead(boardSelectA) << 1) | digitalRead(boardSelectB);

        String productDescriptor = "CodeTV Board " + String(boardIndex);
        String serialDescriptor = "SN0000" + String(boardIndex);

        TinyUSBDevice.setManufacturerDescriptor("High Order");
        TinyUSBDevice.setProductDescriptor(productDescriptor.c_str());
        TinyUSBDevice.setSerialDescriptor(serialDescriptor.c_str());

        if (!TinyUSBDevice.isInitialized())
        {
            TinyUSBDevice.begin(0);
        }
    }

    // Pin configuration.
    {
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);

        pinMode(mosfetAPin, OUTPUT);
        digitalWrite(mosfetAPin, LOW);

        pinMode(mosfetBPin, OUTPUT);
        digitalWrite(mosfetBPin, LOW);

        pinMode(mosfetCPin, OUTPUT);
        digitalWrite(mosfetCPin, LOW);

        pinMode(mosfetDPin, OUTPUT);
        digitalWrite(mosfetDPin, LOW);

        pinMode(interruptPin, INPUT_PULLUP);
    }

    // Initialize MIDI.
    {
        MIDI.begin(MIDI_CHANNEL_OMNI);

        if (TinyUSBDevice.mounted())
        {
            TinyUSBDevice.detach();
            delay(10);
            TinyUSBDevice.attach();
        }

        MIDI.setHandleSystemExclusive(handleSystemExclusive);
    }

    // MPR121 setup.
    {
        resetSensor();
    }
}

void loop()
{
    // Wait for USB mount.
    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    // Read MIDI messages.
    MIDI.read();

    // Handle MPR-121 input.
    if (digitalRead(interruptPin) == LOW)
    {
        uint16_t currentStatus = mpr121.getTouchStatus(device_address);

        for (uint8_t i = 0; i < 12; i++)
        {
            bool wasTouched = lastStatus & (1 << i);
            bool isTouched = currentStatus & (1 << i);

            uint8_t key = keymap[i];

            if (!wasTouched && isTouched)
            {
                uint8_t msg[] = {SYSEX_HEADER, SYSEX_VENDOR, key, 1, SYSEX_FOOTER};
                MIDI.sendSysEx(sizeof(msg), msg, true);

                triggerBuzzer(50);
            }
            else if (wasTouched && !isTouched)
            {
                uint8_t msg[] = {SYSEX_HEADER, SYSEX_VENDOR, key, 0, SYSEX_FOOTER};
                MIDI.sendSysEx(sizeof(msg), msg, true);
            }
        }

        lastStatus = currentStatus;
    }

    // Handle beeps.
    if (millis() - lastBeepTime < lastBeepDuration)
    {
        digitalWrite(buzzerPin, HIGH);
    }
    else
    {
        digitalWrite(buzzerPin, LOW);
        lastBeepDuration = 0;
    }
}
