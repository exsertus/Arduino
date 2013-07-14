#include <SoftwareSerial.h>
#include <DHT22.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define TX 4
#define RX 3
#define SLEEP 1
#define LED 0
#define DHT22_PIN 2
#define INTERVAL 900
#define XBeeAddress "40A25C4A"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


ISR(WDT_vect) {
  // Don't do anything here but we must include this
  // block of code otherwise the interrupt calls an
  // uninitialized interrupt handler.
}

SoftwareSerial XBee (RX, TX);
DHT22 myDHT22(DHT22_PIN);

int Vcc = 0;

// Watchdog timeout values
// 0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
// 6=1sec, 7=2sec, 8=4sec, 9=8sec
void setup_watchdog(int ii) {  
 // The prescale value is held in bits 5,2,1,0
 // This block moves ii itno these bits
 byte bb;
 if (ii > 9 ) ii=9;
 bb=ii & 7;
 if (ii > 7) bb|= (1<<5);
 bb|= (1<<WDCE);
 
 // Reset the watchdog reset flag
 MCUSR &= ~(1<<WDRF);
 // Start timed sequence
 WDTCR |= (1<<WDCE) | (1<<WDE);
 // Set new watchdog timeout value
 WDTCR = bb;
 // Enable interrupts instead of reset
 WDTCR |= _BV(WDIE);
}


void system_sleep() {
 set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode
 sleep_mode(); // System sleeps here
}


// wait for totalTime ms
// the wait interval is to the nearest 4 seconds
void deepsleep(int waitTime) {
  // Calculate the delay time
  waitTime = waitTime / 4;
  int waitCounter = 0;
  while (waitCounter != waitTime) 
  {
    system_sleep();
    waitCounter++;
  }
}

void setup() {
  setup_watchdog(8);
  
  //ACSR |= _BV(ACD);                         //disable the analog comparator
  //ADCSRA &= ~_BV(ADEN); 

  pinMode(LED,OUTPUT); 
  pinMode(SLEEP,OUTPUT);
  XBee.begin(9600);
}

void loop() {
  
  // Wake up Xbee from sleep
  digitalWrite(SLEEP,LOW);
  digitalWrite(LED, HIGH);
     
  ADCSRA |= (1 << ADEN);  // Enable ADC 
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions 
 
  delay(2000);
  
  Vcc = readVcc();
  
  DHT22_ERROR_t errorCode;
  
  int temp = 0;
  int hum = 0;
  
  // Pause for 2 seconds to allow XBee to wake up and establish comms with the co-ordinator
        
  // Read sensor until no errors are recieved
  errorCode = myDHT22.readData(); 
  //while (errorCode != DHT_ERROR_NONE) {
  //  errorCode = myDHT22.readData();
  //  delay(50);
  //} 
  
  hum = myDHT22.getHumidityInt();
  temp = myDHT22.getTemperatureCInt();
  
  ACSR |= _BV(ACD);                         //disable the analog comparator
  ADCSRA &= ~_BV(ADEN);
  
  // Send temperature
  sendData("Temperature",temp);
     
  // 0.5 sec pause between temp and humidity readings
     
  delay(500);    
     
  // Send humidity
  sendData("Humidity",hum);
  
  // Put XBee back to sleep, wait for 3 secs first to allow all remaining data to be sent
     
  delay(2000);
    
  digitalWrite(SLEEP,HIGH);
  digitalWrite(LED,LOW);
     
  // Pause for INTERVAL (60 seconds) before next reading
  
  deepsleep(INTERVAL);
}

void sendData(char* dataset, int val) {  
  XBee.print(dataset);
  XBee.print(",");
  XBee.print(val/10);
  XBee.print(".");
  XBee.print(val%10);
  XBee.print(",");
  XBee.print(Vcc);
  XBee.print(",");
  XBee.print(String(XBeeAddress));
  XBee.print("\n");
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
