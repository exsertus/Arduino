/*============================================================
  Name : XBeeTempHumMoistureSensor
  Author : exsertus.com (Steve Bowerman)
 
  Summary
  Generic Xbee sensor program.
  
  Version : 2.0

  Credits
  
============================================================*/

#include <SoftwareSerial.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Setup 85 and 84 Specifics
#if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  #define TX 7
  #define RX 8
  #define SLEEP 10
  #define SENSPWR 9
  #define DIO1 1
  #define DIO2 2
  #define DIO3 5
  #define AIO1 1
  #define AIO2 2
  #define AIO3 5
  
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #define TX 4
  #define RX 3
  #define SLEEP 1
  #define SENSPWR 0 
  #define DIO1 2
  #define AIO1 1
  
#endif  

#define INTERVAL 5//900
#define CACHESIZE 1

#define PRECISION 10 // 1 decimal place

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

struct LogType {
  int temp;
  int moist;
  int vcc;
  int offset;
};

// Globals

SoftwareSerial XBee (RX, TX);
LogType logs[CACHESIZE];
int logcount = 0;
int offset = 0;

ISR(WDT_vect) {
  // Don't do anything here but we must include this
  // block of code otherwise the interrupt calls an
  // uninitialized interrupt handler.
}


/*----------------------------------------------------------
  Function : setup_watchdog
  Inputs : None
  Return : None
  Globals : None
  
  Summary
  Sets watchdog interupt up
  Watchdog timeout values
  0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
  6=1sec, 7=2sec, 8=4sec, 9=8sec
  Caters for different names for WDT between ATTiny84 and ATTiny85
----------------------------------------------------------*/

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
   
   #if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
      // Start timed sequence
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      // Set new watchdog timeout value
      WDTCSR = bb;
      // Enable interrupts instead of reset
      WDTCSR |= _BV(WDIE);
   #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
      // Start timed sequence
      WDTCR |= (1<<WDCE) | (1<<WDE);
      // Set new watchdog timeout value
      WDTCR = bb;
      // Enable interrupts instead of reset
      WDTCR |= _BV(WDIE);
   #endif

}


/*----------------------------------------------------------
  Function : deepsleep
  Inputs : None
  Return : None
  Globals : None
  
  Summary
  Puts Attiny into sleep mode (consumes around 10uA)

----------------------------------------------------------*/

void deepsleep(int waitTime) {
  // Calculate the delay time
  waitTime = waitTime / 4;
  int waitCounter = 0;
  while (waitCounter != waitTime) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode
    sleep_mode(); // System sleeps here
    waitCounter++;
  }
}


/*----------------------------------------------------------

  Function : setup
  Inputs : None
  Return : None
  Globals : Vcc, XBee, SLEEP, SENSPWR
  
  Summary
  Sets up program, called once when the circuit is powered up.
  Sets pins, creates software serial object, watchdog and 
  gets XBee Address

----------------------------------------------------------*/

void setup() {  
  pinMode(SENSPWR,OUTPUT); 
  pinMode(SLEEP,OUTPUT);  
  digitalWrite(SLEEP,HIGH);
  
  XBee.begin(9600);
  setup_watchdog(8); 
 
}

/*----------------------------------------------------------

  Function : loop
  Inputs : None
  Return : None
  Globals : Vcc, XBee, XBeeAddress, INTERVAL, SENSPWR, SLEEP
  
  Summary
  Main loop routine. Wakes up Xbee, takes reading, sends 
  reading, puts XBee back to sleep, then finally puts ATTIny
  into sleep mode.
  Avg awake consumption = 20mAh, sleep consumption = 10uAh

----------------------------------------------------------*/

void loop() {
   /*
  
     Take sensor readings and store results in memory
   
   */
   
   digitalWrite(SENSPWR, HIGH);
   
   // Enable ADC
   ADCSRA |= (1 << ADEN); 
   ADCSRA |= (1 << ADSC); 
     
   analogReference(INTERNAL);
   
   double temp = analogRead(AIO3);
   temp *= 1.07;
   temp -= 500;
   temp /= 10;
   
   logs[logcount].moist = analogRead(AIO2);
   logs[logcount].temp = temp;
   logs[logcount].vcc = readVcc();
   logs[logcount].offset = INTERVAL*(CACHESIZE-offset-1);
   
   logcount++;  
   offset++;
   
   // Disable ADC  
   ACSR |= _BV(ACD);                         
   ADCSRA &= ~_BV(ADEN);
   
   digitalWrite(SENSPWR, LOW);

   /*
      Upload readings to server
   
   */
 
   if (logcount >= CACHESIZE) {
     
     // Wake up Xbee from sleep
     digitalWrite(SLEEP,LOW);
     delay(500);
     
     for (int i=0; i<logcount; i++) {    
       char data[20];
       int I = 0;
       int D = 0;
       
       I = (int)logs[i].temp;
       D = (int)abs((logs[i].temp-I)*PRECISION);
       sprintf(data,"T,%d.%d,%d,%d\n", I, D, logs[i].vcc, logs[i].offset);
       XBee.print(data);
       delay(500);  
       
       I = logs[i].moist;
       sprintf(data,"M,%d,%d,%d\n", I, logs[i].vcc, logs[i].offset);
       XBee.print(data);
       delay(500);  
    
     }
     
     logcount = 0;
     offset = 0;
     
     digitalWrite(SLEEP,HIGH);
  
   }  

   // Go back to sleep
   deepsleep(INTERVAL);
  
}


/*----------------------------------------------------------

  Function : readVcc
  Inputs : None
  Return : None
  Globals : Vcc
  
  Summary
  Sets the Vcc global variable with the current supply 
  voltage to the Attiny. 
  Works by reading 1.1V reference against AVcc
  set the reference to Vcc and the measurement to the 
  internal 1.1V reference

----------------------------------------------------------*/

long readVcc() {
  #if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
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
