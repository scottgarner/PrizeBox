#include <Arduino.h>

#include <MPR121.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Pin definitions.

const uint8_t boardSelectA = D19;
const uint8_t boardSelectB = D18;

const uint8_t interruptPin = D6;

const uint8_t mosfettiPinA = D3;
const uint8_t mosfettiPinB = D2;
const uint8_t mosfettiPinC = D1;
const uint8_t mosfettiPinD = D0;

const uint8_t buzzerPin = D22;

// Configuration.

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

// Data types.

enum MessageType
{
    KEY_PRESS = 0x01,
    SET_LIGHT_STATE = 0x02,
    TRIGGER_UNLOCK = 0x03,
    TRIGGER_BUZZER = 0x04,
};

// MIDI setup.

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// MPR121 setup.

TwoWire *wire_ptr = &Wire;

MPR121 mpr121;

// Globals.

uint16_t lastStatus = 0;

void handleSystemExclusive(byte *array, unsigned size)
{

    MessageType type = (MessageType)array[2];
    uint8_t channel = array[3];
    uint8_t value = array[4];

    if (type == SET_LIGHT_STATE)
    {
        switch (channel)
        {
        case 0:
            digitalWrite(mosfettiPinA, value);
            break;
        case 1:
            digitalWrite(mosfettiPinB, value);
            break;
        case 2:
            digitalWrite(mosfettiPinC, value);
            break;
        }
    }

    if (type == TRIGGER_UNLOCK)
    {
        digitalWrite(mosfettiPinD, HIGH);
        delay(100);
        digitalWrite(mosfettiPinD, LOW);
    }

    if (type == TRIGGER_BUZZER)
    {
        digitalWrite(buzzerPin, HIGH);
        delay(50);
        digitalWrite(buzzerPin, LOW);
    }
}

void setup()
{
    int boardIndex = (digitalRead(boardSelectA) << 1) | digitalRead(boardSelectB);

    String productDescriptor = "CodeTV Board " + String(boardIndex);
    String serialDescriptor = "SN0000" + String(boardIndex);

    TinyUSBDevice.setManufacturerDescriptor("High Order");
    TinyUSBDevice.setProductDescriptor(productDescriptor.c_str());
    TinyUSBDevice.setSerialDescriptor(serialDescriptor.c_str());

    Serial.begin(115200);

    // Pin configuration.

    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);

    pinMode(mosfettiPinA, OUTPUT);
    digitalWrite(mosfettiPinA, LOW);

    pinMode(mosfettiPinB, OUTPUT);
    digitalWrite(mosfettiPinB, LOW);

    pinMode(mosfettiPinC, OUTPUT);
    digitalWrite(mosfettiPinC, LOW);

    pinMode(mosfettiPinD, OUTPUT);
    digitalWrite(mosfettiPinD, LOW);

    pinMode(interruptPin, INPUT_PULLUP);
    //
    // Manual begin() is required on core without built-in support e.g. mbed rp2040
    if (!TinyUSBDevice.isInitialized())
    {
        TinyUSBDevice.begin(0);
    }

    Serial.begin(115200);

    // usb_midi.setStringDescriptor("TinyUSB MIDI");

    // Initialize MIDI, and listen to all MIDI channels
    // This will also call usb_midi's begin()
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }

    MIDI.setHandleSystemExclusive(handleSystemExclusive);

    // MPR121 setup.
    {
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
}

void loop()
{

#ifdef TINYUSB_NEED_POLLING_TASK
    // Manual call tud_task since it isn't called by Core's background
    TinyUSBDevice.task();
#endif

    // not enumerated()/mounted() yet: nothing to do
    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    MIDI.read();

    if (digitalRead(interruptPin) == LOW)
    {

        uint16_t currentStatus = mpr121.getTouchStatus(device_address);
        for (uint8_t i = 0; i < 12; i++)
        {
            bool wasTouched = lastStatus & (1 << i);
            bool isTouched = currentStatus & (1 << i);

            if (!wasTouched && isTouched)
            {
                uint8_t msg[] = {0xF0, 0x7D, 0x01, i, 1, 0xF7};
                MIDI.sendSysEx(sizeof(msg), msg, true);

                digitalWrite(buzzerPin, HIGH);
                delay(50);
                digitalWrite(buzzerPin, LOW);
            }
            else if (wasTouched && !isTouched)
            {
                uint8_t msg[] = {0xF0, 0x7D, 0x01, i, 0, 0xF7};
                MIDI.sendSysEx(sizeof(msg), msg, true);
            }
        }

        lastStatus = currentStatus;
    }
}
