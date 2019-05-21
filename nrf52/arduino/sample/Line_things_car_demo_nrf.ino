#include <bluefruit.h>

#include "Car.h"

// Device Name: Maximum 30 bytes
#define DEVICE_NAME "LINE Things TH Demo Car"

// User service UUID: Change this to your generated service UUID
#define USER_SERVICE_UUID "1e0eb241-d8a5-4c0e-8ded-3ae4fadc9efa"

#define WRITE_CONTROL_CHARACTERISTIC_UUID "cf2316b5-96ea-4939-a5e2-f88ef3d91e39"
//#define WRITE_DIRECTION_CHARACTERISTIC_UUID "fbfeb748-be33-4896-9f0f-8cd5a86ea961"
//#define WRITE_SPEED_CHARACTERISTIC_UUID "ff4fb94b-98b5-405f-9d4d-1d97b2b12263"
#define NOTIFY_LEFT_SENSOR_CHARACTERISTIC_UUID "5380b1d9-4d37-47b1-ac48-ead2df7b7135"
#define NOTIFY_SENSOR_CHARACTERISTIC_UUID "6c626864-9ae0-4054-822c-1130d8f6f45c"

// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "E625601E-9E55-4597-A598-76018A0D293D"
#define PSDI_CHARACTERISTIC_UUID "26E2B12B-85F0-4F3F-9FDD-91D114270E6E"

#define MOTORAF 20
#define MOTORAB 21
#define MOTORBF 22
#define MOTORBB 23

#define SENSORL A0
#define SENSORR A1

uint8_t userServiceUUID[16];
uint8_t psdiServiceUUID[16];
uint8_t psdiCharacteristicUUID[16];
uint8_t writeControlCharacteristicUUID[16];
//uint8_t writeDirectionCharacteristicUUID[16];
//uint8_t writeSpeedCharacteristicUUID[16];
uint8_t notifyLeftSensorCharacteristicUUID[16];
uint8_t notifySensorCharacteristicUUID[16];

BLEService userService;
BLEService psdiService;
BLECharacteristic psdiCharacteristic;

BLECharacteristic* writeControlCharacteristic;
//BLECharacteristic writeDirectionCharacteristic;
//BLECharacteristic writeSpeedCharacteristic;
BLECharacteristic* notifyLeftSensorCharacteristic;
BLECharacteristic* notifySensorCharacteristic;

Car* car;

bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t leftInfrared = 0;
uint8_t rightInfrared = 0;
char controlDirection = 0;

int incomingByte = 0;

void writeControlCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  int direction = data[0];
  int speed = data[1];

  Serial.printf("direction %d, speed %d", direction, speed); 

  switch(direction) {
    case 0:
    case 2:
    case 5:
      car->setWheelsStraight();
      //Serial.printf("set wheels straight\n");
      break;
    case 1:
    case 6:
      //Serial.printf("turn right\n");
      car->turnRight(5);
      break;
    case 3:
    case 4:
      //Serial.printf("turn left\n");
      car->turnLeft(5);
      break;
    default:
      Serial.println("Direction Invalid\n");
  }

  if(direction <= 3) {
    //Serial.printf("go forward speed %d\n", speed);
    car->goForward(speed);
  }
  else {
    //Serial.printf("go backward\n");
    car->goBackward(speed);
  }
}

void sensorNotify() {
  leftInfrared = car->readLeftSensor();
  rightInfrared = car->readRightSensor();
  //Serial.printf("leftInfrared %d, rightInfrared %d \n", leftInfrared, rightInfrared);
  //notifyLeftSensorCharacteristic->notify(&leftInfrared, sizeof(leftInfrared));
  //delay(20);

  char sensor[] = { (char)leftInfrared, (char)rightInfrared };
  notifySensorCharacteristic->notify(sensor, sizeof(char)*2);
  
}


void setup() {
  Serial.begin(115200);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb
  delay(1000);

  Serial.println("setup ... ");

  car = new Car(MOTORAF, MOTORAB, MOTORBF, MOTORBB, SENSORL, SENSORR);

  Bluefruit.begin();
  Bluefruit.setName(DEVICE_NAME);

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  setupServices();
  startAdvertising();
  Serial.println("Ready to Connect");

  car->stopAndSetWheelsStraight();
}

