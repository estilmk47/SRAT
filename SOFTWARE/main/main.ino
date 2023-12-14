#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

//BLE server name
#define bleServerName "SRAT+:001"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 16;  // 16 -> 60Hz, 41 -> 24Hz, 100 -> 10Hz, 500 -> 2Hz

bool deviceConnected = false;

BLECharacteristic *pOutputCharacteristic;
BLECharacteristic *pInputCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
      deviceConnected = true;
      digitalWrite(2, HIGH);
      Serial.println("BLE connected");
    }

    void onDisconnect(BLEServer *pServer)
    {
      deviceConnected = false;
      digitalWrite(2, LOW);
      Serial.println("BLE disconnected");
      BLEAdvertising *pAdvertising = pServer->getAdvertising();
      pAdvertising->start();
      Serial.println("BLE init -> waiting for connection");
    }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *pInputCharacteristic){
    Serial.println("TODO");
  }
};

// MPU
MPU6050 mpu6050(Wire);

// LEDs
const int r = 4;
const int g = 16;
const int b = 17;
int rgbRainbow[3] = {0, 0, 0};


void setup() {

  Serial.begin(9600);

  // Pin Setup
  pinMode(13, INPUT);
  pinMode(12, INPUT);
  pinMode(14, INPUT);
  pinMode(27, INPUT);
  pinMode(26, INPUT);
  pinMode(25, INPUT);
  pinMode(33, INPUT);
  pinMode(35, INPUT);
  pinMode(34, INPUT);
  pinMode(2, OUTPUT);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);

  // BLE
  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(BLEUUID("be30f8d4-4711-11ee-be56-0242ac120002"));

  pOutputCharacteristic = pService->createCharacteristic(
      BLEUUID("be30f8d4-4711-11ee-be56-0242ac120003"),
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pOutputCharacteristic->addDescriptor(new BLE2902());

  pInputCharacteristic = pService->createCharacteristic(
      BLEUUID("be30f8d4-4711-11ee-be56-0242ac120004"),
      BLECharacteristic::PROPERTY_WRITE);
  pInputCharacteristic->addDescriptor(new BLE2902());
  pInputCharacteristic->setCallbacks(new CharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("BLE init -> waiting for connection");

  // MPU
  Wire.begin(21, 22); // SDA, SCL
  mpu6050.begin();
}

const int bleCharacteristcDataSize = 6;
uint8_t bleCharacteristcData[bleCharacteristcDataSize] = { 0, 0, 0, 0, 0, 0 };

void loop() {
  // Send over bluetooth 
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    // Read data
    bleCharacteristcData[0] = digitalRead(26) << 0 | digitalRead(25) << 1 | digitalRead(33) << 2 | digitalRead(35) << 3;
    bleCharacteristcData[1] = static_cast<uint8_t>(map(analogRead(13), 0, 4095, 0, 255));
    bleCharacteristcData[2] = static_cast<uint8_t>(map(analogRead(14), 0, 4095, 0, 255));

    if (deviceConnected) {
      pOutputCharacteristic->setValue(&bleCharacteristcData[0], bleCharacteristcDataSize);
      pOutputCharacteristic->notify();
      // TODO: set led light equal to mode
    }
  }
}


    // MPU
    mpu6050.update();  
    float roll = mpu6050.getAngleX();
    float pitch = mpu6050.getAngleY();
    float yaw = mpu6050.getAngleZ();
    uint8_t mappedRoll = map(roll, -90, 90, 0, 255);
    uint8_t mappedPitch = map(pitch, -90, 90, 0, 255);
    uint8_t mappedYaw = map(yaw, -180, 180, 0, 255);
    bleCharacteristcData[3] = mappedRoll;
    bleCharacteristcData[4] = mappedPitch;
    bleCharacteristcData[5] = mappedYaw;

    // Print data
    Serial.println("");
    Serial.print(bleCharacteristcData[0]);
    Serial.print(" - ");
    Serial.print(bleCharacteristcData[1]);
    Serial.print(" - ");
    Serial.print(bleCharacteristcData[2]);
    Serial.print(" --> ");
    Serial.print(analogRead(13));
    Serial.print(" - ");
    Serial.print(analogRead(14));

    // LEDs in disconnected state
    if(!deviceConnected){ // Rainbow
      analogWrite(r, rgbRainbow[0]);
      analogWrite(g, rgbRainbow[1]);
      analogWrite(b, rgbRainbow[2]);
      for(int i = 0; i < 3; i++){
        rgbRainbow[i] += i+1;
        if (rgbRainbow[i] > 255){
          rgbRainbow[i] -= 255;
        }
      }
    }

    if (deviceConnected) {
      pOutputCharacteristic->setValue(&bleCharacteristcData[0], bleCharacteristcDataSize);
      pOutputCharacteristic->notify();
      // TODO: set led light equal to mode
    }
  }
}