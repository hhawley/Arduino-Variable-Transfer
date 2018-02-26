// NeoPi Arduino code
#include <Wire.h>
#include <inttypes.h>
#include <VariablesTransfer.h>

// Experiment relay pin
#define RELAY_EXP_PIN 7

//This device I2C address
#define I2C_SLAVE 0x04 


// Digital pot analog pin
#define POT_ANG_READ 0
#define EPSILON_V 20

// Variables
uint16_t* relayIsOn;

void setRelayVal(int howMany);

void setup() {
  Serial.begin(9600);

  VarTransfer::linkVariable(0, relayIsOn);
  *relayIsOn = 0;

  pinMode(RELAY_EXP_PIN, OUTPUT);
  digitalWrite(RELAY_EXP_PIN, LOW);

	Wire.begin(I2C_SLAVE);
	Wire.onReceive(setVoltageValue);
	
}

void loop() {

  if(*relayIsOn) { 
    digitalWrite(RELAY_EXP_PIN, HIGH); 
  }
  else { 
    digitalWrite(RELAY_EXP_PIN, LOW); 
  }

}


void setRelayVal(int howMany) {
  VarTransfer::readVariable(Wire);
}
