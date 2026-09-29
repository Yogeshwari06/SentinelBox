#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// =====================================================
//                  SENTINELBOX
// =====================================================

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);


// =====================================================
// MPU6050
// =====================================================

#define MPU_ADDRESS 0x68

float accX = 0.0;
float accY = 0.0;
float accZ = 0.0;

float baseX = 0.0;
float baseY = 0.0;
float baseZ = 1.0;


// =====================================================
// PASSIVE BUZZER
// =====================================================

#define BUZZER_PIN 26

#define BUZZER_FREQUENCY 5000


// =====================================================
// MANUAL TEMPERATURE & HUMIDITY
// =====================================================

float manualTemperature = 29.4;
float manualHumidity = 65.2;


// =====================================================
// SAFE LIMITS
// =====================================================

#define TEMP_MIN 15.0
#define TEMP_MAX 35.0

#define HUM_MIN 30.0
#define HUM_MAX 80.0


// =====================================================
// MOVEMENT SENSITIVITY
// =====================================================

#define MOVEMENT_THRESHOLD 0.30


// =====================================================
// DOOR DEMONSTRATION TIMER
// =====================================================

// After 15 seconds → Door OPEN
#define DOOR_DEMO_DELAY 15000

// Door remains OPEN for 5 seconds
#define DOOR_OPEN_DURATION 5000

unsigned long demoStartTime;


// =====================================================
// MPU6050 READING
// =====================================================

bool readMPU()
{
  Wire.beginTransmission(MPU_ADDRESS);

  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  int count =
    Wire.requestFrom(
      MPU_ADDRESS,
      6,
      true
    );

  if (count != 6)
  {
    return false;
  }

  int16_t rawX =
    ((int16_t)Wire.read() << 8) |
    Wire.read();

  int16_t rawY =
    ((int16_t)Wire.read() << 8) |
    Wire.read();

  int16_t rawZ =
    ((int16_t)Wire.read() << 8) |
    Wire.read();


  // MPU6050 ±2g range

  accX =
    rawX / 16384.0;

  accY =
    rawY / 16384.0;

  accZ =
    rawZ / 16384.0;

  return true;
}


// =====================================================
// MPU CALIBRATION
// =====================================================

void calibrateMPU()
{
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;

  int count = 0;

  Serial.println();
  Serial.println("================================");
  Serial.println("MPU6050 CALIBRATION");
  Serial.println("KEEP THE BOX COMPLETELY STILL");
  Serial.println("================================");


  for (int i = 0; i < 50; i++)
  {
    if (readMPU())
    {
      sumX += accX;
      sumY += accY;
      sumZ += accZ;

      count++;
    }

    delay(20);
  }


  if (count > 0)
  {
    baseX =
      sumX / count;

    baseY =
      sumY / count;

    baseZ =
      sumZ / count;


    Serial.println("CALIBRATION COMPLETE");

    Serial.print("Base X: ");
    Serial.println(baseX, 3);

    Serial.print("Base Y: ");
    Serial.println(baseY, 3);

    Serial.print("Base Z: ");
    Serial.println(baseZ, 3);
  }
  else
  {
    Serial.println("CALIBRATION FAILED");

    baseX = 0;
    baseY = 0;
    baseZ = 1;
  }
}


// =====================================================
// MOVEMENT DIFFERENCE
// =====================================================

float getMovementDifference()
{
  float dx =
    accX - baseX;

  float dy =
    accY - baseY;

  float dz =
    accZ - baseZ;


  return sqrt(
    dx * dx +
    dy * dy +
    dz * dz
  );
}


// =====================================================
// BUZZER
// =====================================================

void beep()
{
  tone(
    BUZZER_PIN,
    BUZZER_FREQUENCY
  );

  delay(300);

  noTone(
    BUZZER_PIN
  );

  delay(150);
}


// =====================================================
// STARTUP SCREEN
// =====================================================

