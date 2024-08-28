#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>
#include "BLEDevice.h"

/**************************************************************************************************************************************************************

Code is meant to be run on the client, which is the dev module for user input to set PSI and Launch. 

Program will search for the bluetooth service associated with the launcher, then read/write data to the service in order to control launch and parameters.

Since LVGL refused to play nice, client program can only be used in its current state by reading the serial moniter

Code will only compile on board if the following settings under Tools are changed in Arduino IDE

  Flash Size : "8MB (64Mb)"        PSRAM : "OPI PSRAM"

**************************************************************************************************************************************************************/




int InputPSI = 0;
int TargetPSI = 0;
char HexChart[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

ESP_Panel *panel = nullptr;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      Serial.println("Found Device 1515151");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void SetPSI () {
  /*
    Reads user input to set the PSI to a 3 digit int. When user confirms int value, program will validate int is within predetermined range. 
  */

  int InputNum [3] = {0, 0, 0};
  int Decimals = 0;
  int DisplayNum = 0;

  Serial.print("Setting Target PSI\n");

  while(1) {

    DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];
    Serial.printf("Current Value: %d\n", DisplayNum);
    #if ESP_PANEL_USE_LCD_TOUCH
    panel->getLcdTouch()->readData();

    //Figure out which key was pressed by coordinates
    bool touched = panel->getLcdTouch()->getTouchState();
    if(touched) {
        TouchPoint point = panel->getLcdTouch()->getPoint();
        Serial.printf("Touch point: x %d, y %d\n", point.x, point.y);
        if(point.y <= 300 && point.y >= 225) {
          if(point.x <= 350 && point.x >= 260) {
            Serial.print("Read Cancel, returning\n");

            return;
          }
        }
        if(point.y <= 200 && point.y >= 125) {
          if(point.x <= 350 && point.x >= 260) {
            Serial.print("Read Enter, validating\n");
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            /************************************************************************
              Validation check below, change values here to set limits on launch PSI
            *************************************************************************/
            if(0 <= DisplayNum && DisplayNum <= 80) {
              Serial.print("Input valid, returning\n");
              TargetPSI = DisplayNum;
              return;
            }
            else {
              Serial.print("Input not valid\n");
              continue;
            }
          }
        }
        if(point.y <= 250 && point.y >= 175) {
          if(point.x <= 450 && point.x >= 375) {
            Serial.print("Read Backspace\n");
            InputNum[2] = InputNum[1];
            InputNum[1] = InputNum[0];
            InputNum[0] = 0;
            if (Decimals > 0) {
              Decimals--;
            }
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];
            continue;
          }
        }
        if(point.y <= 300 && point.y >= 225) {
          if(point.x <= 90 && point.x >= 25 && Decimals < 4) {
            Serial.print("1\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 1;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 160 && point.x >= 100 && Decimals < 4) {
            Serial.print("2\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 2;
           
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 240 && point.x >= 180 && Decimals < 4) {
            Serial.print("3\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 3;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
        }
        if(point.y <= 200 && point.y >= 125) {
          if(point.x <= 90 && point.x >= 25 && Decimals < 4) {
            Serial.print("4\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 4;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 160 && point.x >= 100 && Decimals < 4) {
            Serial.print("5\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 5;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 240 && point.x >= 180 && Decimals < 4) {
            Serial.print("6\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 6;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
        }
        if(point.y <= 90 && point.y >= 0) {
          if(point.x <= 90 && point.x >= 25 && Decimals < 4) {
            Serial.print("7\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 7;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 160 && point.x >= 100 && Decimals < 4) {
            Serial.print("8\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 8;
           
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 240 && point.x >= 180 && Decimals < 4) {
            Serial.print("9\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 9;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
          if(point.x <= 320 && point.x >= 260 && Decimals < 4) {
            Serial.print("0\n");
            InputNum[0] = InputNum[1];
            InputNum[1] = InputNum[2];
            InputNum[2] = 0;
            
            DisplayNum = (InputNum[0] * 100) + (InputNum[1] * 10) + InputNum[2];

            Decimals++;
          }
        }
    }
    #endif
    sleep(1);
  }
}

void Launch () {
  Serial.print("Sending Launch Signal\n");
  int Check = 0;
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("WARNING: No Characteristic\n");
    return;
  }
  pRemoteCharacteristic->writeValue("0x300", 5);
  Serial.print("Launch Signal sent\n");
  while(Check) {
    sleep(1);
    std::string received = pRemoteCharacteristic->readValue();
    if ( received.length() > 3) {
      if(received[2] = '1') {
        Check = 1;
        Serial.print("Launch Confirmed\n");
      }
    }
  }
  return;
}

void SendPressure () {
  Serial.print("Sending Pressurize Signal\n");
  int PSIWork = TargetPSI;
  int i;
  char CharArray[6] = { '0', 'x', '4', '0', '0', '\0'};
  int Check = 0;

  i = 0;
  while(PSIWork >= 16) {
    PSIWork = PSIWork - 16;
    i++;
  }
  CharArray[3] = HexChart[i];
  CharArray[4] = HexChart[PSIWork];
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("WARNING: No Characteristic\n");
    return;
  }
  Serial.print("Sending signal: ");
  Serial.print(CharArray);
  pRemoteCharacteristic->writeValue(CharArray, 5);
  Serial.print("Signal Sent\n");
  return;

// Tried to make sure another command cant be sent while the tank is being pressureized, but I wasn't able to successfully debug at 2 am

  /*while(Check) {
    sleep(1);
    std::string received = pRemoteCharacteristic->readValue();
    if ( received.length() > 3) {
      if(received[2] = '1') {
        Check = 1;
        Serial.print("Tank Pressurized\n");
      }
    }
  }
  return;*/
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  Serial.println("Hello Arduino!");
    Serial.println("I am ESP32_Display_Panel.");

#if ESP_PANEL_USE_LCD_TOUCH
    panel = new ESP_Panel();

#if defined(ESP_PANEL_BOARD_ESP32_S3_LCD_EV_BOARD) || defined(ESP_PANEL_BOARD_ESP32_S3_KORVO_2)
    /**
     * These development boards require the use of an IO expander to configure the screen,
     * so it needs to be initialized in advance and registered with the panel for use.
     *
     */
    Serial.println("Initialize IO expander");
    /* Initialize IO expander */
    ESP_IOExpander *expander = new ESP_IOExpander_TCA95xx_8bit(ESP_PANEL_LCD_TOUCH_BUS_HOST_ID, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, ESP_PANEL_LCD_TOUCH_I2C_IO_SCL, ESP_PANEL_LCD_TOUCH_I2C_IO_SDA);
    expander->init();
    expander->begin();
    /* Add into panel */
    panel->addIOExpander(expander);
#endif

    Serial.println("Initialize panel");
    /* Initialize bus and device of panel */
    panel->init();
    /* Start panel */
    panel->begin();
#else
    Serial.println("No touch panel");
#endif

    Serial.println("Setup done");
  // put your setup code here, to run once:

}

void loop() {

  // put your main code here, to run repeatedly:
  //Code for User input
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  if(doScan && !connected){
    BLEDevice::getScan()->start(0);  
  }

    #if ESP_PANEL_USE_LCD_TOUCH
    panel->getLcdTouch()->readData();



    bool touched = panel->getLcdTouch()->getTouchState();
    if(touched) {
        TouchPoint point = panel->getLcdTouch()->getPoint();
        Serial.printf("Touch point: x %d, y %d\n", point.x, point.y);
        if(point.y <= 275 && point.y >= 200) {
          if(point.x <= 250 && point.x >= 50) {
            SendPressure();
          } else if (point.x <= 475 && point.x >= 350) {
            Launch();
          } else if (point.x <= 700 && point.x >= 525) {
            SetPSI();
          }
        }
    }
    #endif
    //Serial.println("Waiting for input on main menu\n");
    sleep(1);
}
