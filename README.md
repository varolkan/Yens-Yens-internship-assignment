# Motion-Driven Light System

Raspberry Pi Pico Dual-Core PCB Hat with MPU-6050 and WS2812B LED Strip

[![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi%20Pico-A22846?style=for-the-badge&logo=Raspberry%20Pi&logoColor=white)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![KiCad](https://img.shields.io/badge/KiCad-314CB0?style=for-the-badge&logo=KiCad&logoColor=white)](https://www.kicad.org/)
[![Arduino IDE](https://img.shields.io/badge/Arduino_IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)

## Project Overview

Motion-Driven Light is a dual-core embedded system for the Raspberry Pi Pico featuring:

- Real-time motion sensing via MPU-6050 6-axis accelerometer/gyroscope module
- Physics-based water flow LED animation with smooth gradient falloff on WS2812B addressable RGB LED strip
- Custom PCB Hat design with complete KiCad project files and production-ready Gerber manufacturing outputs
- Hardware-agnostic robustness: Automatic simulation mode when sensor is not detected, enabling code testing without physical hardware
- Dual-core architecture: Core 0 handles sensor I/O and data acquisition, Core 1 manages LED rendering and real-time serial visualization

Technical Assignment for: Yens & Yens  
Author: M. Salih BEKTUR

## Testing Without Hardware (Simulation Mode)

This project includes automatic robustness for testing without physical hardware.

### Quick Start (No Sensor Required)

1. Flash the code to any Raspberry Pi Pico (even without MPU-6050 connected)
2. Open Serial Monitor at 115200 baud
3. System auto-detects missing sensor and engages SIMULATION MODE
4. Watch the animation: A smooth sine wave generates virtual tilt data (-45° to +45°)

### Expected Output

The system will output real-time ASCII visualization showing the liquid moving:

```
STATUS: SIM  | TILT: -44.8 | WATER: [O~~~~~~~~~~~~~~~~~~~~~]
STATUS: SIM  | TILT: -36.9 | WATER: [~O~~~~~~~~~~~~~~~~~~~~]
STATUS: SIM  | TILT: -27.5 | WATER: [~~~O~~~~~~~~~~~~~~~~~~]
STATUS: SIM  | TILT: -17.2 | WATER: [~~~~~O~~~~~~~~~~~~~~~~]
STATUS: SIM  | TILT:  -7.1 | WATER: [~~~~~~~O~~~~~~~~~~~~~~]
STATUS: SIM  | TILT:   2.3 | WATER: [~~~~~~~~~O~~~~~~~|~~~~]
STATUS: SIM  | TILT:  11.4 | WATER: [~~~~~~~~~~~~O~~~~|~~~~]
STATUS: SIM  | TILT:  20.1 | WATER: [~~~~~~~~~~~~~~O~~|~~~~]
STATUS: SIM  | TILT:  29.0 | WATER: [~~~~~~~~~~~~~~~~O|~~~~]
STATUS: SIM  | TILT:  37.6 | WATER: [~~~~~~~~~~~~~~~~~|O~~~]
STATUS: SIM  | TILT:  44.6 | WATER: [~~~~~~~~~~~~~~~~~|~~~O]
STATUS: SIM  | TILT:  37.2 | WATER: [~~~~~~~~~~~~~~~~~|O~~~]
STATUS: SIM  | TILT:  28.4 | WATER: [~~~~~~~~~~~~~~~~O|~~~~]
STATUS: SIM  | TILT:  18.7 | WATER: [~~~~~~~~~~~~~~~O~|~~~~]
STATUS: SIM  | TILT:   8.9 | WATER: [~~~~~~~~~~O~~~~~~|~~~~]
STATUS: SIM  | TILT:  -0.9 | WATER: [~~~~~~~~O~~~~~~~~|~~~~]
STATUS: SIM  | TILT: -10.5 | WATER: [~~~~~~O~~~~~~~~~~|~~~~]
STATUS: SIM  | TILT: -20.0 | WATER: [~~~~O~~~~~~~~~~~~|~~~~]
STATUS: SIM  | TILT: -29.1 | WATER: [~~~O~~~~~~~~~~~~~|~~~~]
STATUS: SIM  | TILT: -38.0 | WATER: [~O~~~~~~~~~~~~~~~~~~~~]
```

Legend:
- O = Current bubble position (liquid center)
- ~ = Water surface
- | = Center reference marker (0° tilt)
- Status shows SIMULATED or LIVE depending on sensor detection

### With Real Hardware

When MPU-6050 is connected:
- System automatically detects sensor at boot
- Display changes to STAT: LIVE
- Tilt the board left/right to control the LED liquid
- LED color shifts: Blue (flat) to Purple (left) to Cyan (right)

## Architecture: Dual-Core Design

The Raspberry Pi Pico's RP2040 chip features two ARM Cortex-M0+ cores. This project leverages both for parallel task execution:

### Core 0: Sensor Data Acquisition
File: setup() and loop() functions
- Initializes I2C communication (GPIO4=SDA, GPIO5=SCL)
- Continuously polls MPU-6050 for 6-axis motion data (50 Hz)
- Calculates roll angle (tilt) from accelerometer readings and stores it in sharedAngle
- Fallback Logic: If sensor fails, generates smooth sine wave simulation
- Updates shared variable sharedAngle (thread-safe volatile)

### Core 1: Visual Output and Rendering
File: setup1() and loop1() functions
- Initializes WS2812B LED strip (GPIO15, 10 LEDs)
- Reads sharedAngle and maps to LED position (0-9)
- LED Physics Animation:
  - Water blob centered at tilt position
  - Smooth gradient falloff using Gaussian distribution
  - Dynamic color shifting based on tilt direction
- ASCII Serial Output: Real-time visualization at 20 FPS
- Rendering rate: ~8ms per frame (125Hz refresh)

### Thread Safety
- volatile float sharedAngle ensures atomic reads/writes between cores
- volatile bool mpuFound indicates sensor status
- No mutex required due to single-variable communication pattern

## Hardware Configuration

### Pin Mapping (Custom PCB Hat)

| Component       | GPIO  | Function          | PCB Connector |
|-----------------|-------|-------------------|---------------|
| MPU-6050 SDA    | GPIO4 | I2C Data          | J2 Pin 1      |
| MPU-6050 SCL    | GPIO5 | I2C Clock         | J2 Pin 2      |
| WS2812B Data In | GPIO15| LED Strip Control | J2 Pin 3      |
| Power (5V)      | VBUS  | LED Strip Power   | J2 Pin 4      |
| Ground          | GND   | Common Ground     | J2 Pin 5      |

### Bill of Materials (PCB)

- Microcontroller: Raspberry Pi Pico (RP2040)
- Sensor: MPU-6050 6-DOF IMU Module
- LEDs: WS2812B Addressable RGB Strip (10 LEDs minimum)
- PCB: Custom 2-layer board (see Final PCB Files/)
- Connectors: JST headers for modular connections

## Installation and Setup

### 1. Software Requirements

- Arduino IDE (v1.8.19+ or v2.x)
- Board Support: Raspberry Pi Pico by Earle F. Philhower, III
  - Add to Board Manager URL: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
- Libraries (Install via Library Manager):
  - Adafruit NeoPixel by Adafruit
  - MPU6050 by Electronic Cats

### 2. Upload Instructions

```bash
# 1. Clone this repository
git clone https://github.com/varolkan/Testing-123.git
cd Testing-123/Yens-Yens-internship-assignment-main

# 2. Open Motion-DrivenLight.ino in Arduino IDE

# 3. Configure Board Settings:
#    - Board: "Raspberry Pi Pico"
#    - Port: (Select your Pico's COM port)
#    - Upload Speed: 921600

# 4. Click Upload (Ctrl+U)

# 5. Open Serial Monitor (Ctrl+Shift+M)
#    - Set baud rate: 115200
```

### 3. Hardware Assembly (Optional)

1. Solder Raspberry Pi Pico to custom PCB
2. Connect MPU-6050 module and WS2812B strip to J2 header (observe polarity)
3. Power via USB-C on Pico

Note: System will auto-detect and run in simulation mode without sensor.

## PCB Design Files

### Included Resources

| Folder                | Contents                              | Purpose                    |
|-----------------------|---------------------------------------|----------------------------|
| Final PCB Files/      | KiCad project files (.kicad_pcb, .kicad_sch, .kicad_pro) | Source design files        |
| Final GERBER FILES/   | Manufacturing outputs (.gbr, .drl, .gbrjob) | Send to PCB fabricator     |

### Manufacturing Specifications

- Layers: 2 (Top + Bottom copper)
- Dimensions: Matches Raspberry Pi Pico footprint
- Thickness: 1.6mm standard FR-4
- Minimum Trace/Space: 0.15mm / 0.15mm
- Finish: HASL or ENIG recommended

Ready to Order: Upload the entire Final GERBER FILES/ folder to JLCPCB, PCBWay, or similar service.

## Testing and Validation

### Functional Tests

1. Sensor Detection Test
   - Boot with/without MPU-6050 connected
   - Verify correct mode displayed in serial output

2. Simulation Mode Test
   - Boot without sensor
   - Confirm smooth sine wave animation (no crashes)

3. Live Mode Test
   - Connect MPU-6050 to I2C pins
   - Tilt board and verify LED responds to physical motion
   - Check color transitions (Blue to Purple/Cyan)

4. Serial Visualization Test
   - Monitor ASCII output for proper formatting
   - Verify 'O' marker moves across 25-character width

### Expected Serial Output Format

```
=== SYSTEM BOOT ===
Core 0: Searching for MPU-6050... [MISSING]
WARNING: Sensor not found. Engaging SIMULATION MODE.
STAT: SIMULATED | TILT:  00.0 deg | WATER: [~~~~~~~~~~~~O~~~~~~~~~~~~|]
```

### Test Results

The code has been validated 3 times using comprehensive test suite covering:
- Simulation mode functionality (no hardware required)
- Live sensor mode with Kalman filtering
- Physics engine stability at all angles
- Boundary conditions and edge cases
- ASCII visualization rendering

All tests passed successfully. See TEST_RESULTS.md for detailed test results and COMPATIBILITY_TEST.md for platform compatibility analysis.

## Key Features

### Technical Highlights

- Zero-Configuration Robustness: Works immediately on any Pico board
- Professional PCB Design: Production-ready Gerber files included
- Efficient Dual-Core Usage: Sensor I/O never blocks LED rendering
- Smooth Physics Animation: Sub-pixel interpolation for fluid motion
- Low Latency: 20ms sensor polling, 60ms render cycle
- Universal Compatibility: Standard Arduino libraries, no custom dependencies

### Customization Options

Modify these constants in Motion-DrivenLight.ino:

```cpp
#define NUM_LEDS      10      // LED strip length (1-100)
#define LED_BRIGHT    150     // Brightness (0-255)
#define ASCII_WIDTH   25      // Serial monitor width
```

## Code Structure

The implementation consists of the following key components:

### Main Arduino File: Motion-DrivenLight.ino

- KalmanFilter class: Advanced sensor fusion to remove jitter from MPU-6050
- Core 0 functions (setup/loop): Sensor acquisition and simulation
- Core 1 functions (setup1/loop1): Physics engine and rendering
- Verlet Integration: Physics-based motion simulation
- Gamma Correction: CIE 1931 lookup table for linear brightness perception

### Physics Constants

- GRAVITY: 0.55 (controls liquid responsiveness)
- DRAG_COEFF: 0.04 (air resistance simulation)
- ELASTICITY: 0.50 (bounce behavior at boundaries)
- BLOB_WIDTH: 1.8 (Gaussian sigma for smooth LED glow)

## Documentation

- Code Comments: Inline documentation throughout source
- Pin Diagram: See Hardware Configuration section above
- Schematic: Available in Final PCB Files/MPU6050.kicad_sch
- Compatibility Analysis: See COMPATIBILITY_TEST.md

## License

This project is open-source under the MIT License. See LICENSE file for details.

## Acknowledgments

Libraries Used:
- Adafruit NeoPixel Library
- MPU6050 Library by Electronic Cats (based on Jeff Rowberg's I2Cdevlib)

Board Support: Raspberry Pi Pico Arduino Core by Earle F. Philhower, III

Assignment: Yens & Yens Technical Test

## Contact

Author: M. Salih BEKTUR  
Repository: https://github.com/varolkan/Yens-Yens-internship-assignment

For questions regarding this technical assignment submission, please open an issue in the repository.
