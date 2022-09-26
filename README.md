# heart-sensor-azure-iot 

This repository provides a sample ESP32 code how to collect a MAX30102 data and send the reading to Azure IoT Hub.

> **Please note**: The code for deployment of Azure and Power BI resources is not included here. This intent of this repository is to share the ESP32 code and its approach to send telemetry data to Azure IoT Hub (MQTT vs REST)



## The sketch
The ESP32 sketch consists of two IC components.

The VIN and GND of both I2C components are connected to the ESP32's VIN and GND. The I2C bus' SCL and SDL are connected as below

```
I2C SCL - > ESP32 Pin D22 (GIPO 22, Green cables in the images below) 
I2C SDA - > ESP32 Pin D21 (GIPO 21, Yellow cables in the images below) 
```

![MAX30102 ESP32 Sketch](images/sketch.png)


#### 1 - MAX30102 pulse oxidimetry and heart-rate monitor biosensor module:

The MAX30102 sensor accuracy depends on its stability and its IR sensor needs to be away from direct light. As such the sensor is mounted in Pulse Oximeter Finger Clip:  

![MAX30102 Clip](images/MAX30102-clip.png)

The MAX30102 clip is 3D printed based on extended version of the following open 3D model from Thingiverse: 

https://www.thingiverse.com/thing:4395147

![MAX30102 Clip](images/clip1.png)

#### 2- Arduino Serial I2C 1602 16×2 Character LCD Module

The display shows the WIFI status, IP Address, and the current sensor reading. It is used for tracing and debugging propose, which is not essential for sending heartbeat telemetry data.

## Azure IoT Hub Setup


The following diagram depicts the Azure IoT setup:

![MAX30102 Clip](images/azure-iot.png)

#### Realtime Visualization
The completed work allows to track the MAX30102 reading realtime :

![MAX30102 realtime](images/realtime-visualization.png)




# Next steps for extending this idea:

1. MAX30102 sensor can sense blood oxygen level and body temperature.
1. The noise cancellation algorithm can improve by leveraging the Azure Stream 1. Analytics power
1. Anomaly detection and alerts
1. The Power BI visualization presented in this POC is basic and raw. The visualization can significantly improve


