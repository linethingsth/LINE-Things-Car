#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Device Name: Maximum 30 bytes
#define DEVICE_NAME "LINE Things Trial ESP32"

// User service UUID: Change this to your generated service UUID
#define USER_SERVICE_UUID "36752cf3-739c-4625-aea1-4d47a70ad656"
// User service characteristics
#define WRITE_CHARACTERISTIC_UUID "E9062E71-9E62-4BC6-B0D3-35CDCD9B027B" // LED
#define NOTIFY_CHARACTERISTIC_UUID "62FBD229-6EDD-4D1A-B554-5C4E1BB29169" // button

#define WRITE_DIRECTION_CHARACTERISTIC_UUID "fbfeb748-be33-4896-9f0f-8cd5a86ea961"
#define WRITE_SPEED_CHARACTERISTIC_UUID "ff4fb94b-98b5-405f-9d4d-1d97b2b12263"
#define NOTIFY_LEFT_SENSOR_CHARACTERISTIC_UUID "5380b1d9-4d37-47b1-ac48-ead2df7b7135"
#define NOTIFY_RIGHT_SENSOR_CHARACTERISTIC_UUID "6c626864-9ae0-4054-822c-1130d8f6f45c"

// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "E625601E-9E55-4597-A598-76018A0D293D"
#define PSDI_CHARACTERISTIC_UUID "26E2B12B-85F0-4F3F-9FDD-91D114270E6E"

#define BUTTON 0
#define LED1 2

BLEServer* thingsServer;
BLESecurity *thingsSecurity;
BLEService* userService;
BLEService* psdiService;
BLECharacteristic* psdiCharacteristic;
BLECharacteristic* writeCharacteristic;
BLECharacteristic* notifyCharacteristic;

BLECharacteristic* writeDirectionCharacteristic;
BLECharacteristic* writeSpeedCharacteristic;
BLECharacteristic* notifyLeftSensorCharacteristic;
BLECharacteristic* notifyRightSensorCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;

volatile int btnAction = 0;

uint8_t leftInfrared = 0;
uint8_t rightInfrared = 0;

int incomingByte = 0;

class serverCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class writeCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *bleWriteCharacteristic) {
      std::string value = bleWriteCharacteristic->getValue();
      if ((char)value[0] <= 1) {
        digitalWrite(LED1, (char)value[0]);
      }
    }
};

class writeDirectionCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *bleWriteCharacteristic) {
      std::string value = bleWriteCharacteristic->getValue();

      // value 0-6
      Serial.println("Direction" + (char)value[0]);

      switch((char)value[0]) {
        case 0:
          Serial.println("Direction = 0");
          break;
        case 1:
          Serial.println("Direction = 1");
          break;
        case 2:
          Serial.println("Direction = 2");
          break;
        case 3:
          Serial.println("Direction = 3");
          break;
        case 4:
          Serial.println("Direction = 4");
          break;
        case 5:
          Serial.println("Direction = 5");
          break;
        case 6:
          Serial.println("Direction = 6");
          break;
        default:
          Serial.println("Direction Invalid");
      }
    }
};

class writeSpeedCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *bleWriteCharacteristic) {
      std::string value = bleWriteCharacteristic->getValue();

      // value 0-5
      Serial.println("Speed " + (int)value[0]);

      switch((char)value[0]) {
        case 0:
          Serial.println("Speed = 0");
          break;
        case 1:
          Serial.println("Speed = 1");
          break;
        case 2:
          Serial.println("Speed = 2");
          break;
        case 3:
          Serial.println("Speed = 3");
          break;
        case 4:
          Serial.println("Speed = 4");
          break;
        case 5:
          Serial.println("Speed = 5");
          break;
        default:
          Serial.println("Speed Invalid");
      }

    }
};

