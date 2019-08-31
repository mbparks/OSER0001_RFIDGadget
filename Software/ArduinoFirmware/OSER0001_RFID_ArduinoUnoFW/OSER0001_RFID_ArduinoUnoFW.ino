/**************************************************************************
  File:    OSER0001_RFID_ArduinoUnoFW
  Author:  mbparks
  Date:    30AUG2019
  Ver.:    0.0.1
  License: BSD

  This code is four gadget that can be used in an escape room .
  It will activate a relay once all RFID tagged objects are placed
  over the correct PN532 RFID readers.
  Supports 1 through 4 RFID readers.

  Based on example code for PN532 by Adafruit Industries
  
  This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
  This library works with the Adafruit NFC breakout
  ----> https://www.adafruit.com/products/364

  Check out the links above for our tutorials and wiring diagrams
  These chips use SPI to communicate, 4 required to interface

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

**************************************************************************/
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

#define NUMREADERS 2

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print (x)
#else
#define DEBUG_PRINT(x)
#endif

#ifdef DEBUG
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#endif

Adafruit_PN532 *nfcReaders[NUMREADERS];

const uint32_t ssPins[NUMREADERS] = {PN532_SS1, PN532_SS2};  //PN532_SS3, PN532_SS4};
const uint32_t nfcAcceptedTags[NUMREADERS] = {3872679160, 3584511166};   //0, 0};

bool isLocked = true;


/**************************************************************************
 * Function: setup
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/

void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Welcome to the Escape Room RFID Gadget!"));

  for (int x = 0; x < NUMREADERS; x++)
  {
    nfcReaders[x] = new Adafruit_PN532(PN532_SCK, PN532_MISO, PN532_MOSI, ssPins[x]);
    nfcReaders[x]->begin();
    uint32_t versiondata = nfcReaders[x]->getFirmwareVersion();
    if (! versiondata) {
      Serial.print(F("Didn't find PN53x board"));
      while (1); // halt
    }
    // Got ok data, print it out!
    Serial.print(F("Found chip PN5"));
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print(F("Firmware ver. "));
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    nfcReaders[x]->SAMConfig();
    nfcReaders[x]->setPassiveActivationRetries(1);
  }

  Serial.println(F("System initialized ..."));
}



/**************************************************************************
 * Function: loop
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/
void loop(void) {
  if (isLocked == true)
  {
    uint32_t readNfcTags[NUMREADERS] = {0, 0};
    uint32_t countOfMatchingTags = 0;

    for (int x = 0; x < NUMREADERS; x++) {
      uint8_t success;
      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
      uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
      uint32_t cardid = 0;

      DEBUG_PRINT("READER #");
      DEBUG_PRINTLN(x);

      success = nfcReaders[x]->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

      if (success) {
        // Display some basic information about the card
        DEBUG_PRINTLN(F("Found an ISO14443A card"));
        DEBUG_PRINTLN(F("  UID Value: "));
        nfcReaders[x]->PrintHex(uid, uidLength);

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
          DEBUG_PRINTLN(F("Seems to be a Mifare Classic card."));
        }
      }

      DEBUG_PRINT(F("Tag ID: "));
      DEBUG_PRINTLN(cardid);
      readNfcTags[x] = cardid;
      delay(1000);
    }

    for (int z = 0; z < NUMREADERS; z++) {
      if (readNfcTags[z] == nfcAcceptedTags[z])
      {
        countOfMatchingTags++;
      }
    }

    if (countOfMatchingTags == NUMREADERS) {
      activateRelay();
    }
  }
}

/**************************************************************************
 * Function: activateRelay
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/
void activateRelay()
{
  DEBUG_PRINTLN(F("ALERT!!!  ALERT!!! All Tags Correct. Activate Relay.  ALERT!!! ALERT !!!"));
  isLocked = false;
}
