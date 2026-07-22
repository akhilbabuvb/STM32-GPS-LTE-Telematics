# STM32-GPS-LTE-Telematics
STM32-Based 4G Vehicle Telematics Device | Product Prototype

Designed and developed a real-time telematics device using STM32F103C8T6, u-blox NEO-M8N GNSS receiver, and SIM A7670C 4G LTE/GSM cellular module for vehicle tracking applications.

Implemented dual-UART communication, GNSS NMEA data parsing (latitude, longitude, speed, UTC, heading, and GPS-based distance travelled), and cellular modem communication using AT commands.

Developed SMS-based remote command functionality (LOC, SPEED, STATUS, and DIST) to support remote vehicle tracking and device diagnostics.

Implemented HTTP-based GPS telemetry transmission over LTE/GPRS, enabling real-time communication with a cloud platform for remote location monitoring.

Developed embedded firmware for hardware integration, UART communication, GNSS data processing, and cloud connectivity using STM32CubeIDE, ST-Link V2, and UART debugging tools.

###########################################################   PROJECT ARCHITECTURE    ###########################################################################

                    +---------------------+
                    |   NEO-M8M GPS       |
                    | 9600 bps UART       |
                    +----------+----------+
                               |
                               | UART1 RX
                               |
                               ▼
+------------------------------------------------------+
|                STM32F103C8T6 (Blue Pill)             |
|------------------------------------------------------|
|                                                      |
| UART1 (GPS)                                          |
|  • Interrupt Reception                               |
|  • Ring Buffer                                       |
|  • NMEA Sentence Receiver                            |
|  • GPS Parser ($GNRMC)                               |
|                                                      |
| Application Layer                                    |
|  • Latitude                                          |
|  • Longitude                                         |
|  • Speed                                             |
|  • UTC Time                                          |
|  • Device Status                                     |
|                                                      |
| UART2 (LTE Modem)                                    |
|  • AT Command Driver                                 |
|  • HTTP/MQTT                                         |
|  • Send Telemetry                                    |
|                                                      |
+----------------------+-------------------------------+
                       |
                       | UART
                       ▼
             +-------------------------+
             | A7670C 2G/4G Board      |
             | AT Commands             |
             | TCP/IP / HTTP / MQTT    |
             +-----------+-------------+
                         |
                         |
                    Cellular Network
                         |
                         ▼
                  Cloud Server
 ############################################################# SOFTWARE ARCHITECTURE ###########################################################################

 main()
│
├── System_Init()
├── UART1_Init()      // GPS
├── UART2_Init()      // A7670C
├── RingBuffer_Init()
│
└── while(1)
    │
    ├── Read GPS sentence
    │
    ├── Parse $GNRMC
    │
    ├── Store:
    │     • Latitude
    │     • Longitude
    │     • Speed
    │     • UTC Time
    │
    ├── Build telemetry packet
    │
    ├── Send AT commands
    │
    └── Upload to cloud

 


# STM32 GPS & LTE Telematics System - Stage 1

An ARM Cortex-M3 (STM32F103C8T6) firmware application designed to interface with GPS and LTE communication modules. This stage focuses on peripheral initialization, clock configuration, and serial debugging layout.

## 🛠️ Development Stack & Tools
* **Microcontroller:** STM32F103C8T6 (Blue Pill)
* **IDE:** STM32CubeIDE (v1.x)
* **Flashing & Diagnostics:** STM32CubeProgrammer via ST-Link V2
* **Debugging Terminal:** Tera Term via CH340G USB-to-TTL Converter

## 🔌 Hardware Setup (Stage 1)
* **Debug Port:** UART1 (TX=PA9, RX=PA10) configured at 115200-8-N-1 for system logs.
* **Programming Interface:** SWD (TMS/TCK) via ST-Link V2.