void loop() {
  uint8_t btnValue;

  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }

  if (deviceConnected) {
    sensorNotify();
    delay(100);
  }

  /*
  // Disconnection
  if (!deviceConnected && oldDeviceConnected) r{
    delay(500); // Wait for BLE Stack to be ready
    thingsServer->startAdvertising(); // Restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // Connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  */
}

void setupServices(void) {
  // Convert String UUID to raw UUID bytes
  strUUID2Bytes(USER_SERVICE_UUID, userServiceUUID);
  strUUID2Bytes(PSDI_SERVICE_UUID, psdiServiceUUID);
  strUUID2Bytes(PSDI_CHARACTERISTIC_UUID, psdiCharacteristicUUID);
  strUUID2Bytes(WRITE_CONTROL_CHARACTERISTIC_UUID, writeControlCharacteristicUUID);
  //strUUID2Bytes(WRITE_DIRECTION_CHARACTERISTIC_UUID, writeDirectionCharacteristicUUID);
  //strUUID2Bytes(WRITE_SPEED_CHARACTERISTIC_UUID, writeSpeedCharacteristicUUID);
  strUUID2Bytes(NOTIFY_LEFT_SENSOR_CHARACTERISTIC_UUID, notifyLeftSensorCharacteristicUUID);
  strUUID2Bytes(NOTIFY_SENSOR_CHARACTERISTIC_UUID, notifySensorCharacteristicUUID);

  // Setup User Service
  userService = BLEService(userServiceUUID);
  userService.begin();

  // Setup write characteristic (Direction / Speed)
  writeControlCharacteristic = new BLECharacteristic(writeControlCharacteristicUUID);
  writeControlCharacteristic->setProperties(CHR_PROPS_WRITE);
  writeControlCharacteristic->setWriteCallback(writeControlCallback);
  writeControlCharacteristic->setPermission(SECMODE_ENC_NO_MITM, SECMODE_ENC_NO_MITM);
  writeControlCharacteristic->setFixedLen(2);
  writeControlCharacteristic->begin();

  // Setup notify characteristic (sensor left right)
  notifyLeftSensorCharacteristic = new BLECharacteristic(notifyLeftSensorCharacteristicUUID);
  notifyLeftSensorCharacteristic->setProperties(CHR_PROPS_NOTIFY);
  notifyLeftSensorCharacteristic->setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
  notifyLeftSensorCharacteristic->setFixedLen(1);
  notifyLeftSensorCharacteristic->begin();

  notifySensorCharacteristic = new BLECharacteristic(notifySensorCharacteristicUUID);
  notifySensorCharacteristic->setProperties(CHR_PROPS_NOTIFY);
  notifySensorCharacteristic->setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
  notifySensorCharacteristic->setFixedLen(sizeof(char) * 2);
  notifySensorCharacteristic->begin();
  
  // Setup PSDI Service
  psdiService = BLEService(psdiServiceUUID);
  psdiService.begin();

  psdiCharacteristic = BLECharacteristic(psdiCharacteristicUUID);
  psdiCharacteristic.setProperties(CHR_PROPS_READ);
  psdiCharacteristic.setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
  psdiCharacteristic.setFixedLen(sizeof(uint32_t) * 2);
  psdiCharacteristic.begin();

  // Set PSDI (Product Specific Device ID) value
  uint32_t deviceAddr[] = { NRF_FICR->DEVICEADDR[0], NRF_FICR->DEVICEADDR[1] };
  psdiCharacteristic.write(deviceAddr, sizeof(deviceAddr));
}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  deviceConnected = true;
  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  deviceConnected = false;
  Serial.println("Disconnected");
  Serial.println("Advertising!");
}

void startAdvertising(void) {
  // Start Advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(userService);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start(0);
}

// UUID Converter
void strUUID2Bytes(String strUUID, uint8_t binUUID[]) {
  String hexString = String(strUUID);
  hexString.replace("-", "");

  for (int i = 16; i != 0 ; i--) {
    binUUID[i - 1] = hex2c(hexString[(16 - i) * 2], hexString[((16 - i) * 2) + 1]);
  }
}

char hex2c(char c1, char c2) {
  return (nibble2c(c1) << 4) + nibble2c(c2);
}

char nibble2c(char c) {
  if ((c >= '0') && (c <= '9'))
    return c - '0';
  if ((c >= 'A') && (c <= 'F'))
    return c + 10 - 'A';
  if ((c >= 'a') && (c <= 'f'))
    return c + 10 - 'a';
  return 0;
}
