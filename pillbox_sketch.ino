#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include "Adafruit_BLE_UART.h"

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 3     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 4

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);



RTC_DS3231 RTC;
#define PWM_COUNT 1020   //determines how often the LED flips
#define LOOP_DELAY 5000 //ms delay time in loop
#define RTC_SQW_IN 7     // input square wave from RTC into T1 pin (D5)                             //WE USE TIMER1 so that it does not interfere with Arduino delay() command
#define INT0_PIN   8     // INT0 pin for 32kHz testing?
#define LED_PIN    9     // random LED for testing...tie to ground through series resistor..
#define LED_ONBAORD 13   // Instead of hooking up an LED, the nano has an LED at pin 13.

volatile long TOGGLE_COUNT = 0;

/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  Serial.begin(57600);
  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Print echo demo"));

  BTLEserial.setDeviceName("PillBox"); /* 7 characters max! */

  BTLEserial.begin();
  
    Wire.begin();
    RTC.begin();
  
  if (! RTC.isrunning()) {
  Serial.println(F("RTC is NOT running!"));
  // following line sets the RTC to the date & time this sketch was compiled
  RTC.adjust(DateTime(__DATE__, __TIME__));    }

DateTime now = RTC.now();
DateTime compiled = DateTime(__DATE__, __TIME__);
if (now.unixtime() < compiled.unixtime()) {
  //Serial.println("RTC is older than compile time!  Updating");
  RTC.adjust(DateTime(__DATE__, __TIME__));
}

RTC.enable32kHz(true);
RTC.SQWEnable(true);    
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
boolean new_prescription = false;
int addr = 0;
byte value = 0;

void loop()
{
  // Tell the nRF8001 to do whatever it should be working on.
  BTLEserial.pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {

   Call(); 

   
}
}
String medicine[4];
int time[4];
int med_counter = 1;

void RTC_Clock()
{
    DateTime now = RTC.now();
    
    RTC.forceTempConv(true);  //DS3231 does this every 64 seconds, we are simply testing the function here
    float temp_float = RTC.getTempAsFloat();
    int16_t temp_word = RTC.getTempAsWord();
    int8_t temp_hbyte = temp_word >> 8;
    int8_t temp_lbyte = temp_word &= 0x00FF;
    Serial.print(now.year(), DEC);    Serial.print('/');
    Serial.print(now.month(), DEC);    Serial.print('/');
    Serial.print(now.day(), DEC);    Serial.print(' ');
    Serial.print(now.hour(), DEC);    Serial.print(':');
    Serial.print(now.minute(), DEC);    Serial.print(':');
    Serial.print(now.second(), DEC);    Serial.println();
    //Display temps
    Serial.print("Temp as float: ");    Serial.print(temp_float, DEC);
    Serial.println();    Serial.print("Temp as word: ");
    Serial.print(temp_hbyte, DEC);    Serial.print(".");
    Serial.print(temp_lbyte, DEC);    Serial.println(); Serial.println();
    
    delay(LOOP_DELAY);
}



void Call()
{
    String text = "";
    
    while (BTLEserial.available()) 
    {
      char c = BTLEserial.read();
      text = text + c;
      Serial.print(c);
      Serial.println(text[0]);
    }
      
    if (text[0] == '$')
    {Serial.println();
      Serial.println(F("Prescription is going to be sent from the phone to the Pill Box"));
      Serial.println(F("Need to log the new prescription to the EEPROM"));      
      int num = text.length();
      int pillQty = text[num-1] - 48;
      Serial.print(F("Prescription has "));
      Serial.print(pillQty);
      Serial.println(F(" pills"));
      
      String s = "Send Data";
      uint8_t sendbuffer[20];
      s.getBytes(sendbuffer, 20);
      char sendbuffersize = min(20, s.length());
      BTLEserial.write(sendbuffer, sendbuffersize);   
    }  
    
    else if (text[0] == '!')
    {
    if (text[1] == '1')
      {
      for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0);
        addr = 0;
      }
      
      
      Serial.println();
      Serial.print(F("Going to recieve # "));
      Serial.print(text[1]);
      Serial.println(F(" prescription:"));
      
      int num = text.length();
      Serial.print(F("Prescription length is  "));
      Serial.println(num-2);
      
      byte values = 0;
      
      
      for (int i = 2; i < num; i++)
      {  EEPROM.write(addr, text[i]);
         medicine[med_counter] = medicine[med_counter] + text[i];
         values = EEPROM.read(addr);
         Serial.print(F("EEPROM address is: "));
         Serial.println(addr);
         Serial.print(F("The prescription is: "));
         Serial.println(values);
         addr++;
      } 
     
      Serial.println(medicine[med_counter]);
      EEPROM.write(addr, '|');
      addr++;
       }
       
      else if (text[0] == '&')
      {
      Serial.println();
      Serial.print(F("Going to recieve time information "));
      Serial.print(text[1]);
      Serial.println(F(" prescription:"));
      
      int num = text.length();
      Serial.print(F("Time length is  "));
      Serial.println(num-2);   
         
      byte values = 0;
      
      
      for (int i = 2; i < num; i++)
      { values = text[i];
       EEPROM.write(addr, text[i]);
       time[med_counter] = time[med_counter] + text[i];
//     values = EEPROM.read(addr);Serial.print(F("EEPROM address is: "));Serial.println(addr);
       addr++;         
      }
      EEPROM.write(addr, '#');
      med_counter++;
  }
else if (text[0] == '#')
{
  EEPROM_Read();
}

}   

void EEPROM_Read()

{
  int value = 0;
  int address = 0;
  String data;
  char c;
  
  while (value != '#')
  {
  value = EEPROM.read(address);    
  c = value;
  data = data + c;
  Serial.print(address);
  Serial.print("\t");
  Serial.print(value, DEC);
  Serial.print("\t"); 
  Serial.print(c);
  Serial.println();
  address++;
  }
  Serial.print(F("Complete EEPROM data : "));
  Serial.println(data);
  
  RTC_Clock(); 

  }
      
      
 
    

    



  
 

