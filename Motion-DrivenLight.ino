/*
 * PROJECT: Motion-Driven Light (INTERNSHIP FINAL SUBMISSION)
 * AUTHOR: M.Salih BEKTUR
 *
 * SYSTEM ARCHITECTURE:
 * - Core 0: Sensor Fusion (Kalman Filtered MPU-6050)
 * - Core 1: Physics Engine (Verlet Integration) & Rendering
 *
 * COMPLIANCE CHECKLIST:
 * [x] Dual-Core Implementation (Asymmetric Multiprocessing)
 * [x] Robustness: Auto-switches to Simulation if sensor missing
 * [x] Visuals: Blue "Water Glow" with Purple/Cyan tilt shift
 * [x] Interaction: "Flow Delay" via Physics & Sparkle effects
 * [x] Serial: Single-line ASCII visualization (works w/o LEDs)
 *
 * ENGINEERING EXTRAS ("Tier 4" Features):
 * - Kalman Filter: Cleans noisy sensor data for smooth input.
 * - Verlet Integration: Conservation of energy for realistic "sloshing".
 * - Gamma Correction: CIE 1931 Lookup table for linear brightness.
 * - Sub-pixel Gaussian Rendering: Smooth movement between LEDs.
 */

#include <Wire.h>
#include "MPU6050.h"
#include <Adafruit_NeoPixel.h>
#include <math.h>

// --- CONFIGURATION ---
#define LED_PIN       15      // GPIO Pin for Data
#define NUM_LEDS      10      // LED Count
#define MPU_SDA       4       // I2C SDA
#define MPU_SCL       5       // I2C SCL
#define ASCII_WIDTH   25      // Width of Serial Monitor visualization

// --- PHYSICS CONSTANTS ---
#define GRAVITY       0.55    // Lower = Heavy liquid, Higher = Snappy
#define DRAG_COEFF    0.04    // Air resistance (Non-linear)
#define ELASTICITY    0.50    // Energy kept after hitting wall (0.0 - 1.0)
#define BLOB_WIDTH    1.8     // Width of the water glow (Gaussian Sigma)

// --- OBJECTS ---
MPU6050 mpu;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- SHARED DATA (Thread Safe) ---
volatile float sharedAngle = 0.0;
volatile bool sensorConnected = false;

// =========================================================
// CLASS: KALMAN FILTER
// Advanced sensor fusion to remove jitter from the MPU6050
// =========================================================
class KalmanFilter {
  private:
    float Q_angle = 0.001; 
    float Q_bias = 0.003;  
    float R_measure = 0.03;
    float angle = 0.0;     
    float bias = 0.0;      
    float P[2][2]; // Error covariance matrix 

  public:
    KalmanFilter() {
      P[0][0] = 0; P[0][1] = 0;
      P[1][0] = 0; P[1][1] = 0;
    }

    float getAngle(float newAngle, float newRate, float dt) {
      // 1. Predict
      float rate = newRate - bias;
      angle += dt * rate;

      P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
      P[0][1] -= dt * P[1][1];
      P[1][0] -= dt * P[1][1];
      P[1][1] += Q_bias * dt;

      // 2. Update
      float S = P[0][0] + R_measure;
      float K[2]; 
      K[0] = P[0][0] / S;
      K[1] = P[1][0] / S;

      float y = newAngle - angle; 
      angle += K[0] * y;
      bias += K[1] * y;

      float P00_temp = P[0][0];
      float P01_temp = P[0][1];

      P[0][0] -= K[0] * P00_temp;
      P[0][1] -= K[0] * P01_temp;
      P[1][0] -= K[1] * P00_temp;
      P[1][1] -= K[1] * P01_temp;

      return angle;
    }
};

KalmanFilter kFilter;

// =========================================================
// CORE 0: SENSOR ACQUISITION
// =========================================================
void setup() {
  Serial.begin(115200);
  
  // I2C Initialization
  Wire.setSDA(MPU_SDA);
  Wire.setSCL(MPU_SCL);
  Wire.begin();
  
  delay(2000); // Safety wait for Serial
  
  Serial.println("=== BOOTING SYSTEM ===");
  Serial.print("Core 0: Initializing Sensor...");
  
  mpu.initialize();
  sensorConnected = mpu.testConnection();
  
  if(sensorConnected) {
    Serial.println(" [SUCCESS]");
  } else {
    Serial.println(" [FAILED]");
    Serial.println("WARNING: Sensor missing. Switching to ROBUSTNESS MODE (Simulation).");
  }
}

unsigned long prevMicros = 0;

void loop() {
  if (sensorConnected) {
    // --- REAL SENSOR MODE ---
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Calculate Delta Time for Kalman Integration
    unsigned long currMicros = micros();
    float dt = (currMicros - prevMicros) / 1000000.0;
    prevMicros = currMicros;

    // Calculate Raw Pitch/Roll
    // NOTE: Depending on chip orientation, you might need atan2(ax, az) instead.
    // Standard module orientation usually uses ay, az for roll.
    float accAngle = atan2(ay, az) * 180.0 / PI;
    float gyroRate = gx / 131.0; 

    // Update Shared Variable via Kalman Filter
    sharedAngle = kFilter.getAngle(accAngle, gyroRate, dt);
    
  } else {
    // --- ROBUSTNESS MODE (Simulation) ---
    // Generates a smooth sine wave to prove the code works without hardware
    float t = millis() / 1000.0;
    sharedAngle = 45.0 * sin(t * 2.0); // Swing -45 to +45
    delay(10); // 100Hz simulation rate
  }
}

