// States:
// Operation: running operations: Sense, Compute, Transit or Load
// Safe: Enter deep sleep mode (low energy availability)
// Store: Store data to MRAM (very low energy availability, almost off)

// Functions to handle the operations
// Compute: Perform calculations based on sensor data, uses TFLModel Class
// Sense: Captures image from Arduino Cam, uses Camera Class
// Store: Stores data to MRAM when energy availability is very low
// Load: Loads model data from MRAM
// Transmit: Sends data to the cloud or another device using BLE

// Interrupt mechanism:
// - Interrupts are used to wake the device from deep sleep mode and put it to sleep
// - Interrupts are triggered by an external signal from PMU based on energy availability


// Note: delays are added in between states for testing purposes and visibility of LED changes
//#include <esp_task_wdt.h>

#include "states.h"
#include "Camera.h"
#include "TFLModel.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "esp_sleep.h"

// ---- Deep Sleep Setup ----
const int DEEP_WAKEUP_PIN = 2;  // Pin used for waking up from deep sleep
volatile bool deepSleepWakeFlag = false;
int deepSleepPinState;

// ---- Light Sleep Setup ----
const int LIGHT_WAKEUP_PIN = 0;  // Pin used for waking up from light sleep
volatile bool lightSleepWakeFlag = false;
int lightSleepPinState;

const int Pin3Red = 3;

// void wakeupISR() {
//   deepSleepWakeFlag = true;
// }

// void deepSleep() {
//   attachInterrupt(digitalPinToInterrupt(DEEP_WAKEUP_PIN), wakeupISR, FALLING);

//   Serial.println("Entering deep sleep...");
//   Serial.flush();

//   // Disable peripherals
//   for (int pin = 0; pin <= 21; pin++) {
//     if (pin != DEEP_WAKEUP_PIN) {
//       pinMode(pin, INPUT_PULLDOWN);
//     }
//   }
//   Wire.end();
//   SPI.end();

//   NRF_POWER->SYSTEMOFF = 1;

//   __SEV();
//   __WFE();
//   __WFE();
// }

// void lightSleepISR() {
//   lightSleepWakeFlag = true;
// }

// void lightSleep() {
//   // pinMode(LIGHT_WAKEUP_PIN, INPUT_PULLDOWN);
//   attachInterrupt(digitalPinToInterrupt(LIGHT_WAKEUP_PIN), lightSleepISR, FALLING);

//   lightSleepWakeFlag = false;
//   delay(100); // Optional: let pin settle
//   bool deep_sleep = false;

//   Serial.println("In light Sleep ...");
//   while (!lightSleepWakeFlag) {
//     __SEV();  // Set event
//     __WFE();  // Clear event
//     __WFE();  // Wait for actual interrupt
//     // Check if the device should go to deep sleep
//     if (deepSleepPinState == HIGH) {
//         Serial.println("Storing before entering deep sleep mode...");
//         deep_sleep = true;
//         break;
//     }
//   }
//   if (deep_sleep) {
//     currentState = STORE;
//   } else {
//     Serial.println("Woke up from light sleep");
//     currentState = nextState;
//   }
// }


void deepSleep() {
    Serial.println("Entering deep sleep...");
    Serial.flush();

    // Configure external wakeup (wake when pin goes LOW = 0)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)DEEP_WAKEUP_PIN, 0);

    // Disable peripherals
    SPI.end();
    Wire.end();

    // Enter deep sleep (device will reset on wake)
    esp_deep_sleep_start();
}

void lightSleep() {
    Serial.println("Entering light sleep...");
    Serial.flush();

    // Make sure the pin is configured
    pinMode(LIGHT_WAKEUP_PIN, INPUT_PULLUP);

    // Configure external wakeup
    //esp_sleep_enable_ext0_wakeup((gpio_num_t)DEEP_WAKEUP_PIN, 0);

    // Enter light sleep (execution resumes here on wake)
    //esp_light_sleep_start();

    while(digitalRead(LIGHT_WAKEUP_PIN) == HIGH){ 

    }

    if (digitalRead(DEEP_WAKEUP_PIN) == HIGH) {
        Serial.println("Energy dropped below threshold!");
        Serial.println("Entering deep sleep...");
        currentState = STORE;
    } else {
        Serial.println("Woke up from light sleep");
        currentState = nextState;
    }

}


