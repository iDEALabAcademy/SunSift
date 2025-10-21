#pragma once

#include "Camera.h"
#include "TFLModel.h"

#define STATEADDRESS 0x00000000                               // Address in MRAM to store current state (1 byte)
#define IS_NVM_VALID_ADDRESS (STATEADDRESS + sizeof(uint8_t)) // Address in MRAM to store isNVMValid flag (1 byte, 0 or 1)
#define LAYERADDRESS (IS_NVM_VALID_ADDRESS + sizeof(uint8_t)) // Address in MRAM to store layer number (int, 1 byte)
#define INPUTADDRESS (LAYERADDRESS + sizeof(int))             // Address in MRAM to store input
#define IMAGEADDRESS (INPUTADDRESS + (3072 * sizeof(float)))             // Address in MRAM to store image

//extern const int LIGHT_SLEEP_PIN_4;

enum State
{
    COMPUTE = 0b001,  // 01 Capture image from camera
    SENSE = 0b011,    // 11 Perform calculations based on sensor data
    STORE = 0b010,    // 10 Send data to the cloud or another device using BLE
    TRANSMIT = 0b000, // 00 Store data to MRAM (very low energy availability, almost off)
    LOAD = 0b100,      // 100 Load data from MRAM
    STOP = 0b101,     // 101 Light sleep mode
    SLEEP = 0b110     // 110 Deep sleep mode

};

extern State currentState;
extern State nextState;

extern const int NUM_LAYERS; 
extern int layer;
extern float *temp1;
extern float *temp2;
extern uint8_t *imageBuffer; 

extern int width;     
extern int height;    
extern int channels;  


extern Camera camera;

extern bool isNVMValid;   // Flag to indicate if NVM data is valid

extern int lightSleepPinState;
extern int deepSleepPinState;

// Chip Selects
const int MRAM_CS_PIN = 10;
const int CAM_CS_PIN = 7;

// LED
const int LED_PIN_1 = 4; 
const int LED_PIN_2 = 5;
const int LED_PIN_3 = 6;

// Function prototypes
void performSense();
void performCompute();
void transmitData(uint8_t*, size_t);
void storeData();
void loadData();
void stateLED(State state);
void BLEsetup();
void check_MRAM_if_true_write_false();

// ---------- Templated MRAM I/O ----------

template <typename T>
void storeDataToMRAM(uint32_t address, const T *data, size_t length = 1)
{
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    const uint8_t *byteData = reinterpret_cast<const uint8_t *>(data);
    size_t byteLength = length * sizeof(T);

    pinMode(MRAM_CS_PIN, OUTPUT);
    digitalWrite(MRAM_CS_PIN, HIGH);

    // Write enable
    digitalWrite(MRAM_CS_PIN, LOW);
    SPI.transfer(0x06); // WREN
    digitalWrite(MRAM_CS_PIN, HIGH);
    delayMicroseconds(1);

    // Write data
    digitalWrite(MRAM_CS_PIN, LOW);
    SPI.transfer(0x02); // WRITE
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    for (size_t i = 0; i < byteLength; ++i)
    {
        SPI.transfer(byteData[i]);
    }

    digitalWrite(MRAM_CS_PIN, HIGH);
}

template <typename T>
void loadDataFromMRAM(uint32_t address, T *data, size_t length = 1)
{
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    uint8_t *byteData = reinterpret_cast<uint8_t *>(data);
    size_t byteLength = length * sizeof(T);

    pinMode(MRAM_CS_PIN, OUTPUT);
    digitalWrite(MRAM_CS_PIN, HIGH);

    digitalWrite(MRAM_CS_PIN, LOW);
    SPI.transfer(0x03); // READ
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    for (size_t i = 0; i < byteLength; ++i)
    {
        byteData[i] = SPI.transfer(0x00);
    }

    digitalWrite(MRAM_CS_PIN, HIGH);
}