/**************************************************************************/
/*! 
    @file     readMifareClassic.pde
    @author   Adafruit Industries
  @license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    Reads the 4 byte (32 bit) ID of a MiFare Classic card.
    Since the classic cards have only 32 bit identifiers you can stick
  them in a single variable and use that to compare card ID's as a
  number. This doesn't work for ultralight cards that have longer 7
  byte IDs!
   
    Note that you need the baud rate to be 115200 because we need to
  print out the data and read from the card at the same time!

This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI to communicate, 4 required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>


// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  13
#define PN532_MISO 12
#define PN532_MOSI 11
#define PN532_SS1  8
#define PN532_SS2  7
//#define PN532_SS3  6
//#define PN532_SS4  5


#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#endif


Adafruit_PN532 nfc1(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS1);
Adafruit_PN532 nfc2(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS2);
//Adafruit_PN532 nfc3(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS3);
//Adafruit_PN532 nfc4(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS4);


const uint32_t nfcAcceptedTagId1 = 3872679160;
const uint32_t nfcAcceptedTagId2 = 3584511166;
//const uint32_t nfcAcceptedTagId3 = 0;
//const uint32_t nfcAcceptedTagId4 = 0;


void setup(void) {
  Serial.begin(115200);
  Serial.println("Welcome to the Escape Room RFID Gadget!");

  nfc1.begin();
  nfc2.begin();
  //nfc3.begin();
  //nfc4.begin();

  getNfcVersion(&nfc1);
  getNfcVersion(&nfc2);
  //getNfcVersion(&nfc3);
  //getNfcVersion(&nfc4);

  nfc1.SAMConfig();
  nfc2.SAMConfig();
  //nfc3.SAMConfig();
  //nfc4.SAMConfig();

  nfc1.setPassiveActivationRetries(1);
  nfc2.setPassiveActivationRetries(1);
  //nfc3.setPassiveActivationRetries(1);
  //nfc4.setPassiveActivationRetries(1);
  
  Serial.println("System initialized ...");
}


void loop(void) {

  uint32_t nfcTag1 = 0;
  uint32_t nfcTag2 = 0;
  //uint32_t nfcTag3 = 0;
  //uint32_t nfcTag4 = 0;
  
  DEBUG_PRINT("READER #1");
  nfcTag1 = readNfc(&nfc1);
  DEBUG_PRINT(nfcTag1);
  delay(1000);
  
  DEBUG_PRINT("READER #2");
  nfcTag2 = readNfc(&nfc2);
  DEBUG_PRINT(nfcTag2);
  delay(1000);

  /************************
  DEBUG_PRINT("READER #3");
  nfcTag3 = readNfc(&nfc3);
  DEBUG_PRINT(nfcTag3);
  delay(1000);
  
  DEBUG_PRINT("READER #4");
  nfcTag4 = readNfc(&nfc4);
  DEBUG_PRINT(nfcTag4);
  delay(1000);
  ***********************/

  if((nfcTag1 == nfcAcceptedTagId1) && (nfcTag2 == nfcAcceptedTagId2)) // && (nfcTag3 == nfcAcceptedTagId3) && (nfcTag4 == nfcAcceptedTagId4));
  {
    DEBUG_PRINT("All Tags Correct. Activate Relay.");
  }
}


void getNfcVersion(Adafruit_PN532 * nfc)
{
  uint32_t versiondata = nfc->getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); 
  Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); 
  Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); 
  Serial.println((versiondata>>8) & 0xFF, DEC);
}



uint32_t readNfc(Adafruit_PN532 * nfc)
{
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint32_t cardid = 0;
    
  success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    DEBUG_PRINT("Found an ISO14443A card");
    DEBUG_PRINT("  UID Value: ");
    nfc->PrintHex(uid, uidLength);
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3]; 
      DEBUG_PRINT("Seems to be a Mifare Classic card.");
    }
  }
  
  return cardid;
}
