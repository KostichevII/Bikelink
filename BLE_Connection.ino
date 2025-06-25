#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

#define DISTANCE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SPEED_CHARACTERISTIC_UUID "5172cc35-f93b-4567-b650-b3de954c2104"
#define PASSWORDFIELD_CHARACTERISTIC_UUID "58c600e5-196f-4c47-930a-7f011e87ba17"
#define FLAGS_CHARACTERISTIC_UUID "40487fd2-c1b7-4626-81b5-710b66eba89a"
#define COMMANDS_CHARACTERISTIC_UUID "2f0a9d9a-a778-4aea-b7eb-03d2baa50a0a"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharDistance;
BLECharacteristic *pCharSpeed;
BLECharacteristic *pCharPassfield;
BLECharacteristic *pCharFlags;
BLECharacteristic *pCharCommands;
//-----------------------------------
//String password = "greenhouse";
//-----------------------------------
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Device connected!");
  }
  
  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Device disconnected!");
    pServer->getAdvertising()->start();
  }
};

void setupBLE(){
  //Configuring the BLE
  Serial.println("Starting BLE!");

  BLEDevice::init("BikeLink");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);

  pCharDistance = pService->createCharacteristic(
                      DISTANCE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharSpeed = pService->createCharacteristic(
                      SPEED_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharPassfield = pService->createCharacteristic(
                      PASSWORDFIELD_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharFlags = pService->createCharacteristic(
                      FLAGS_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharCommands = pService->createCharacteristic(
                      COMMANDS_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharFlags->setValue("false");

  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void updateSpeedChar(double newSpeed){
  uint8_t buffer[8];
  memcpy(buffer, &newSpeed, sizeof(newSpeed));
  pCharSpeed->setValue(buffer, sizeof(buffer));
  pCharSpeed->notify();
}

void updateDistanceChar(double newDistance){
  uint8_t buffer[8];
  memcpy(buffer, &newDistance, sizeof(newDistance));
  pCharDistance->setValue(buffer, sizeof(buffer));
  pCharDistance->notify();
}

bool checkPassword(){
  String pswd = pCharPassfield->getValue();
  //-------------------------------
  String stringPassword="";
  int size=sizeof(passwordInputed)/sizeof(passwordInputed[0]);
  for(int i=0; i< size; i++){
    stringPassword+=String(passwordInputed[i]);
  }
  //----------------------------
  if(pswd == stringPassword){
    return true;
    pCharPassfield->setValue("");
  }
  else
    return false;
}

void setFlags(bool security){
  if(security)
    pCharFlags->setValue("true");
  else
    pCharFlags->setValue("false");
  pCharFlags->notify();
}

String getCommand(){
  String command = pCharCommands->getValue();
  pCharCommands->setValue("");
  return command;
}