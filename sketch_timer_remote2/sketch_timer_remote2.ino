
// Control Holiday lights with timing and motion sensing
#include <DS3231.h>

// Pointer to ISR function
typedef void (*PinFunc ) ( byte);

// Pointer to polling function
typedef int (*PollFunc ) ( byte ) ;

// Code from the Demo Example of the DS3231 Library
DS3231  rtc(SDA, SCL);

byte ch3OnPin = 6 ;
byte ch3OffPin = 7 ;
byte ch1OnPin = 10 ;
byte ch1OffPin = 9 ;
byte pirPin = 8 ; // check this

int gTimingEnabled = 0 ;

// Turn on/off wireless remotes
void activateRemote ( byte pin )
{
  for ( int i = 0 ; i < 5 ; i++ )
  {
    digitalWrite ( pin, HIGH ) ;
    delay ( 1000 ) ;
    digitalWrite ( pin, LOW ) ;
    delay ( 1000 ) ;
  }
}

int pollAndWait ( int totalWait, int pollTime, PollFunc pFunc, byte pinNum )
{
  uint32_t startTime = millis ( ) ;
  uint32_t timeNow = millis ( ) ;
  int retVal = 0 ;

  while ( timeNow - startTime < totalWait )
  {
    int result = ( *pFunc ) ( pinNum ) ;
    if ( result )
    {
      retVal = 1 ;
    }
    delay ( pollTime ) ;
    timeNow = millis ( ) ;
    return retVal ;
  }
  return 0 ;
}

// Generic event class
class Event
{
  public:
    byte triggered = 0 ;
    char* eventName = "" ;
    char* hr ;
    char* minute ;

    Event ( )
    {
    }

    virtual void tick ( char* timeStr ) {};
} ;

// Timed event.  When time is met, call the eventFunction
class TimerEvent : public Event 
{
  public:
    
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
        Serial.print ( eventName ) ;
        Serial.print ( ",Timed," ) ;
        Serial.print ( timeStr ) ;
        Serial.print ( "," ) ;
        Serial.println ( parm ) ;
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

// When duration is met, call the event function.
class DurationEvent : Event 
{
  public:
    int duration = 0 ;
    uint32_t startTime = 0 ;
    int timerRunning = 0 ;
    PinFunc eventFunction1 = 0 ;
    PinFunc eventFunction2 = 0 ;
    int parm1 = 0 ;
    int parm2 = 0 ;
    
    DurationEvent ( char* pName, int interval, PinFunc pFunc1, int pParm1, PinFunc pFunc2, int pParm2 )
    {
      eventName = pName ;
      duration = interval ;
      eventFunction1 = pFunc1 ;
      eventFunction2 = pFunc2 ;
      parm1 = pParm1 ;
      parm2 = pParm2 ;
    }

    void tick ( )
    {
      if ( timerRunning )
      {
        if ( ( millis ( ) - startTime ) > duration )
        {
          timerRunning = 0 ;
          Serial.print( eventName ) ;
          Serial.print ( ",stop," ) ;
          Serial.println ( parm2 ) ;
          ( *eventFunction2 ) ( parm2 ) ;
        }
      }
    }

    void startRunning ( int timingEnabled )
    {
      if ( timingEnabled )
      {
        startTime = millis ( ) ;
        timerRunning = 1 ;
        Serial.print ( eventName ) ;
        Serial.print ( "Start," ) ;
        Serial.println ( parm2 ) ;
        ( *eventFunction1 ) ( parm1 ) ;
      }
    }

    void stopRunning ( )
    {
      timerRunning = 0 ;
    }

    void enableTimer ( )
    {
      gTimingEnabled = 1 ;
    }

    void disableTimer ( )
    {
      gTimingEnabled = 0 ;
    }
} ;

void activateTiming ( byte state )
{
  gTimingEnabled = state ;
}

// Porch lights on at 5 PM, off at midnight
// xmas lights on at 5:15PM, off at 3 AM.
// toggle xmas lights on with motion after midnight and before 6 am
TimerEvent porchLightON ( "17", "00", "Porch lights ON!", activateRemote, ch3OnPin ) ;
TimerEvent porchLightOFF ( "23", "59", "Portch lights OFF!", activateRemote, ch3OffPin ) ;
TimerEvent xmasLightsON ( "17", "15", "Xmas Lights On!", activateRemote, ch1OnPin ) ;
TimerEvent xmasLightsOFF ( "23", "58", "Xmas Lights Off!", activateRemote, ch1OffPin ) ;

TimerEvent armLightToggle ( "00", "01", "Arm PIR sensing", activateTiming, 1 ) ; 
TimerEvent disarmLightToggle ( "07", "00", "Disarm PIR sensing", activateTiming, 0 ) ; 
DurationEvent xmasLightsBrighten ( "Brighten Up", 60000, activateRemote, ch3OnPin, 
  activateRemote, ch3OffPin ) ;

// Standard part of the Arduino idiom -- run once initialization.
void setup()
{
  // Setup Serial connection
  Serial.begin(115200);

  // Initialize the rtc object
  rtc.begin();

  // Output pins
  pinMode ( ch3OnPin, OUTPUT ) ;
  pinMode ( ch3OffPin, OUTPUT ) ;
  pinMode ( ch1OnPin, OUTPUT ) ;
  pinMode ( ch1OffPin, OUTPUT ) ;
  pinMode ( pirPin, INPUT ) ;
  
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(22, 36, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(( uint8_t ) 24, ( uint8_t ) 11, ( uint16_t ) 2017);   // Set the date to January 1st, 2014
  delay ( 500 ) ;

  // start off
  activateRemote ( ch3OffPin ) ;
  activateRemote ( ch1OffPin ) ;

  // debugging
  delay ( 10000 ) ;
  activateRemote ( ch3OnPin ) ;
  activateRemote ( ch1OnPin ) ;
  
  //activateTiming ( 1 ) ;
  Serial.println ( "Starting timing" ) ;
}

// called to see if we need to turn on xmas lights based
// on a PIR trip
int checkPIRTripped ( byte pin )
{
  if( digitalRead ( pin ) )
  {
    return 0 ;
  }
  return 1 ;
}

// Standard part of the Arduino Idiom -- runs forever after setup ( )
void loop()
{
  Serial.print ( "TOD," ) ;
  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(",");

  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(",");

  // Send time
  Serial.println(rtc.getTimeStr());

  // Wait one second before repeating
  //delay (10000);

  char* timeStr = rtc.getTimeStr ( ) ;
  porchLightON.tick ( timeStr ) ;
  porchLightOFF.tick ( timeStr ) ;
  xmasLightsON.tick ( timeStr  ) ;
  xmasLightsOFF.tick ( timeStr ) ;
  armLightToggle.tick ( timeStr ) ;
  disarmLightToggle.tick ( timeStr ) ;
  xmasLightsBrighten.tick ( ) ;

  int result = pollAndWait ( 10000, 500, checkPIRTripped, pirPin ) ;
  if ( result )
  {
    Serial.print ( "Motion," ) ;
    Serial.print(rtc.getDateStr());
    Serial.print(",");
    Serial.println ( rtc.getTimeStr ( )  ) ;
    xmasLightsBrighten.startRunning ( gTimingEnabled ) ;
  }
  
}