void showStartup()
{
  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(2);

  display.setCursor(5, 5);

  display.println("SentinelBox");


  display.setTextSize(1);

  display.setCursor(25, 34);

  display.println("SMART STORAGE");


  display.setCursor(35, 48);

  display.println("SYSTEM READY");


  display.display();

  delay(2000);
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(21, 22);

  Wire.setClock(100000);


  // ===================================================
  // BUZZER
  // ===================================================

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  noTone(
    BUZZER_PIN
  );


  // ===================================================
  // OLED
  // ===================================================

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS))
  {
    Serial.println("OLED ERROR");

    while (true);
  }


  // ===================================================
  // STARTUP
  // ===================================================

  showStartup();


  // ===================================================
  // WAKE MPU6050
  // ===================================================

  Wire.beginTransmission(
    MPU_ADDRESS
  );

  Wire.write(0x6B);
  Wire.write(0x00);

  Wire.endTransmission();

  delay(100);


  // ===================================================
  // CALIBRATE MPU
  // ===================================================

  calibrateMPU();


  // ===================================================
  // START DOOR DEMO TIMER
  // ===================================================

  demoStartTime = millis();


  // ===================================================
  // SERIAL INFORMATION
  // ===================================================

  Serial.println();
  Serial.println("================================");
  Serial.println("       SENTINELBOX READY");
  Serial.println("================================");

  Serial.print("Manual Temperature: ");
  Serial.print(manualTemperature);
  Serial.println(" C");

  Serial.print("Manual Humidity: ");
  Serial.print(manualHumidity);
  Serial.println(" %");

  Serial.println("MPU6050: WORKING");
  Serial.println("Passive Buzzer: GPIO26");
  Serial.println("Buzzer Frequency: 5000 Hz");

  Serial.println("Door Demo: 15 seconds");
  Serial.println("Door Open Duration: 5 seconds");

  Serial.println("================================");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ===================================================
  // READ MPU
  // ===================================================

  bool mpuWorking =
    readMPU();


  // ===================================================
  // MOVEMENT DETECTION
  // ===================================================

  bool movementDetected =
    false;


  float movementDifference =
    0.0;


  if (mpuWorking)
  {
    movementDifference =
      getMovementDifference();


    if (movementDifference >
        MOVEMENT_THRESHOLD)
    {
      movementDetected = true;
    }
  }


  // ===================================================
  // TEMPERATURE STATUS
  // ===================================================

  bool temperatureUnsafe =
    false;


  if (manualTemperature < TEMP_MIN ||
      manualTemperature > TEMP_MAX)
  {
    temperatureUnsafe = true;
  }


  // ===================================================
  // HUMIDITY STATUS
  // =====================================================

  bool humidityUnsafe =
    false;


  if (manualHumidity < HUM_MIN ||
      manualHumidity > HUM_MAX)
  {
    humidityUnsafe = true;
  }


  // ===================================================
  // DOOR DEMONSTRATION
  // ===================================================

  unsigned long elapsedTime =
    millis() - demoStartTime;


  bool demoDoorOpen = false;


  if (elapsedTime >= DOOR_DEMO_DELAY &&
      elapsedTime <
      (DOOR_DEMO_DELAY + DOOR_OPEN_DURATION))
  {
    demoDoorOpen = true;
  }


  // ===================================================
  // RESET DEMO TIMER
  // ===================================================

  if (elapsedTime >=
      (DOOR_DEMO_DELAY + DOOR_OPEN_DURATION))
  {
    demoStartTime = millis();
  }


  // ===================================================
  // OVERALL SYSTEM STATUS
  // ===================================================

  bool systemUnsafe =
    movementDetected ||
    temperatureUnsafe ||
    humidityUnsafe ||
    demoDoorOpen;


  // ===================================================
  // SERIAL MONITOR
  // =====================================================

  Serial.println();
  Serial.println("--------------------------------");


  Serial.print("Temperature: ");
  Serial.print(
    manualTemperature,
    1
  );
  Serial.println(" C");


  Serial.print("Humidity: ");
  Serial.print(
    manualHumidity,
    1
  );
  Serial.println(" %");


  Serial.print("Acceleration X: ");

  if (mpuWorking)
  {
    Serial.print(accX, 2);
    Serial.println(" g");
  }
  else
  {
    Serial.println("ERROR");
  }


  Serial.print("Acceleration Y: ");

  if (mpuWorking)
  {
    Serial.print(accY, 2);
    Serial.println(" g");
  }
  else
  {
    Serial.println("ERROR");
  }


  Serial.print("Acceleration Z: ");

  if (mpuWorking)
  {
    Serial.print(accZ, 2);
    Serial.println(" g");
  }
  else
  {
    Serial.println("ERROR");
  }


  Serial.print("Movement Difference: ");
  Serial.println(
    movementDifference,
    2
  );


  Serial.print("Movement: ");

  if (movementDetected)
  {
    Serial.println("DETECTED");
  }
  else
  {
    Serial.println("NORMAL");
  }


  Serial.print("Door: ");

  if (demoDoorOpen)
  {
    Serial.println("OPEN");
  }
  else
  {
    Serial.println("CLOSED");
  }


  Serial.print("System State: ");

  if (systemUnsafe)
  {
    Serial.println("UNSAFE");
  }
  else
  {
    Serial.println("SAFE");
  }


  // ===================================================
  // OLED DISPLAY
  // ===================================================

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);


  // ---------------------------------------------------
  // TITLE
  // ---------------------------------------------------

  display.setCursor(0, 0);

  display.println("SENTINELBOX");


  display.drawLine(
    0,
    9,
    127,
    9,
    SSD1306_WHITE
  );


  // ---------------------------------------------------
  // TEMPERATURE
  // ---------------------------------------------------

  display.setCursor(0, 13);

  display.print("Temp: ");

  display.print(
    manualTemperature,
    1
  );

  display.println(" C");


  // ---------------------------------------------------
  // HUMIDITY
  // ---------------------------------------------------

  display.setCursor(0, 24);

  display.print("Hum : ");

  display.print(
    manualHumidity,
    1
  );

  display.println(" %");


  // ---------------------------------------------------
  // DOOR
  // ---------------------------------------------------

  display.setCursor(0, 35);

  if (demoDoorOpen)
  {
    display.println(
      "Door: OPEN"
    );
  }
  else
  {
    display.println(
      "Door: CLOSED"
    );
  }


  // ---------------------------------------------------
  // MOVEMENT
  // ---------------------------------------------------

  display.setCursor(0, 46);

  if (!mpuWorking)
  {
    display.println(
      "Move: MPU ERROR"
    );
  }
  else if (movementDetected)
  {
    display.println(
      "Move: DETECTED"
    );
  }
  else
  {
    display.println(
      "Move: NORMAL"
    );
  }


  // ---------------------------------------------------
  // SYSTEM STATUS
  // ---------------------------------------------------

  display.setCursor(0, 57);

  if (systemUnsafe)
  {
    display.println(
      "SYSTEM: UNSAFE"
    );
  }
  else
  {
    display.println(
      "SYSTEM: SAFE"
    );
  }


  display.display();


  // ===================================================
  // BUZZER
  // ===================================================

  if (movementDetected)
  {
    // Actual MPU6050 movement alert

    beep();
  }
  else if (demoDoorOpen)
  {
    // Simulated door-open demonstration alert

    beep();
  }
  else
  {
    noTone(
      BUZZER_PIN
    );

    delay(300);
  }
}