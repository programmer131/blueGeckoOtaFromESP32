/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
#include "ebl.h"
uint8_t eblsWriteComplete[]={0x03};// write to mention OTA file sent, gecko to take action
uint8_t initiateOTA[]={0x00};// write to initiate OTA process
bool putToDfu=false;
bool performOta=false;
//bool bleScan=true;
static BLEUUID serviceUUID("1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("F7BF3564-FB6D-4E53-88A4-5E37E0326063");
static BLEUUID    charUUID_Data("984227f3-34fc-4045-a5d0-2c581f81a153");

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristic2;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
}
 BLEClient*  pClient  = BLEDevice::createClient();
bool connectToServer(BLEAddress pAddress) {
    Serial.print("Forming a connection to ");
    Serial.println(pAddress.toString().c_str());
    
    // Connect to the remote BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

 // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      return false;
    }
    Serial.println(" - Found our service");
    if(putToDfu)
    {       
        // Obtain a reference to the characteristic in the service of the remote BLE server.
         pRemoteCharacteristic2= pRemoteService->getCharacteristic(charUUID);          
        if (pRemoteCharacteristic2 == nullptr) {
          Serial.print("Failed to find our characteristic UUID: ");
          Serial.println(charUUID_Data.toString().c_str());
          return false;
        }
        Serial.println(" - Found our characteristic");
       // pRemoteCharacteristic->registerForNotify(notifyCallback);
    }
    else if(performOta)
    {             
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID_Data);
        pRemoteCharacteristic2= pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
          Serial.print("Failed to find our characteristic UUID: ");
          Serial.println(charUUID_Data.toString().c_str());
          return false;
        }    
        if (pRemoteCharacteristic2 == nullptr) {
          Serial.print("Failed to find our characteristic UUID: ");
          Serial.println(charUUID_Data.toString().c_str());
          return false;
        }
        Serial.println(" - Found our characteristic");
        //pRemoteCharacteristic->registerForNotify(notifyCallback);
    }
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
    Serial.println(advertisedDevice.getName().c_str());
    if (strncmp(advertisedDevice.getName().c_str(),"FACTORY",7)==0)
    {
      Serial.println("FACTORY device found");       
      Serial.println("./n./n./n./n../n../n.../n.");
      advertisedDevice.getScan()->stop();
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());     
      putToDfu=true;
      doConnect = true;
    }
   else if (strcmp(advertisedDevice.getName().c_str(),"OTA")==0)
     {
      Serial.println("ota device found");       
      advertisedDevice.getScan()->stop();
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      performOta = true;    // Found our server
      doConnect = true;
     }
  } 
}; 
unsigned int len;
unsigned int packetLen=15;
unsigned int index2=0;
char firstRun=1;
void setup() {
  len=sizeof(test);
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);  
} // End of setup.

void scanStart()
{
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);  
}
// This is the Arduino main loop function.
void loop() {
  if(!connected && !putToDfu && !doConnect && !performOta)
  {
   // scanStart();
  }
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    Serial.println("trying to connect");
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; scanning again.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    Serial.println("now in connected loop");
    if(putToDfu)
    {
       pRemoteCharacteristic2->writeValue(initiateOTA,1,true);
       connected=false;
       putToDfu=false;
       delay(5);
       pClient->disconnect(); 
      }
    else if(performOta)
    {
      if(firstRun)
      {
        pRemoteCharacteristic2->writeValue(initiateOTA,1,true);
        firstRun=0;
      }
      else
      {
        //String newValue = "Time since boot: " + String(millis()/1000);
           
        // Set the characteristic's value to be the array of bytes that is actually a string.
        //Serial.println(len);
        Serial.println(index2);
        //Serial.println(packetLen);
	pRemoteCharacteristic->writeValue((uint8_t*)&test[index2],packetLen);
	//pRemoteCharacteristic->writeValue(test+index2,packetLen);
        index2+=packetLen;
        if((len-index2)<=packetLen)
        {
          packetLen=(len-index2);
         // delay(5);
        }      
        if(index2>=len)
        {
           Serial.println("Setting new characteristic over"); 
           pRemoteCharacteristic2->writeValue(eblsWriteComplete,1,true);
           connected=false;
           delay(5);
           pClient->disconnect();           
        }
      }
    }
    delay(5);
  }  
  //delay(1000); // Delay a second between loops.
} // End of loop
