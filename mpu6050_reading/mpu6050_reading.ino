#include "Wire.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

#define TCAADDR 0x70
#define BUTTON_PIN    7
#define LED_PIN       25
//#define MPUADDR 0x68

Adafruit_MPU6050 mpu;

void selectChannel(uint8_t channel) {
  Wire.begin();
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

sensors_event_t a, g, temp;
double calib_offset[6][3] = {{0.12, 0.02, 0.0},
                          {0.05, 0.03,-0.03},
                          {0.05,-0.01, 0.02},
                          {0.09,-0.01, 0.01},
                          {0.12, 0.03, 0.00},
                          {0.05, 0.00, 0.02}};

// standard Arduino setup()
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  Wire.setSDA(4);
  Wire.setSCL(5);
  //Wire.setClock(600);
  Wire.begin();
    
  Serial.println("\nTCA9548A Scanner ready!");


  /* TCA scanning */
  
  
  for (uint8_t t = 0; t < 8; t++) {
    selectChannel(t);
    //if (t == 5){selectChannel(6);}
    Serial.print("TCA Port #"); Serial.println(t);

    for (uint8_t addr = 0; addr <= 127; addr++) {
      
      // Don't report on the TCA9548A itself!
      if (addr == TCAADDR) continue;

      // See whether a device is on this address
      Wire.begin();
      Wire.beginTransmission(addr);

      // See if something acknowledged the transmission
      int response = Wire.endTransmission();
      //Serial.println(response);
      if (response == 0) {
        Serial.print("Found I2C 0x");  Serial.println(addr, HEX);
      }
      // else{
      //   Serial.println("I2C went wrong!!!");
      //   while(1)
      //   {Serial.println("I2C went wrong!!!");delay(1000);}
      //}
    }

    // Slow the loop scanner down a bit
    delay(100);
  }
  Serial.println("\nScan completed.");

  Wire.beginTransmission(TCAADDR);
  Wire.write(0xFF);
  Wire.endTransmission();

  for (uint8_t t = 0; t < 6; t++) {
    selectChannel(t);
    //if ( t == 5) {selectChannel(6);}
    // Try to initialize!
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      while (1) {
        delay(10);
      }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    //Serial.println("");
    
  }
  Serial.println("Init successfully!!!");
  
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  
}

void loop() {
  
  /* Get new sensor events with the readings */
  

  //sensors_event_t a, g, temp;
  for (uint8_t i = 0; i<6; i++)
  {
    selectChannel(i);
    //if ( i == 5) {selectChannel(6);}
    mpu.getEvent(&a, &g, &temp);
    Serial.print(a.acceleration.x);
    Serial.print(",");
    Serial.print(a.acceleration.y);
    Serial.print(",");
    Serial.print(a.acceleration.z);
    Serial.print(",");
    Serial.print(g.gyro.x + calib_offset[i][0]);
    Serial.print(",");
    Serial.print(g.gyro.y + calib_offset[i][1]);
    Serial.print(",");
    Serial.print(g.gyro.z + calib_offset[i][2]);
    if (i == 5){Serial.println("");}
    else {Serial.print(",");}
    //delay(100);
  }

  //delay(10);
  
}