#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define TCAADDR       0x70
#define BUTTON_PIN    7
#define LED_PIN       25
#define NUM_SAMPLES   50


Adafruit_MPU6050 mpu;

//long int t1;

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


void setup(void) {

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Wire.setSDA(4);
  Wire.setSCL(5);
  //Wire.setClock(600);
  //Wire.setTimeout(1);
  Wire.begin();

  Serial.begin(115200);
  // while (!Serial) {
  //   delay(10); // will pause Zero, Leonardo, etc until serial console opens
  // }


  // Serial.println("\nTCA9548A Scanner ready!");

  // for (uint8_t t = 0; t < 8; t++) {
  //   selectChannel(t);
  //   Serial.print("TCA Port #"); Serial.println(t);

  //   for (uint8_t addr = 0; addr <= 127; addr++) {
      
  //     // Don't report on the TCA9548A itself!
  //     if (addr == TCAADDR) continue;

  //     // See whether a device is on this address
  //     Wire.beginTransmission(addr);

  //     // See if something acknowledged the transmission
  //     int response = Wire.endTransmission();
  //     if (response == 0) {
  //       Serial.print("Found I2C 0x");  Serial.println(addr, HEX);
  //     }
  //   }

  //   // Slow the loop scanner down a bit
  //   delay(1000);
  // }
  // Serial.println("\nScan completed.");

  Wire.beginTransmission(TCAADDR);
  Wire.write(0xFF);
  Wire.endTransmission();

  // for (uint8_t t = 0; t < 6; t++) {
  //   Serial.println("+1");}
  for (uint8_t t = 0; t < 6; t++) {
    selectChannel(t);
    //if (t==5){selectChannel(6);}
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
    delay(100);
    //Serial.println("");
    
  }
  
  // selectChannel(1);

  // if (!mpu.begin()) {
  //   Serial.println("Failed to find MPU6050 chip");
  //   while (1) {
  //     delay(10);
  //   }
  // }


  delay(100);
  digitalWrite(LED_PIN, LOW);
  //Serial.println("timestamp,a0x,a0y,a0z,g0x,g0y,g0z,a1x,a1y,a1z,g1x,g1y,g1z,a2x,a2y,a2z,g2x,g2y,g2z,a3x,a3y,a3z,g3x,g3y,g3z,a4x,a4y,a4z,g4x,g4y,g4z,a5x,a5y,a5z,g5x,g5y,g5z");
  //t1 = millis();
}

void loop() {
  // long int t1,t2 = millis();
  //while ((t2-t1) <= 1000){
  while (digitalRead(BUTTON_PIN) == 1);
  //_________START_READING__________
  Serial.println("timestamp,a0x,a0y,a0z,g0x,g0y,g0z,a1x,a1y,a1z,g1x,g1y,g1z,a2x,a2y,a2z,g2x,g2y,g2z,a3x,a3y,a3z,g3x,g3y,g3z,a4x,a4y,a4z,g4x,g4y,g4z,a5x,a5y,a5z,g5x,g5y,g5z");
  digitalWrite(LED_PIN, HIGH);

  long int timestamp, start_time = millis();

  for (int i = 0; i < NUM_SAMPLES; i++) {
    timestamp = millis();
    Serial.print(timestamp - start_time);
    Serial.print(",");
  
    for (uint8_t i = 0; i<6; i++){

      selectChannel(i);
      //if (i==5){selectChannel(6);}
    
      mpu.getEvent(&a, &g, &temp);

      /* Print out the values */
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
      if (i == 5){Serial.print("");}
      else {Serial.print(",");}
      //delay(10);
    }
    Serial.println();
  }
  Serial.println();
  digitalWrite(LED_PIN, LOW);
  while (digitalRead(BUTTON_PIN) == 0);
}