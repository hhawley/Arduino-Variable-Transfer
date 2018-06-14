// Variable transfer example code
#include <inttypes.h>
#include <VariablesTransfer.h>

// Experiment relay pin
#define RELAY_EXP_PIN 7

// Variables in the buffer
uint16_t* relayIsOn;
uint16_t* value;

void setup() {
  Serial.begin(9600);

  VarTransfer::linkVariable(0, relayIsOn);
  VarTransfer::linkVariable(1, value);
  *relayIsOn = 0;
  *value = 502;

  pinMode(RELAY_EXP_PIN, OUTPUT);
  digitalWrite(RELAY_EXP_PIN, LOW);
	
}

void loop() {

  int err = VarTransfer::processVariables(Serial);
  if(err == VT_SUCESS) {
    // Code to be executed if a correct transfer was made
  } else if(err != VT_NO_DATA){
    Serial.print("Error at processing command. Error No: ");
    Serial.println(err);
  }

  if(*relayIsOn) { 
    digitalWrite(RELAY_EXP_PIN, HIGH); 
  }
  else { 
    digitalWrite(RELAY_EXP_PIN, LOW); 
  }

}

