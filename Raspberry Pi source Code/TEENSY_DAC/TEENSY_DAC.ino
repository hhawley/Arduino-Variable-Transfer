#include <VariablesTransfer.h>
#include <inttypes.h>
#define Table256

#include "fullTables.c"                

uint16_t* isOn;
uint16_t* wavefunctionAmplitude;
uint16_t* wavefunctionFrequency;
uint16_t* typeFunction;

IntervalTimer waveTimer;               

// for some reason, need function prototypes
void setupPeriod();
void MakeSineISR();

unsigned int sinePeriod;               

volatile byte t_bitMask = MAX_BITMASK;
volatile byte t_ratio = 1;
volatile unsigned char DACValue = 0;
volatile unsigned char sineTableindex = 0;

void setup() {
  Serial.begin(9600);

  VarTransfer::linkVariable(0, isOn);
  VarTransfer::linkVariable(1, wavefunctionAmplitude);
  VarTransfer::linkVariable(2, wavefunctionFrequency);
  VarTransfer::linkVariable(3, typeFunction);

  *isOn = 1;
  *wavefunctionAmplitude = 20;
  *wavefunctionFrequency = 5000;
  *typeFunction = 1;
  
  setupPeriod();                       
  waveTimer.priority(255);

}

void loop() {
  if(VarTransfer::readVariable(Serial)) {
    noInterrupts();
    waveTimer.end();
    if(*wavefunctionAmplitude < 1) { *wavefunctionAmplitude = 1;}
    if(*typeFunction > NUM_FUNCTS - 1) { *typeFunction = NUM_FUNCTS - 1; }
    setupPeriod();
    interrupts();
  }

}

void setupPeriod() {
  //waveTimer.end();
  int tablePeriod = 0;
  sineTableindex = 0;                 
  sinePeriod = 1000000 / *(wavefunctionFrequency); // wave period
  for(t_bitMask = MAX_BITMASK; t_bitMask > 2; t_bitMask = t_bitMask / 2) {
    
    tablePeriod = sinePeriod / (t_bitMask + 1);
    if(tablePeriod > 5) { break; }
  }

  t_ratio = (MAX_BITMASK + 1) / (t_bitMask + 1);
  // Attach ISR function to the interrupt and start the tone on DAC
  waveTimer.begin(MakeSineISR, tablePeriod);
}

void MakeSineISR() {
  
  if(*isOn) {
    sineTableindex = sineTableindex + t_ratio;
    DACValue = functions[*(typeFunction)][sineTableindex];
    DACValue = DACValue / *(wavefunctionAmplitude);
  
    analogWrite(A12, DACValue); 
  }

}
