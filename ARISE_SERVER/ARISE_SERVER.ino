#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

char CharArray[6];
int i;
char HexChart[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

/***************************************************************************************************************************************************************************************************

Code for the ESP32 that will be wired directly to the launcher and will take commands from the ARISE_CLIENT program

Commands are formatted as a string of hex where the first digit represents the specific function and the rest of the string is used to pass arguments

For example, in the string 0x450, 4 represents the value for the pressurize function. The argument '50' when converted to decimal provides the target value of 80 PSI

    The server will then periodically set the characteristic to 0x0**, where 0 tells the client we're still executing the command, and ** will be the current PSI in hex

    The command values of 0 and 1 are reserved for the server to tell the client whether or not the current command has been executed, and thus if the server is listening for another command.

***************************************************************************************************************************************************************************************************/




BLECharacteristic *pCharacteristic;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string CharArray = pCharacteristic->getValue();

      if (CharArray.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < CharArray.length(); i++)
          Serial.print(CharArray[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void Pressurize (int TargetPSI) {
  int Reading = ReadPSI();
  Serial.print("Starting Pressurize loop\n");
  while(Reading != TargetPSI) {
    Reading = ReadPSI();
    if (Reading > TargetPSI) {
      Serial.print("Above target\n");
      digitalWrite(32, HIGH);
      digitalWrite(25, LOW);
    }
    if (Reading < TargetPSI) {
      Serial.print("Below Target\n");
      digitalWrite(32, LOW);
      digitalWrite(25, HIGH);
    }
    if (Reading == TargetPSI) {
      break;
    }
    i = 0;
    while(Reading >= 16) {
      Reading = Reading - 16;
      i++;
    }

    /*CharArray[0] = '0';
    CharArray[1] = 'x';
    CharArray[2] = '0';
    CharArray[3] = HexChart[i];
    CharArray[4] = HexChart[Reading];
    CharArray[5] = '\0';
    pCharacteristic->setValue(CharArray);*/
    sleep(0.5);
  }
  Serial.print("Reached target, exiting loop\n");
  digitalWrite(32, LOW);
  digitalWrite(25, LOW);
  pCharacteristic->setValue("0x100");
  Serial.print("Returning from Pressurize");
}



void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);

  Serial.println("Starting BLE work!");

  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("0x000");
  pService->start();

  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  pinMode(25, OUTPUT);
  pinMode(32, OUTPUT);

  Serial.print("Setup done, entering loop\n");
}

int ReadPSI () {
  int PSI = map(analogRead(15), 0, 4095, 0, 122);
  Serial.printf("PSI read: %d\n", PSI);
  return PSI;
}

void loop() {
  int PSI;
  std::string received = pCharacteristic->getValue();
  if (received.length() > 3)
  {
    if (received[2] == '2') {
      Serial.print("Get PSI command\n");
      PSI = ReadPSI();
      i = 0;
      while(PSI >= 16) {
        PSI = PSI - 16;
        i++;
      }
      CharArray[0] = '0';
      CharArray[1] = 'x';
      CharArray[2] = '1';
      CharArray[3] = HexChart[i];
      CharArray[4] = HexChart[PSI];
      CharArray[5] = '\0';
      pCharacteristic->setValue(CharArray);
      Serial.print("Get PSI complete\n");
    }
    else if (received[2] == '3') {
      Serial.print("Launch Command\n");
      digitalWrite(32, HIGH);
      sleep(1.5);
      digitalWrite(32, LOW);
      pCharacteristic->setValue("0x100");
      Serial.print("Launch Complete\n");
    }
    else if (received[2] == '4') {
      Serial.print("Pressurize Command");
      for(i = 0; i < 16; i++) {
        if (HexChart[i] == received[3]) {
          PSI = i * 16;
          break;
        }
      }
      for(i = 0; i < 16; i++) {
        if (HexChart[i] == received[4]) {
          PSI = PSI + i;
          break;
        }
      }
      Serial.printf("PSI recieved: %d\n", PSI);
      Pressurize(PSI);
    }
  }
  delay(1000);
}