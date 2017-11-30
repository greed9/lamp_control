#include <DS3231.h>

// Code from the Demo Example of the DS3231 Library
DS3231  rtc(SDA, SCL);

#define ON 0
#define OFF 1

byte onPin = 6 ;
byte offPin = 7 ;

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

class TimerEvent
{
  public:
    char* hr ;
    char* minute ;
    byte onOffStatus ;
    byte triggered = 0 ;
    char* eventName = "" ;

    TimerEvent ( )
    {
      hr = "00" ;
      minute = "00" ;
      onOffStatus = ON ;
      triggered = 0 ; // Not triggered (yet)
    }

    TimerEvent ( char* pHr, char* pMin, byte pOnOff, char* pName )
    {
      hr = pHr ;
      minute = pMin ;
      onOffStatus = pOnOff ;
      triggered = 0 ;
      eventName = pName ;
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
        if( onOffStatus == ON )
        {
          Serial.print ( eventName ) ;
          Serial.print ( " " ) ;
          Serial.println ( "Turning ON" ) ;
          turnOn ( 3 ) ;
        }
        if ( onOffStatus == OFF )
        {
          Serial.print (eventName ) ;
          Serial.print ( " " ) ;
          Serial.println ( "Turning OFF:" ) ;
          turnOff ( 3 ) ;
        }
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

TimerEvent lightON ( "17", "00", ON, "Lights ON!" ) ;
TimerEvent lightOFF ( "23", "59", OFF, "Lights OFF!" ) ;
//TimerEvent lightON ( "10", "21", ON, "Lights ON!" ) ;
//TimerEvent lightOFF ( "10", "23", OFF, "Lights OFF!" ) ;

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);
  // Uncomment the next line if you are using an Arduino Leonardo
  //while (!Serial) {}

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