// void IRAM_ATTR lightSleepISR() {
//   lightSleepWakeFlag = true;
// }

// void lightSleep() {
//   pinMode(LIGHT_WAKEUP_PIN, INPUT_PULLDOWN);
//   attachInterrupt(digitalPinToInterrupt(LIGHT_WAKEUP_PIN), lightSleepISR, FALLING);

//   lightSleepWakeFlag = false;
//   Serial.println("Entering pseudo light sleep...");

//   while (!lightSleepWakeFlag) {
//     // Enter a light idle; yields and lets Wi-Fi/BLE tick
//     esp_light_sleep_start();   // works even if pin isn’t RTC, ISR will wake CPU
//   }

//   detachInterrupt(LIGHT_WAKEUP_PIN);
//   Serial.println("Woke up!");
// }


void setup()
{   
    Serial.begin(115200);
    SPI.begin();
    Wire.begin();

    while (!Serial);

    pinMode(Pin3Red, INPUT_PULLDOWN);
    int pinRedState = digitalRead(Pin3Red);
    Serial.print("Pin red state: ");
    Serial.println(pinRedState);

    pinMode(DEEP_WAKEUP_PIN, INPUT_PULLDOWN);
    if (digitalRead(DEEP_WAKEUP_PIN) == HIGH) {
        Serial.println("Deep sleep signal detected at startup → going to STORE");
        currentState = STORE;   // Go straight to STORE state
    }

    // --- Check wakeup reason ---
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup from EXT0"); break;
        case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup from EXT1"); break;
        case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup from Timer"); break;
        default : Serial.println("Wakeup from unknown reason"); break;
    }

    pinMode(MRAM_CS_PIN, OUTPUT);
    pinMode(CAM_CS_PIN, OUTPUT);
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(LED_PIN_3, OUTPUT);

    // Deselect all devices at startup
    digitalWrite(CAM_CS_PIN, HIGH);
    digitalWrite(MRAM_CS_PIN, HIGH);

    // Set up BLE communication
    BLEsetup();

    // Load isNVMValid flag from MRAM
    // uint8_t isNVMValidByte;
    // loadDataFromMRAM(IS_NVM_VALID_ADDRESS, &isNVMValidByte, sizeof(isNVMValidByte));
    // isNVMValid = (isNVMValidByte != 0); // Convert byte to boolean

    isNVMValid = false;

    if (isNVMValid)
    {   
        Serial.println("NVM is valid, loading data.");
        loadData();
    }
    else
    {
        Serial.println("NVM is not valid, skipping loadData.");
    }

    // Initialize camera
    if (camera.Init())
    {
        Serial.println("Camera init done");
    }
    else
    {
        Serial.println("Camera init failed");
        return;
    }

    Serial.println("Setup complete. Entering main loop...");
}

void loop()
{
    // Main loop
    // This is where the device will perform its operations based on the current state
    Serial.println("---------------------------------");
    Serial.println("Current State: " + String(currentState));
    Serial.println("Next State: " + String(nextState));
    Serial.println("---------------------------------");

    delay(5000);

    switch (currentState)
    {
    case SENSE:
        stateLED(SENSE); // Indicate sensing state with LED
        performSense(); // Capture image from camera
        break;
    case COMPUTE:
        stateLED(COMPUTE); // Indicate compute state with LED
        performCompute(); // Perform calculations based on sensor data
        break;
    case TRANSMIT:
        stateLED(TRANSMIT); // Indicate transmit state with LED
        transmitData(imageBuffer, width * height * channels);
        delete[] imageBuffer;
        imageBuffer = nullptr;
        break;
    case STORE:
        stateLED(STORE); // Indicate store state with LED
        storeData(); // Store data to MRAM when energy availability is very low
        break;
    case STOP:
        stateLED(STOP); // Indicate stop state with LED
        //delay(5000);
        //currentState = nextState;
        //Serial.println("In Light Sleep");
        lightSleep(); // Enter light sleep mode, waiting for an interrupt to wake up
        break;
    case SLEEP:
        //stateLED(SLEEP); // Indicate sleep state with LED
        //delay(5000);
        //currentState = nextState;
        deepSleep(); // Enter deep sleep mode, waiting for an interrupt to wake up
        break;
    default:
        break;
    }
    delay(5000);
}