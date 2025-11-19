# Motion-Driven Light System
### Raspberry Pi Pico Dual-Core PCB Hat with MPU-6050 & WS2812B LED Strip

[![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi%20Pico-A22846?style=for-the-badge&logo=Raspberry%20Pi&logoColor=white)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![KiCad](https://img.shields.io/badge/KiCad-314CB0?style=for-the-badge&logo=KiCad&logoColor=white)](https://www.kicad.org/)
[![Arduino IDE](https://img.shields.io/badge/Arduino_IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)

---

## 📋 Project Overview

**Motion-Driven Light** is a dual-core embedded system for the Raspberry Pi Pico featuring:

- **Real-time motion sensing** via MPU-6050 accelerometer/gyroscope
- **Physics-based "water flow" LED animation** on WS2812B addressable RGB strip
- **Custom PCB Hat design** with full KiCad project files and Gerber manufacturing outputs
- **Hardware-agnostic robustness**: Automatic simulation mode when sensor is not detected
- **Dual-core architecture**: Core 0 handles sensor I/O, Core 1 manages LED rendering and serial visualization

**Technical Assignment for:** Yens & Yens  
**Author:** M. Salih BEKTUR

---

## 🚀 How to Test Without Hardware (Simulation Mode)

This project includes **automatic robustness** for testing without physical hardware:

### Quick Start (No Sensor Required)

1. **Flash the code** to any Raspberry Pi Pico (even without MPU-6050 connected)
2. **Open Serial Monitor** at 115200 baud
3. **System auto-detects** missing sensor and engages **SIMULATION MODE**
4. **Watch the animation**: A smooth sine wave generates virtual tilt data (-45° to +45°)

### What You'll See

The system will output real-time ASCII visualization showing the "liquid" moving:

```
STAT: SIMULATED | TILT:  35.2 deg | WATER: [~~~~~~~~~~~~~~~O~~~~~~~~~|]
STAT: SIMULATED | TILT:  38.4 deg | WATER: [~~~~~~~~~~~~~~~~O~~~~~~~~|]
STAT: SIMULATED | TILT:  39.1 deg | WATER: [~~~~~~~~~~~~~~~~~O~~~~~~~|]
STAT: SIMULATED | TILT:  37.5 deg | WATER: [~~~~~~~~~~~~~~~~O~~~~~~~~|]
STAT: SIMULATED | TILT:  33.8 deg | WATER: [~~~~~~~~~~~~~~O~~~~~~~~~~|]
STAT: SIMULATED | TILT:  28.1 deg | WATER: [~~~~~~~~~~~~~O~~~~~~~~~~~|]
STAT: SIMULATED | TILT:  20.9 deg | WATER: [~~~~~~~~~~~O~~~~~~~~~~~~~|]
STAT: SIMULATED | TILT:  12.3 deg | WATER: [~~~~~~~~~~O~~~~~~~~~~~~~~|]
STAT: SIMULATED | TILT:  03.1 deg | WATER: [~~~~~~~~O~~~~~~~~~~~~~~~~|]
STAT: SIMULATED | TILT: -05.8 deg | WATER: [~~~~~~~O~~~~~~~~~~~~~~~~~|]
STAT: SIMULATED | TILT: -14.2 deg | WATER: [~~~~~~O~~~~~~~~~~~~~~~~~~|]
STAT: SIMULATED | TILT: -21.6 deg | WATER: [~~~~O~~~~~~~~~~~~~~~~~~~~|]
```

**Legend:**
- `O` = Current "bubble" position (liquid center)
- `~` = Water surface
- `|` = Center reference marker (0° tilt)
- Status shows `SIMULATED` or `LIVE` depending on sensor detection

### With Real Hardware

When MPU-6050 is connected:
- System automatically detects sensor at boot
- Display changes to `STAT: LIVE`
- Tilt the board left/right to control the LED "liquid"
- LED color shifts: **Blue** (flat) → **Purple** (left) → **Cyan** (right)

---

## 🏗️ Architecture: Dual-Core Design

The Raspberry Pi Pico's RP2040 chip features **two ARM Cortex-M0+ cores**. This project leverages both for parallel task execution:

### Core 0: Sensor Data Acquisition
**File:** `setup()` and `loop()` functions
- Initializes I2C communication (GPIO 4=SDA, GPIO 5=SCL)
- Continuously polls MPU-6050 for 6-axis motion data (50 Hz)
- Calculates roll angle (tilt) from accelerometer readings
- **Fallback Logic**: If sensor fails, generates smooth sine wave simulation
- Updates shared variable `currentRoll` (thread-safe volatile)

### Core 1: Visual Output & Rendering
**File:** `setup1()` and `loop1()` functions
- Initializes WS2812B LED strip (GPIO 15, 10 LEDs)
- Reads `currentRoll` and maps to LED position (0-9)
- **LED Physics Animation**:
  - "Water blob" centered at tilt position
  - Smooth gradient falloff (±2.5 LED spread)
  - Dynamic color shifting based on tilt direction
- **ASCII Serial Output**: Real-time visualization at 15 FPS
- Rendering rate: ~60ms per frame

### Thread Safety
- `volatile float currentRoll` ensures atomic reads/writes between cores
- No mutex required due to single-variable communication pattern

---

## 🛠️ Hardware Configuration

### Pin Mapping (Custom PCB Hat)

| Component       | GPIO | Function          | PCB Connector |
|-----------------|------|-------------------|---------------|
| MPU-6050 SDA    | GP4  | I2C Data          | J1 Pin 1      |
| MPU-6050 SCL    | GP5  | I2C Clock         | J1 Pin 2      |
| WS2812B Data In | GP15 | LED Strip Control | J2 Pin 3      |
| Power (5V)      | VBUS | LED Strip Power   | J2 Pin 1      |
| Ground          | GND  | Common Ground     | J2 Pin 2      |

### Bill of Materials (PCB)

- **Microcontroller**: Raspberry Pi Pico (RP2040)
- **Sensor**: MPU-6050 6-DOF IMU Module
- **LEDs**: WS2812B Addressable RGB Strip (10 LEDs minimum)
- **PCB**: Custom 2-layer board (see `Final PCB Files/`)
- **Connectors**: JST headers for modular connections

---

## 📦 Installation & Setup

### 1. Software Requirements

- **Arduino IDE** (v1.8.19+ or v2.x)
- **Board Support**: Raspberry Pi Pico by Earle F. Philhower, III
  - Add to Board Manager URL: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **Libraries** (Install via Library Manager):
  - `Adafruit NeoPixel` by Adafruit
  - `MPU6050` by Electronic Cats

### 2. Upload Instructions

```bash
# 1. Clone this repository
git clone https://github.com/varolkan/Yens-Yenspreview-repo.git
cd Yens-Yenspreview-repo

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
2. Connect MPU-6050 module to J1 header
3. Connect WS2812B strip to J2 header (observe polarity!)
4. Power via USB-C on Pico

**No Sensor?** System will auto-detect and run in simulation mode.

---

## 📐 PCB Design Files

### Included Resources

| Folder                | Contents                              | Purpose                    |
|-----------------------|---------------------------------------|----------------------------|
| `Final PCB Files/`    | KiCad project files (.kicad_pcb, .kicad_sch, .kicad_pro) | Source design files        |
| `Final GERBER FILES/` | Manufacturing outputs (.gbr, .drl, .gbrjob) | Send to PCB fabricator     |

### Manufacturing Specs

- **Layers**: 2 (Top + Bottom copper)
- **Dimensions**: Matches Raspberry Pi Pico footprint
- **Thickness**: 1.6mm standard FR-4
- **Minimum Trace/Space**: 0.15mm / 0.15mm
- **Finish**: HASL or ENIG recommended

**Ready to Order:** Upload the entire `Final GERBER FILES/` folder to JLCPCB, PCBWay, or similar service.

---

## 🧪 Testing & Validation

### Functional Tests

1. **Sensor Detection Test**
   - Boot with/without MPU-6050 connected
   - Verify correct mode displayed in serial output

2. **Simulation Mode Test**
   - Boot without sensor
   - Confirm smooth sine wave animation (no crashes)

3. **Live Mode Test**
   - Connect MPU-6050 to I2C pins
   - Tilt board and verify LED responds to physical motion
   - Check color transitions (Blue → Purple/Cyan)

4. **Serial Visualization Test**
   - Monitor ASCII output for proper formatting
   - Verify 'O' marker moves across 25-character width

### Expected Serial Output Format

```
=== SYSTEM BOOT ===
Core 0: Searching for MPU-6050... [MISSING]
WARNING: Sensor not found. Engaging SIMULATION MODE.
STAT: SIMULATED | TILT:  00.0 deg | WATER: [~~~~~~~~~~~~O~~~~~~~~~~~~|]
```

---

## 🎯 Key Features

### ✨ Technical Highlights

- **Zero-Configuration Robustness**: Works immediately on any Pico board
- **Professional PCB Design**: Production-ready Gerber files included
- **Efficient Dual-Core Usage**: Sensor I/O never blocks LED rendering
- **Smooth Physics Animation**: Sub-pixel interpolation for fluid motion
- **Low Latency**: 20ms sensor polling, 60ms render cycle
- **Universal Compatibility**: Standard Arduino libraries, no custom dependencies

### 🔧 Customization Options

Modify these constants in `Motion-DrivenLight.ino`:

```cpp
#define NUM_LEDS      10      // LED strip length (1-100)
#define LED_BRIGHT    150     // Brightness (0-255)
#define ASCII_WIDTH   25      // Serial monitor width
```

---

## 📚 Documentation

- **Code Comments**: Inline documentation throughout source
- **Pin Diagram**: See Hardware Configuration section above
- **Schematic**: Available in `Final PCB Files/MPU6050.kicad_schFINAL.kicad_sch`

---

## 📄 License

This project is open-source under the [MIT License](LICENSE).

---

## 🙏 Acknowledgments

- **Libraries Used**:
  - Adafruit NeoPixel Library
  - MPU6050 Library by Electronic Cats (based on Jeff Rowberg's I2Cdevlib)
- **Board Support**: Raspberry Pi Pico Arduino Core by Earle F. Philhower, III
- **Assignment**: Yens & Yens Technical Test

---

## 📞 Contact

**Author:** M. Salih BEKTUR  
**Repository:** [github.com/varolkan/Yens-Yenspreview-repo](https://github.com/varolkan/Yens-Yenspreview-repo)

For questions regarding this technical assignment submission, please open an issue in the repository.

---

**Built with ❤️ for embedded systems and creative engineering**
