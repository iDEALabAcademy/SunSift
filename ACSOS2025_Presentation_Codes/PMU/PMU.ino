#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

const int ANALOG_PIN = A0;  // Replace with the pin you're using
const int TRIGGER_PIN1 = 11; // D3 for safe threshold (red)
const int TRIGGER_PIN2 = 8; // D2 for deep sleep (yellow)
const int TRIGGER_PIN3 = 7; // D0 for light sleep (blue)


bool is_on_deep_sleep = false;
float voltage;
float const TH_OP = 1.5; // Operation threshold
float const TH_SAFE = 1.0; // Safe threshold
float const TH_STORE = 0.5; // Store threshold

// Reads ADC value and calculate voltage ref
float readVoltage(){
  analogReadResolution(12);  // 0–4095, default is 10
  int adcValue = analogRead(ANALOG_PIN);
  voltage = (adcValue / 4095.0) * 3.3;  // Vref is Vin*2 if R1=R2 

  return voltage;
}
void setup() {
  // Set D2 and D3 as output
  Serial.begin(115200);
  pinMode(TRIGGER_PIN1, OUTPUT);
  pinMode(TRIGGER_PIN2, OUTPUT);  // D2
  pinMode(TRIGGER_PIN3, OUTPUT);  // D3
}


void loop() {

  voltage = readVoltage();
  Serial.println(voltage);

  if (voltage < TH_STORE) {
    // D2 == 1 (Deep Sleep)
    digitalWrite(TRIGGER_PIN2, HIGH);
    is_on_deep_sleep = true;
  }
  else if (voltage > TH_OP) //FIXME: refer to comment on the board
  {
      digitalWrite(TRIGGER_PIN2, LOW);
      is_on_deep_sleep = false;
  }

  if (voltage > TH_SAFE)
  { 
    digitalWrite(TRIGGER_PIN1, LOW);
  }
  else
    digitalWrite(TRIGGER_PIN1, HIGH);
    
  if(!is_on_deep_sleep)
  {
    if ((voltage > TH_OP)) 
    { 
      digitalWrite(TRIGGER_PIN3, LOW);
      delay(1000);
    }
    digitalWrite(TRIGGER_PIN3, HIGH);
  }

  delay(10000);

}
