#include <DS3231.h>

// Code from the Demo Example of the DS3231 Library
DS3231  rtc(SDA, SCL);

byte ch3OnPin = 6 ;
byte ch3OffPin = 7 ;
byte ch1OnPin = 10 ;
byte ch1OffPin = 9 ;

// Turn on/off wireless remotes
void activateRemote ( byte pin )
{
  for ( int i = 0 ; i < 3 ; i++ )
  {
    digitalWrite ( pin, HIGH ) ;
    delay ( 1000 ) ;
    digitalWrite ( pin, LOW ) ;
    delay ( 1000 ) ;
  }
}

// Pointer to ISR function
typedef void (*PinFunc ) ( byte);

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

// Porch lights on at 5 PM, off at midnight
// xmas lights on at 5:15PM, off at 3 AM.
TimerEvent porchLightON ( "17", "00", "Porch lights ON!", activateRemote, ch3OnPin ) ;
TimerEvent porchLightOFF ( "23", "59", "Portch lights OFF!", activateRemote, ch3OffPin ) ;
TimerEvent xmasLightsON ( "17", "15", "Xmas Lights On!", activateRemote, ch1OnPin ) ;
TimerEvent xmasLightsOFF ( "03", "00", "Xmas Lights Off!", activateRemote, ch1OffPin ) ;

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
  
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(22, 36, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(( uint8_t ) 24, ( uint8_t ) 11, ( uint16_t ) 2017);   // Set the date to January 1st, 2014
  delay ( 500 ) ;

  // start off
  activateRemote ( ch3OffPin ) ;
  activateRemote ( ch1OffPin ) ;
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
  delay (10000);

  porchLightON.tick ( rtc.getTimeStr ( ) ) ;
  porchLightOFF.tick ( rtc.getTimeStr ( ) ) ;
  xmasLightsON.tick ( rtc.getTimeStr ( ) ) ;
  xmasLightsOFF.tick ( rtc.getTimeStr ( ) ) ;
  
}
