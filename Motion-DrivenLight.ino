/*
 * PROJECT: Motion-Driven Light (Pico PCB Hat)
 * AUTHOR: M.Salih BEKTUR
 *
 * HARDWARE CONFIGURATION:
 * - Core 0: MPU-6050 Sensor (I2C0: SDA=GP4, SCL=GP5)
 * - Core 1: WS2812B LED Strip (Data=GP15, Power=VBUS, GND=GND)
 *
 * REQUIRED LIBRARIES (Install via Library Manager):
 * 1. "Adafruit NeoPixel" by Adafruit
 * 2. "MPU6050" by Electronic Cats (Jeff Rowberg based)
 *
 * BOARD SETTING:
 * Select "Raspberry Pi Pico" (Earle Philhower core recommended for dual-core support)
 */

#include <Wire.h>
#include "MPU6050.h"
#include <Adafruit_NeoPixel.h>

// --- CONFIGURATION ---
#define LED_PIN       15      // GPIO pin connected to J2 Pin 3
#define NUM_LEDS      10      // Number of LEDs in your strip
#define LED_BRIGHT    150     // Max brightness (0-255)
#define MPU_SDA       4       // I2C0 SDA Pin
#define MPU_SCL       5       // I2C0 SCL Pin
#define ASCII_WIDTH   25      // Width of Serial Monitor visualization

// --- SHARED DATA (Thread-Safe) ---
volatile float currentRoll = 0.0; 
volatile bool mpuFound = false;

// --- OBJECTS ---
MPU6050 mpu;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// =========================================================
// CORE 0: SENSOR LOGIC & ROBUSTNESS
// =========================================================

void setup() {
  // 1. Init Serial
  Serial.begin(115200);
  delay(2000); // Safety delay for USB

  // 2. Init I2C
  Wire.setSDA(MPU_SDA);
  Wire.setSCL(MPU_SCL);
  Wire.begin();

  Serial.println("=== SYSTEM BOOT ===");
  Serial.print("Core 0: Searching for MPU-6050...");

  // 3. Init & Check (Correct Order)
  mpu.initialize(); 
  
  if (mpu.testConnection()) {
    mpuFound = true;
    Serial.println(" [ONLINE]");
  } else {
    mpuFound = false;
    Serial.println(" [MISSING]");
    Serial.println("WARNING: Sensor not found. Engaging SIMULATION MODE.");
  }
}

void loop() {
  if (mpuFound) {
    // --- A. REAL SENSOR MODE ---
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Calculate Roll Angle (Tilt Left/Right) in degrees
    // atan2 returns radians, convert to degrees
    float angle = atan2(ay, az) * 180.0 / PI;
    
    currentRoll = angle; // Update shared variable

  } else {
    // --- B. SIMULATION MODE (Robustness) ---
    // Generate a smooth sine wave (-45 to +45 degrees)
    float timeVal = millis() / 1000.0;
    currentRoll = 45.0 * sin(timeVal * 2.0); 
  }

  delay(20); // Run fast (50Hz)
}

// =========================================================
// CORE 1: VISUALS (LEDs & ASCII)
// =========================================================

void setup1() {
  strip.begin();
  strip.setBrightness(LED_BRIGHT);
  strip.show(); 
}

void loop1() {
  // 1. Thread Safety: Copy volatile variable to local scope immediately
  float roll = currentRoll;

  // --- LED LOGIC: "Water Flow" ---
  
  // Manual float mapping for precision
  // Constrain roll to -45..45, map to 0..(NUM_LEDS-1)
  float r_clamped = constrain(roll, -45.0f, 45.0f);
  float centerPos = (r_clamped + 45.0f) * (float)(NUM_LEDS - 1) / 90.0f;
  
  strip.clear(); 

  for (int i = 0; i < NUM_LEDS; i++) {
    // Calculate distance from the "center of liquid"
    float dist = abs(centerPos - i);
    
    // Draw the blob (width approx 2.5 LEDs)
    if (dist < 2.5) { 
      // Color Shift: Blue (Flat) -> Cyan (Right) -> Purple (Left)
      uint8_t red = 0;
      uint8_t green = 0;
      
      if (roll < -10) { // Left Tilt -> Purple
         red = map((long)abs(roll), 10, 45, 0, 200);
      } else if (roll > 10) { // Right Tilt -> Cyan
         green = map((long)abs(roll), 10, 45, 0, 200);
      }
      
      strip.setPixelColor(i, strip.Color(red, green, 255)); // Base Blue
    }
  }
  strip.show(); 

  // --- SERIAL MONITOR: ASCII Art ---
  // Map roll to character position
  int asciiPos = (int)((r_clamped + 45.0f) * (float)(ASCII_WIDTH - 1) / 90.0f + 0.5f);
  asciiPos = constrain(asciiPos, 0, ASCII_WIDTH - 1);

  Serial.print("STAT: ");
  Serial.print(mpuFound ? "LIVE    " : "SIMULATED");
  Serial.print(" | TILT: ");
  
  // Clean formatting for alignment
  if (roll >= 0) Serial.print(" "); 
  if (abs(roll) < 10) Serial.print("0"); 
  Serial.print(roll, 1);
  Serial.print(" deg | WATER: [");

  for (int i = 0; i < ASCII_WIDTH; i++) {
    if (i == asciiPos) {
      Serial.print("O"); // The bubble
    } else if (i == ASCII_WIDTH / 2) {
      Serial.print("|"); // Center marker
    } else {
      Serial.print("~"); // Water
    }
  }
  Serial.println("]");

  delay(60); // ~15 FPS
}
