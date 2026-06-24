# ESP32 Motor Controller

This code turns an ESP32 into a **closed-loop motor controller**. It provides a simple serial interface to communicate with a high-level computer (e.g. running ROS2) and generates the appropriate PWM signals for a motor driver to drive two motors in closed loop. With Latest LEDC Update for ESP32 Boards.

This is a fork of [manojramesh-io/ROS2-ESP32-Serial-Bridge](https://github.com/manojramesh-io/ROS2-ESP32-Serial-Bridge), modified to work with **4-pin motor drivers like the L298N** (IN1, IN2, IN3, IN4, ENA, ENB) instead of the original single direction-pin + PWM layout. Individual PID gains per motor have also been added.

For an alternative with full ROS nodes, see [joshnewans/ros_arduino_bridge](https://github.com/joshnewans/ros_arduino_bridge).

> As I only require a subset of the functionality, I have no idea what does and doesn't work beyond what is detailed below. Feedback and improvements are welcome.

---

## What's Changed from the Original

- **L298N (4-pin) motor driver support** — rewired for IN1/IN2/ENA (right) and IN3/IN4/ENB (left) instead of a single direction + PWM pin per motor
- **Individual PID gains per motor** — left and right motors can be tuned independently
- Removed unused ROS node code
- Fixed encoder floating-pin issue (pull-ups + external resistor for GPIO 34/35)
- Fixed PID timer initialization and auto-stop reset

---

## Functionality

- Receives motor speed requests over serial and provides encoder feedback
- Closed-loop PID speed control using quadrature encoders
- Raw PWM mode for open-loop testing
- GPIO utilities (analog/digital read/write, pin mode) from the original codebase — untested by me

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 |
| Motor Driver | L298N (or any IN1/IN2/ENA style driver) |
| Encoders | Quadrature A/B channel, one per motor |

### Pin Configuration

Defined in `config.h` — change to match your wiring:

| Signal | GPIO | Notes |
|---|---|---|
| Encoder 0 — Channel A (Left) | 32 | `INPUT_PULLUP` |
| Encoder 0 — Channel B (Left) | 35 | External 10kΩ to 3.3V required — no internal pull-up (NOT RECOMENDED)|
| Encoder 1 — Channel A (Right) | 34 | External 10kΩ to 3.3V required — no internal pull-up (NOT RECOMENDED)|
| Encoder 1 — Channel B (Right) | 33 | `INPUT_PULLUP` |
| Left Motor ENB (PWM) | 27 | |
| Left Motor IN3 | 26 | |
| Left Motor IN4 | 27 | |
| Right Motor ENA (PWM) | 14 | |
| Right Motor IN1 | 14 | |
| Right Motor IN2 | 25 | |

> **GPIO 34 and 35** are input-only pins on the ESP32 with **no internal pull-up resistors**. You must add a physical **10kΩ resistor from each pin to 3.3V** on your board, otherwise floating inputs will trigger false encoder interrupts and cause motors to spin uncontrollably. Only Used for a special purpose here.

---

## Software Dependencies

- [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32)
- Arduino IDE or PlatformIO

Add ESP32 board support in Arduino IDE via Board Manager URL:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## Configuration

All tunable parameters are in `config.h`:

```cpp
#define BAUDRATE           115200  // Serial baud rate
#define PID_RATE           30      // PID loop frequency (Hz)
#define MAX_PWM            255     // Maximum PWM output
#define AUTO_STOP_INTERVAL 2000    // Stop motors if no command received (ms)
                                   // Set to 9999999 to disable auto-stop
```

### PID Gains

Select a mode by commenting/uncommenting in `config.h`:

**Individual gains per motor (recommended):**
```cpp
#define INDIVIDUAL_PID_GAINS

#define KP_RIGHT 20
#define KD_RIGHT 12
#define KI_RIGHT 0
#define KO_RIGHT 30

#define KP_LEFT  18
#define KD_LEFT  12
#define KI_LEFT  0
#define KO_LEFT  30
```

**Identical gains for both motors:**
```cpp
#define IDENTICAL_PID_GAINS

#define KP 20
#define KD 12
#define KI 0
#define KO 50
```

---

## Serial Command Protocol

Send ASCII commands terminated with a **carriage return (`\r`)** at **115200 baud**.

| Command | Format | Description |
|---|---|---|
| `e` | `e` | Read current encoder counts |
| `r` | `r` | Reset encoder counts and PID state |
| `m` | `m <left> <right>` | Set closed-loop speed (ticks/frame) |
| `o` | `o <left> <right>` | Set raw PWM speed (-255 to 255), bypasses PID |
| `u` | `u <Kp>:<Kd>:<Ki>:<Ko>` | Update PID gains (identical mode only) |
| `b` | `b` | Get baud rate |
| `a` | `a <pin>` | Analog read |
| `d` | `d <pin>` | Digital read |
| `w` | `w <pin> <0\|1>` | Digital write |
| `x` | `x <pin> <val>` | Analog (DAC) write |
| `c` | `c <pin> <0\|1>` | Set pin mode (0 = INPUT, 1 = OUTPUT) |

### Examples

```
e             → Returns: "leftCount rightCount"
r             → Returns: "OK"  (resets encoders)
m 50 50       → Both motors forward at 50 ticks/frame
m -50 50      → Spin left in place
m 0 0         → Stop (PID controlled)
o 128 128     → Raw PWM, ~half speed forward
o 0 0         → Stop (raw)
u 20:12:0:50  → Update PID gains (identical mode only)
```

### Gotchas

- **Motor speed unit** is ticks per PID loop frame. At the default 30 Hz rate, `(ticks per second) / 30` gives you the value to send. For example, if your encoder does 300 ticks/sec at full speed, send `m 10 10`.
- **Auto-stop** — by default, motors stop if no command is received for 2 seconds. Keep sending `m` commands in your control loop, or increase `AUTO_STOP_INTERVAL` in `config.h`.
- **Carriage return** — commands must end with `\r`, not `\n`. In Python: `ser.write(b'm 50 50\r')`.
- **PID parameter order** is `Kp Kd Ki Ko` for the `u` command.
- **Serial port permissions** on Linux — add your user to the `dialout` group: `sudo usermod -aG dialout $USER`

---

## File Structure

```
ROS2_BRIDGE/
├── ROS2_ESP32_BRIDGE.ino   # Main sketch — serial parser and control loop
├── config.h                 # Pin assignments and tunable parameters
├── commands.h               # Command character definitions
├── motor_driver.h/.cpp      # L298N driver (IN1/IN2/ENA, IN3/IN4/ENB)
├── encoder.h/.cpp           # Quadrature encoder ISRs and position counters
└── pid.h                    # PID controller logic and update loop
```

---

## Uploading

1. Open `ROS2_ESP32_BRIDGE.ino` in Arduino IDE.
2. Go to **Tools → Board** and select your ESP32 board (e.g. ESP32 Dev Module).
3. Go to **Tools → Port** and select the correct COM/tty port.
4. Click **Upload**.

---

## ROS2 Integration

Configure your ROS2 serial base driver with:

- **Port:** `/dev/ttyUSB0` (or whichever port the ESP32 appears on)
- **Baud rate:** `115200`

Use the `m` command for closed-loop speed control. Convert your desired speed from ticks/second to ticks/frame using: `ticks_per_frame = ticks_per_second / PID_RATE`.

---

## TODO

- [ ] Document PID tuning process
- [ ] Test additional L298N wiring configurations
- [ ] Test untested original features (servo, analog/digital GPIO)
