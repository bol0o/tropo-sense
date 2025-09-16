![image](images/logo.png)
# 🌦️ A Self-Sustainable Weather Station

TropoSense is a **low-power**, **standalone** weather station designed for **reliability** and **remote data collection**. It uses AVR microcontrollers to monitor and transmit meteorological data via GSM, with a focus on power efficiency and **modularity**.

For the full development log, **[click here](docs/dev-log.md)**.

## 📡 Features

- Monitors: **ground and air temperature**, **humidity**, **pressure**, **wind speed**, **wind direction**, and **rainfall**
- Built with raw AVR microcontrollers
- Modular 2-MCU system:
    - **Main AVR**: Gathers data from sensors
    - **Monitoring AVR**: Continuously tracks real-time weather activity
- Uses **GSM module** for communication
- Designed to be self-sustainable and efficient for long-term deployment

## 🔧 Architecture

### Main AVR
- Collects:
    - BME280 for pressure, humidity, and temperature
    - DS18B20 for ground temperature
    - Handles GSM communication via A7670E

### Monitoring AVR
- Continuously monitors:
    - Rain gauge
    - Anemometer (wind speed)
    - Wind vane (direction)


## ⚡ Power Efficiency

Troposense is optimized for ultra-low power consumption:

- Minimal power draw in idle states
- Efficient GSM bursts for data transmission
- Lightweight custom firmware on both AVRs

## 🚧 Work In Progress
Troposense is in the early stages of a redesign from a previous version. The construction phase breakdown is as follows:

### Stage 1:
- ✅ Designing the hardware
- ☐ Writing the software
    - ✅ BME280
    - ✅ DS18B20
    - ☐ GSM
    - ☐ Real-Time monitoring AVR
    - ☐ DS3231
    - ☐ INA219
- ☐ Power performance tests
- ☐ Ruggedness tests
- ☐ Designing the PCB

### Stage 2:
- ☐ Assembling the final version of the station 
- ☐ Second phase of ruggedness and power testing

### Stage 3:
- ☐ Writing the website
- ☐ Writing the mobile app