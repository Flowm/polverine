## POLVERINE_DEMO: Stream sensor data over USB

### A Complete yet Simple Demo

This firmware collects sensor measurements and transmits them using the ESP32-S3 module's native USB connection. 

The system also benefits from the latest SDK for the BMV080 sensor, which is now publicly available. This SDK enables the reading of particulate matter (PM) values for PM10, PM2.5, and PM1. The full range of sensed quantities includes:

- Temperature
- Relative Humidity
- Barometric Pressure
- Volatile Organic Compounds (V.O.C.)
- Particulate Matter 10 (PM10)
- Particulate Matter 2.5 (PM2.5)
- Particulate Matter 1 (PM1)

### Data Visualization Made Easy

To facilitate data analysis and visualization, a separate NodeRed ["RealTime Flows"]((../nodered/client_rt_flows.json)) interface has been developed. This interface collects data from USB connection and plots data in real time. 
![](../images/client_flow_realtime.png)

![](../images/client_dash_realtime.png)

### Adding Bosch SDKs to Polverine source code

The POLVERINE_DEMO communicates with the Bosch BME690 & BMV080 sensors and need to use the SDKs published by Bosch Sensortec.

The SDKs source code and compiled libraries cannot be stored in this repository, but must be downloaded by the user from the Bosch website by accepting the licence agreement.

- [BMV080 SDK r.11.0.0](https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/sdk-v11-0-0.html)
- [BME690 BSEC r.3.1.0](https://www.bosch-sensortec.com/software-tools/double-opt-in-forms/bsec-software-3-1-0-0-form-1.html)


#### Bosch BMV080 SDK
1) Unzip the downloaded BMV080 SDK file 
2) move the SDK's folder into the ../deps/bosch-sensortec/ folder
3) and rename it as: "BMV080-SDK"

The folder "BMV080-SDK" should have the following subfolders and/or files:

- CHANGELOG.md  
- LICENSE.md  
- README.md  
- api  
- api_examples"

#### Bosch BME690 BSEC Library
1) Unzip the downloaded BSEC Library file 
2) move the library's folder into the ../deps/bosch-sensortec/ folder
3) and rename it as: "bsec_library_v3.x"
 
The folder "bsec_library_v3.x" should have at least the following subfolders and/or files:

- algo
- examples


### Firmware Upload
The POLVERINE_DEMO puts the ESP32-S3 MCU in low power mode that disables USB. To upload the firmware using USB port you must put Polverine in boot mode: 

1. press boot button (SW1 - BOOT)
2. press reset button (SW2 - EN)
3. release reset button
4. release boot button
5. upload firmware
6. press reset button
7. release reset button


When in bootloader mode the RGB led is lit.

### Low Power Mode

The application is using ESP32 Low Power Mode. To use full power mode comment out the call to:

	pm_init();

in /src/main.c, in function main(), line 89

USB Transmission interferes with Low power mode, enabling Light Sleep in ESP32 is ineffective.
