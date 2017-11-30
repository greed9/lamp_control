#include <DS3231.h>
#include <TimerOne.h>           // Avaiable from http://www.arduino.cc/playground/Code/Timer1

// Code from the Demo Example of the DS3231 Library
DS3231  rtc(SDA, SCL);

#define ON 0
#define OFF 1

byte onPin = 6 ;
byte offPin = 7 ;

// Turn on wireless remote
void turnOn ( int nTimes )
{
  for ( int i = 0 ; i < nTimes ; i++ )
  {
    digitalWrite ( onPin, HIGH ) ;
    delay ( 1000 ) ;
    digitalWrite ( onPin, LOW ) ;
    delay ( 1000 ) ;
  }
}

// Turn off wireless remote
void turnOff ( int nTimes )
{
  for ( int i = 0 ; i < nTimes ; i++ )
  {
    digitalWrite ( offPin, HIGH ) ;
    delay ( 1000 ) ;
    digitalWrite ( offPin, LOW ) ;
    delay ( 1000 ) ;
  }
}

// zero-crossing sync dimming using:
// https://www.amazon.com/RobotDyn-controller-control-Arduino-Raspberry/dp/B072K9P7KH/ref=sr_1_1?ie=UTF8&qid=1512008507&sr=8-1&keywords=robotdyn+dimmer

class Dimmer 
{
  public:
    int acPin = 3 ;             // output to dimmer pwm pin
    int dim = 128 ;             // dimming level 128 is off, 0 is full on
    int freqStep = 65 ;         // magic number for 60 hz timing
    volatile int intCount = 0 ; // Number of zero crossings so far
    volatile int ts = 0 ;       // time step
    volatile boolean zeroCross = 0 ;
    int stepCount = 8 ;         // change dim by this amount each time?

    Dimmer ( ) 
    {
      initialize ( ) ;
    }

    Dimmer ( int pPin, int pStartLevel = 128 )
    {
      acPin = pPin ;
      dim = pStartLevel ;
      initialize ( ) ;
    }

    void initialize ( )
    {
      pinMode(acPin, OUTPUT);                          // Set the Triac pin as output
      attachInterrupt(0, zeroCrossWrapper, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
      Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
      Timer1.attachInterrupt(dimCheckWrapper, freqStep);      // Go to dim_check procedure every 75 uS (50Hz)  or 65 uS (60Hz)
  
    }

    // Actual ISR's w/in object, wrapped with external global func
    void zeroCrossISR ( )
    {
      zeroCross = true;               // set flag for dim_check function that a zero cross has occured
      ts = 0;                             // stepcounter to 0.... as we start a new cycle
      digitalWrite(acPin, LOW);
      intCount ++ ;
    }

    // Actual ISR's w/in object, wrapped with external global func
    void dimCheckISR ( )
    {
        if(zeroCross == true)
        {              
          if(ts>=dim) 
          {                     
            digitalWrite(acPin, HIGH);  // turn on light       
            ts = 0 ;  // reset time step counter                         
            zeroCross = false;    // reset zero cross detection flag
          } 
          else 
          {
            ts++;  // increment time step counter                     
          }    
      }
    }

    // Brighten
    void increase ( int pStep )
    {
      dim += pStep ;
      if ( dim > 127 )
      {
        dim = 127 ;
      }
    }

    // Dim the power
    void decrease ( int pStep )
    {
      dim -= pStep ;
      if ( dim < 0 )
      {
        dim = 0 ;
      }
    }
} ;

Dimmer dimmer ( 3 ) ;

// Global wrapper function for ISR.  Deals with "this" argument problem
void zeroCrossWrapper ( ) 
{
  dimmer.zeroCrossISR ( ) ;
}

// Global wrapper function for ISR.  Deals with "this" argument problem
void dimCheckWrapper ( )
{
  dimmer.dimCheckISR ( ) ;
}

// Pointer to ISR function
typedef void (*PinFunc ) ( int);

// Timed event.  When time is met, call the eventFunction
class TimerEvent
{
  public:
    char* hr ;
    char* minute ;
    byte triggered = 0 ;
    char* eventName = "" ;
    // typedef void (*myfunc)();
    PinFunc eventFunction = 0 ;
    int parm = 0 ;

    TimerEvent ( )
    {
      hr = "00" ;
      minute = "00" ;
      triggered = 0 ; // Not triggered (yet)
      eventFunction = 0 ;
      parm = 0 ;
    }

    TimerEvent ( char* pHr, char* pMin, char* pName, PinFunc pFunc, int pParm )
    {
      hr = pHr ;
      minute = pMin ;
      triggered = 0 ;
      eventName = pName ;
      eventFunction = pFunc ;
      parm = pParm ;
    }

    void tick ( char* timeStr )
    {
      char theHr[3] = { 0 } ;
      char theMin[3] = { 0 } ;
      strncpy ( theHr, timeStr, 2 ) ;
      strncpy ( theMin, &timeStr[3], 2 ) ;
      
      if ( strcmp ( theHr, hr ) == 0 && strcmp ( theMin, minute ) == 0 && triggered == 0 )
      {
        triggered = 1 ;
        Serial.println ( eventName ) ;
        (*eventFunction) ( parm ) ;
      }
      else
      {
        if ( strcmp ( theHr, hr ) < 0 || strcmp ( theMin, minute ) < 0 && triggered == 1 )
        {
          triggered = 0 ;
        }
      }
    }
} ; // end TimerEvent class

TimerEvent lightON ( "17", "00", "Lights ON!", turnOn, 3 ) ;
TimerEvent lightOFF ( "23", "59", "Lights OFF!", turnOff, 3 ) ;
//TimerEvent lightON ( "10", "21", ON, "Lights ON!" ) ;
//TimerEvent lightOFF ( "10", "23", OFF, "Lights OFF!" ) ;

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);

  // Initialize the rtc object
  rtc.begin();

  // Output pins
  pinMode ( onPin, OUTPUT ) ;
  pinMode ( offPin, OUTPUT ) ;

  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(22, 36, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(( uint8_t ) 24, ( uint8_t ) 11, ( uint16_t ) 2017);   // Set the date to January 1st, 2014
  delay ( 500 ) ;

  turnOff ( 3 ) ;
}

void loop()
{
  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");

  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");

  // Send time
  Serial.println(rtc.getTimeStr());

  // Wait one second before repeating
  delay (1000);

  lightON.tick ( rtc.getTimeStr ( ) ) ;
  lightOFF.tick ( rtc.getTimeStr ( ) ) ;
}
