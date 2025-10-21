#include <Arduino.h>

const int ANALOG_PIN  = A0;
const int TRIGGER_PIN1 = 11;
const int TRIGGER_PIN2 = 8;
const int TRIGGER_PIN3 = 7;

const float R_TOP = 1000000.0;   // ohms /
const float R_BOTTOM = 100000.0; // ohms

// thresholds now expressed in terms of Vin (not Vmid)
const float TH_OP   = 1.5;   // Vin operation threshold
const float TH_SAFE = 1.0;   // Vin safe threshold
const float TH_STORE= 0.5;   // Vin store threshold

bool  is_on_deep_sleep = false;
float voltage, vmid;

float readVin() {
  analogReadResolution(12);           // 0–4095
  int adcValue = analogRead(ANALOG_PIN);
  float v_mid = (adcValue / 4095.0) * 3.3; // divider midpoint in volts
  // back-calculate Vin
  return v_mid * (R_TOP + R_BOTTOM) / R_BOTTOM;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN1, OUTPUT);
  pinMode(TRIGGER_PIN2, OUTPUT);
  pinMode(TRIGGER_PIN3, OUTPUT);
}

void loop() {
  voltage = readVin();
  vmid = (voltage*R_BOTTOM) / (R_TOP+R_BOTTOM);
  Serial.print("Vin = ");
  Serial.print(voltage, 3);
  Serial.print(" Vmid = ");
  Serial.println(vmid, 3);

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
