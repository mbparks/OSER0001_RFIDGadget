/**************************************************************************
  File:    OSER0001_RFID_ArduinoUnoFW
  Author:  mbparks
  Date:    2SEP2019
  Ver.:    0.0.2
  License: MIT

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


const uint32_t NUM_READERS = 2;        //Adjust number as needed 1 to 4
const uint32_t PN532_SCK_PIN =  13;
const uint32_t PN532_MISO_PIN = 12;
const uint32_t PN532_MOSI_PIN = 11;
const uint32_t PN532_SS1_PIN =  8;
const uint32_t PN532_SS2_PIN =  7;
const uint32_t PN532_SS3_PIN = 6;
const uint32_t PN532_SS4_PIN = 5;

const uint32_t ACCEPTED_TAG_ID1 = 3872679160;
const uint32_t ACCEPTED_TAG_ID2 = 3584511166;
//const uint32_t ACCEPTED_TAG_ID3 = 0;
//const uint32_t ACCEPTED_TAG_ID4 = 0;

const uint32_t PWM_OUT_PIN = 3;
const uint32_t OPEN_LED_PIN = 2;
const uint32_t CLOSED_LED_PIN = 4;
const uint32_t PWM_LOW_VAL = 0;
const uint32_t PWM_HIGH_VAL = 255;

const uint32_t DELAY_AMT = 1000;
bool isLocked = true;

const uint32_t SS_PINS[NUM_READERS] = {PN532_SS1_PIN, PN532_SS2_PIN};  //PN532_SS3_PIN, PN532_SS4_PIN};
const uint32_t NFC_ACCEPTED_TAGS[NUM_READERS] = {ACCEPTED_TAG_ID1, ACCEPTED_TAG_ID2};   //ACCEPTED_TAG_ID3, ACCEPTED_TAG_ID4};

Adafruit_PN532 *nfcReaders[NUM_READERS];


/**************************************************************************
 * Function: setup
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/
void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Welcome to the Escape Room RFID Gadget v0.0.1!"));

  for (int x = 0; x < NUM_READERS; x++)
  {
    nfcReaders[x] = new Adafruit_PN532(PN532_SCK_PIN, PN532_MISO_PIN, PN532_MOSI_PIN, SS_PINS[x]);
    nfcReaders[x]->begin();
    //delay(DELAY_AMT*3);
    
    uint32_t versiondata = 9999;
    while (versiondata == 9999) {
      versiondata = nfcReaders[x]->getFirmwareVersion();
      Serial.print(F("Didn't find PN53x board #"));
      Serial.println(x);
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

  pinMode(PWM_OUT_PIN, OUTPUT);
  pinMode(OPEN_LED_PIN, OUTPUT);
  pinMode(CLOSED_LED_PIN, OUTPUT);
  
  analogWrite(PWM_OUT_PIN, PWM_LOW_VAL);
  digitalWrite(OPEN_LED_PIN, LOW);
  digitalWrite(CLOSED_LED_PIN, HIGH);

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
    uint32_t readNfcTags[NUM_READERS];
    uint32_t countOfMatchingTags = 0;

    for (int w = 0; w < NUM_READERS; w++)
    {
      readNfcTags[w] = 0;
    }

    for (int x = 0; x < NUM_READERS; x++) {
      uint8_t success;
      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
      uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
      uint32_t cardid = 0;

      DEBUG_PRINT("READER #");
      DEBUG_PRINTLN(x);

      success = nfcReaders[x]->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

      if (success) {
        // Display some basic information about the card
        DEBUG_PRINT(F("Found an ISO14443A card with "));
        //DEBUG_PRINT(F("  UID Value: "));
        //nfcReaders[x]->PrintHex(uid, uidLength);

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
          //DEBUG_PRINTLN(F("Seems to be a Mifare Classic card."));
        }
      }

      DEBUG_PRINT(F("tag ID#"));
      DEBUG_PRINTLN(cardid);
      readNfcTags[x] = cardid;
      delay(DELAY_AMT);
    }

    for (int z = 0; z < NUM_READERS; z++) {
      if (readNfcTags[z] == NFC_ACCEPTED_TAGS[z])
      {
        countOfMatchingTags++;
      }
    }

    if (countOfMatchingTags == NUM_READERS) {
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
  analogWrite(PWM_OUT_PIN, PWM_HIGH_VAL);
  digitalWrite(OPEN_LED_PIN, HIGH);
  digitalWrite(CLOSED_LED_PIN, LOW);
  isLocked = false;
}
