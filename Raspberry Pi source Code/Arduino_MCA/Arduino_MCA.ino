#include <VariablesTransfer.h>
#include <inttypes.h>

#define VOLT_REF 5
#define ADC_BITS 10
#define MAX_CHANNEL (1 << ADC_BITS)
#define VOLT_TO_VAL(x) (int)((x * MAX_CHANNEL) / VOLT_REF)
#define V_MAX VOLT_TO_VAL(1.5)
#define MCA_CHANNELS (V_MAX)


/// PIN DEFINITIONS
#define TRIGGER 2
#define RESET 13
#define ADC_CHANNEL 0
/// END PIN DEFINITIONS

volatile uint32_t MCA_BUFFER[MCA_CHANNELS];

uint16_t *turnON;
uint16_t *turnOFF;
uint16_t *sendValues;
uint16_t *resetValues;

volatile int adcVal = 0;
bool isOn = false;

void save_values(void);
void reset_values(void);
void set_adc(void);
int fast_adc(void);

void setup() {
  Serial.begin(9600);

  VarTransfer::linkVariable(0, turnON);
  VarTransfer::linkVariable(1, turnOFF);
  VarTransfer::linkVariable(2, sendValues);
  VarTransfer::linkVariable(3, resetValues);

  *isOn = 0;
  *sendValues = 0;
  *resetValues = 0;
  
  pinMode(TRIGGER, INPUT);
  pinMode(RESET, OUTPUT);

  digitalWrite(RESET, LOW);
  

}

void loop() {

  if(VarTransfer::readVariable(Serial)) {
    if(*sendValues && *isOn) {

      save_values();
      *sendValues = 0;
      
    }

    if(*resetValues) {

      reset_values();
      *resetValues = 0;
      
    }

    if(*turnON) {
      EIFR = 0xFF; 
      attachInterrupt( digitalPinToInterrupt (TRIGGER), ftm0_isr, RISING);
      isOn = true;
      *turnON = 0;
    } 

    if(*turnOFF) {
      detachInterrupt( digitalPinToInterrupt (TRIGGER) );
      isOn = false;
      *turnOFF = 0;
    }
    
  }

}

void ftm0_isr(void){
    delayMicroseconds(1);
    // We delay de measurement to avoid "glitches" For example: two measurements at the same time.
    // At this point we dont care about speed anymore. We require a good measurement
    int value = fast_adc();
    if(value < V_MAX) {
      MCA_BUFFER[value]++;
    }

    digitalWrite(RESET, HIGH);
    digitalWrite(RESET, LOW);
}


void save_values() {

  if(isOn) {
    detachInterrupt( digitalPinToInterrupt (TRIGGER) );
  }
  
  Serial.print("DATA:{");

  //char* mca_channels_char = (char*)&MCA_BUFFER;
  
  for(int i = 0; i < MCA_CHANNELS-1; i++) {
    Serial.print(MCA_BUFFER[i], DEC);
    Serial.print(",");
  }

  Serial.print(MCA_BUFFER[MCA_CHANNELS - 1], DEC);
  
  Serial.println("}");

  if(isOn) {
    EIFR = 1; 
    attachInterrupt( digitalPinToInterrupt (TRIGGER), ftm0_isr, RISING);
  }

}

void reset_values() {

  if(isOn) {
    detachInterrupt( digitalPinToInterrupt (TRIGGER) );
  }
  
  memset((uint16_t*)MCA_BUFFER, 0, sizeof(MCA_BUFFER));

  if(isOn) {
    EIFR = 1; 
    attachInterrupt( digitalPinToInterrupt (TRIGGER), ftm0_isr, RISING);
  }
  
}

void set_adc() {
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (0 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (0 << ADPS0) | (0 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (0 << ADATE); //enabble auto trigger
  ADCSRA |= (0 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  //ADCSRA |= (1 << ADSC); //start ADC measurements
}

int fast_adc() {
  adcVal = 0;
  ADCSRA |= (1 << ADSC); //start ADC measurements
  while((ADCSRA >> ADIF) & 1);
  adcVal = ADCL;
  adcVal |= ( ADCH << 8 );
  
  return adcVal;
}