void sensorNotify() {
  leftInfrared = map(4095 - (uint16_t)analogRead(A0), 0, 4095, 0, 100);
  rightInfrared = map(4095 - (uint16_t)analogRead(A3), 0, 4095, 0, 100);

//  Serial.printf("analog A0: %d", (uint16_t)analogRead(A0));
//  Serial.println("leftInfrared: " + leftInfrared);

  notifyLeftSensorCharacteristic->setValue(&leftInfrared, 1);
  notifyLeftSensorCharacteristic->notify();

  delay(15);

  notifyRightSensorCharacteristic->setValue(&rightInfrared, 1);
  notifyRightSensorCharacteristic->notify();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, 0);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(BUTTON, buttonAction, CHANGE);
  pinMode(A0, INPUT);
  pinMode(A3, INPUT);
  BLEDevice::init("");
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);

  // Security Settings
  BLESecurity *thingsSecurity = new BLESecurity();
  thingsSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
  thingsSecurity->setCapability(ESP_IO_CAP_NONE);
  thingsSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  setupServices();
  startAdvertising();
  Serial.println("Ready to Connect");
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

  while (btnAction > 0 && deviceConnected) {
    btnValue = !digitalRead(BUTTON);
    btnAction = 0;
    notifyCharacteristic->setValue(&btnValue, 1);
    notifyCharacteristic->notify();
    Serial.println("read button");
    delay(20);
  }

  if (deviceConnected) {
    sensorNotify();
    delay(100);
  }

  // Disconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Wait for BLE Stack to be ready
    thingsServer->startAdvertising(); // Restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // Connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void setupServices(void) {
  // Create BLE Server
  thingsServer = BLEDevice::createServer();
  thingsServer->setCallbacks(new serverCallbacks());

  // Setup User Service
  userService = thingsServer->createService(USER_SERVICE_UUID);

  // Create Characteristics for User Service
  writeCharacteristic = userService->createCharacteristic(WRITE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  writeCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
//  writeCharacteristic->setCallbacks(new writeCallback());

  //========= Write Characteristic-=========

  // Create Direction Characteristics for User Service
  writeDirectionCharacteristic = userService->createCharacteristic(WRITE_DIRECTION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  writeDirectionCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  writeDirectionCharacteristic->setCallbacks(new writeDirectionCallback());

  // Create Speed Characteristics for User Service
  writeSpeedCharacteristic = userService->createCharacteristic(WRITE_SPEED_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  writeSpeedCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  writeSpeedCharacteristic->setCallbacks(new writeSpeedCallback());



  notifyCharacteristic = userService->createCharacteristic(NOTIFY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  notifyCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  //  BLE2902* ble9202 = new BLE2902();
  //  ble9202->setNotifications(true);
  //  ble9202->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  //  notifyCharacteristic->addDescriptor(ble9202);

  //========= Notify Characteristic-=========

  notifyLeftSensorCharacteristic = userService->createCharacteristic(NOTIFY_LEFT_SENSOR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  notifyLeftSensorCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  BLE2902* ble9202_leftsensor = new BLE2902();
  ble9202_leftsensor->setNotifications(true);
  ble9202_leftsensor->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  notifyLeftSensorCharacteristic->addDescriptor(ble9202_leftsensor);

  notifyRightSensorCharacteristic = userService->createCharacteristic(NOTIFY_RIGHT_SENSOR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  notifyRightSensorCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  BLE2902* ble9202_rightsensor = new BLE2902();
  ble9202_rightsensor->setNotifications(true);
  ble9202_rightsensor->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  notifyRightSensorCharacteristic->addDescriptor(ble9202_rightsensor);

  // Setup PSDI Service
  psdiService = thingsServer->createService(PSDI_SERVICE_UUID);
  psdiCharacteristic = psdiService->createCharacteristic(PSDI_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  psdiCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  // Set PSDI (Product Specific Device ID) value
  uint64_t macAddress = ESP.getEfuseMac();
  psdiCharacteristic->setValue((uint8_t*) &macAddress, sizeof(macAddress));

  // Start BLE Services
  userService->start();
  psdiService->start();
}

void startAdvertising(void) {
  // Start Advertising
  BLEAdvertisementData scanResponseData = BLEAdvertisementData();
  scanResponseData.setFlags(0x06); // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04
  scanResponseData.setName(DEVICE_NAME);

  thingsServer->getAdvertising()->addServiceUUID(userService->getUUID());
  thingsServer->getAdvertising()->setScanResponseData(scanResponseData);
  thingsServer->getAdvertising()->start();
}

void buttonAction() {
  btnAction++;
}
