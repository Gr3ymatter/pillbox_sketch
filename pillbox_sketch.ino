/*********************************************************************
This is an example for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

// This version uses the internal data queing so you can treat it like Serial (kinda)!

#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include <EEPROM.h>

// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK = 13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ 53
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 8

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);
/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  Serial.begin(9600);
  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Print echo demo"));

  BTLEserial.setDeviceName("PillBox"); /* 7 characters max! */

  BTLEserial.begin();
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

void Call()
{
    String text = "";
    
    while (BTLEserial.available()) 
    {
      char c = BTLEserial.read();
      text = text + c;
      Serial.print(c);
    }
      
    if (text[0] == '$')
    {Serial.println();
      Serial.println("Prescription is going to be sent from the phone to the Pill Box");
      Serial.println("Need to log the new prescription to the EEPROM");      
      int num = text.length();
      int pillQty = text[num-1] - 48;
      Serial.print("Prescription has ");
      Serial.print(pillQty);
      Serial.println(" pills");
      
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
      Serial.print("Going to recieve # ");
      Serial.print(text[1]);
      Serial.println(" prescription:");
      
      int num = text.length();
      Serial.print("Prescription length is  ");
      Serial.println(num-2);
      
      byte values = 0;
      
      
      for (int i = 2; i < num; i++)
      {  EEPROM.write(addr, text[i]);
         medicine[med_counter] = medicine[med_counter] + text[i];
         values = EEPROM.read(addr);
         Serial.print("EEPROM address is: ");
         Serial.println(addr);
         Serial.print("The prescription is: ");
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
      Serial.print("Going to recieve time information ");
      Serial.print(text[1]);
      Serial.println(" prescription:");
      
      int num = text.length();
      Serial.print("Time length is  ");
      Serial.println(num-2);   
         
      byte values = 0;
      
      
      for (int i = 2; i < num; i++)
      { values = text[i];
       EEPROM.write(addr, text[i]);
       time[med_counter] = time[med_counter] + text[i];
//     values = EEPROM.read(addr);Serial.print("EEPROM address is: ");Serial.println(addr);
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
  Serial.print("Complete EEPROM data : ");
  Serial.println(data);
  

    


  

    

  }
      
      
 
    

    



  
 

