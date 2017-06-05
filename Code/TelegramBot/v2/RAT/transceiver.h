/*H========================================================================
* FILENAME :        transeiver.h
*
* DESCRIPTION :
*       All functions to retrieve data from the transceiver.
* 
* AUTHORS :    RLP Group 09-03
*                 - Daniel Azemar   dani.azemar@gmail.com
*                 - Jialuo Chen     
*                 - Sergi Pous
*                 - Adria Rico
*                 
* START DATE :    Feb 2017
* 
*========================================================================H*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

const byte address[5] = {'x','R','A','T','x'};
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup_transceiver(){
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
}

bool newData = false;
int data;
float dataFloat;

int getData() {
  //Gets a byte of data
  newData = false;
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    newData = true;
  }
  return data;
}

float getDataFloat() {
  //Gets 4 bytes of data (used with location: latitude and longitude)
  byte flo[4];
  for(int i = 0; i<sizeof(float); i++){
    uint32_t maxMillis = millis() + 10000;
    while (!radio.available() && millis() < maxMillis);
    flo[i] = getData();
    Serial.println((int)flo[i]);
  }
  
  return *((float*)flo);
}