// =========================================================
// CORE 1: PHYSICS & RENDERING
// =========================================================

// Gamma Correction Table (Visual Polish)
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// Physics State Struct
struct FluidState {
  float pos;     // Current Position (0.0 to NUM_LEDS)
  float old_pos; // Previous Position (for Verlet velocity calc)
  float accel;   // Current Acceleration
};

// Start in the center
FluidState water = { (float)NUM_LEDS/2.0f, (float)NUM_LEDS/2.0f, 0.0f };

void setup1() {
  strip.begin();
  strip.setBrightness(255); // We handle brightness via software logic
  strip.show();
}

void loop1() {
  // --- STEP 1: VERLET PHYSICS ENGINE ---
  // Retrieve latest angle from Core 0
  float angle = sharedAngle;
  
  // 1. Calculate Forces
  // Gravity pulls the liquid based on sine of the angle
  float force = sin(angle * PI / 180.0) * GRAVITY; 
  
  // Drag (Air Resistance) - Quadratic Drag for realism
  float velocity = water.pos - water.old_pos;
  float drag = -velocity * abs(velocity) * DRAG_COEFF; 
  
  water.accel = force + drag;

  // 2. Integrate Position (Verlet Integration)
  // pos = 2*pos - old_pos + accel * dt^2
  float temp_pos = water.pos;
  water.pos = (2.0f * water.pos) - water.old_pos + (water.accel * 0.05f); 
  water.old_pos = temp_pos;

  // 3. Wall Collisions (Elasticity)
  if (water.pos < 0) {
    water.pos = 0;
    water.old_pos = -velocity * ELASTICITY; // Bounce back
  } 
  else if (water.pos > NUM_LEDS - 1) {
    water.pos = NUM_LEDS - 1;
    water.old_pos = (NUM_LEDS - 1) + (velocity * ELASTICITY); // Bounce back
  }

  // --- STEP 2: RENDERING ENGINE ---
  strip.clear();
  static int frameCount = 0;
  frameCount++;

  for (int i = 0; i < NUM_LEDS; i++) {
    // Calculate distance from the "center of mass" of the water
    float dist = abs(water.pos - i);
    
    // Gaussian Intensity (Bell Curve) for smooth blob
    float intensity = exp(-pow(dist, 2) / BLOB_WIDTH);
    
    if (intensity > 0.01) {
      // --- COLOR LOGIC (Per Guidelines) ---
      // "Blue water glow in middle... Left->Purple, Right->Cyan"
      
      // Determine tilt offset relative to center of strip
      float centerOffset = water.pos - ((NUM_LEDS - 1) / 2.0f);
      
      // Base Water Color = BLUE
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 255;

      if (centerOffset < -0.5) { 
        // Tilted Left (Negative) -> Add RED to make PURPLE
        r = map(constrain((int)abs(centerOffset*50), 0, 255), 0, 255, 0, 200);
      } 
      else if (centerOffset > 0.5) { 
        // Tilted Right (Positive) -> Add GREEN to make CYAN
        g = map(constrain((int)abs(centerOffset*50), 0, 255), 0, 255, 0, 200);
      }

      // Apply Intensity Mask
      int finalR = r * intensity;
      int finalG = g * intensity;
      int finalB = b * intensity;

      // --- FUN TOUCH: Temporal Dithering ---
      // Adds a subtle "shimmer" by flipping LSB on every frame
      if (frameCount % 2 == 0 && intensity > 0.2) {
         if(finalR < 255) finalR++; 
         if(finalG < 255) finalG++; 
         if(finalB < 255) finalB++; 
      }

      // Apply Gamma Correction for human eye perception
      strip.setPixelColor(i, 
        pgm_read_byte(&gamma8[min(255, finalR)]), 
        pgm_read_byte(&gamma8[min(255, finalG)]), 
        pgm_read_byte(&gamma8[min(255, finalB)])
      );
    }
  }
  strip.show();

  // --- STEP 3: SERIAL MONITOR (ASCII ART) ---
  // Throttle to ~20 FPS to keep Serial clear and Physics fast
  static unsigned long lastSerialTime = 0;
  if (millis() - lastSerialTime > 50) {
    lastSerialTime = millis();

    // Mapping: Physics Space (0..9) -> ASCII Space (0..24)
    int asciiPos = (int)((water.pos / (float)(NUM_LEDS - 1)) * (ASCII_WIDTH - 1));
    asciiPos = constrain(asciiPos, 0, ASCII_WIDTH - 1);

    Serial.print("STATUS: ");
    Serial.print(sensorConnected ? "LIVE " : "SIM  ");
    Serial.print("| TILT: ");
    
    // Pad numbers for alignment (prevent text jumping)
    if (angle >= 0) Serial.print(" ");
    if (abs(angle) < 10) Serial.print("0");
    Serial.print(angle, 1);
    
    Serial.print(" | WATER: [");
    for (int i = 0; i < ASCII_WIDTH; i++) {
      if (i == asciiPos) Serial.print("O");       // The bubble
      else if (i == ASCII_WIDTH/2) Serial.print("|"); // Center mark
      else Serial.print("~");                     // Water
    }
    Serial.println("]");
  }
  
  // Loop Delay (Physics Speed)
  delay(8); // ~125Hz Refresh Rate
}
