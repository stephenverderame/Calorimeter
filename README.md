# Calorimeter
## Project Description
The idea is to create a mask that is worn over the mouth and nose in order to measure VO2 and VCO2 (Volume of oxygen inhaled and volume of carbon dioxide exhaled).
With these metrics it would then be able to use indirect calorimetry to accurately measure calorie expenditure.
The system acts as a UDP client to remotely send the data to a computer for calculations and analysis.
### Sensors
Arduino Uno
NodeMCU ESP8266
K30 CO2 Sensor
Wind Sensor Rev C

## Wiring
Arduino -- NodeMCU via UART with SoftwareSerial
2 (RX) -- D7 (TX)
3 (TX) -- D6 (RX)
5V -- Common 5V -- Vin
GND -- Common GND -- GND

Arduino -- K30 via UART with SoftwareSerial
12 (RX) -- TX
13 (TX) -- RX
GND -- Common GND -- GND
9V Battery -- Vin

Arduino -- WindSensor 
A0 -- RV
A1 -- TMP
5V -- Common 5V -- Vin
GND -- Common GND -- GND

## Design
With air passing through a hole of a known radius, Volume Flow can be calculated as V = va.
Multiplying the volume flow by the concentration of oxygen and carbon dioxide would procure our VO2 and VCO2 metrics.
With these metrics, it would then be possible to calculate the respiratory quotient RQ = VCO2/VO2 which is dependent on the type of molecule being burned.
Using a forumula such as the Weir Equation (P = 3.94VO2 + 1.11VCO2) it would then be possible to calculate power and energy expenditure.
Every half second, the O2, CO2 and wind speed measurements are sent to a remote computer via UDP.
The ESP must be on the same LAN with the server. The network and server ip can be configured by connecting to the Calorimeter Wifi AP and
accessing the ESP Config page via HTTP and ip 192.168.4.1. The server and client communicate over port 8000.

## Usage
With this device it would be possible to calculate REE (resting energy expenditure), total energy expenditure during the time period the
device is used and VO2Max. VO2Max can be calculated by performing a ramp test or FTP test while wearing the mask. The highest VO2 calculated
would then be the VO2Max. Since HR / Max HR is proportional to VO2 / VO2 Max it would then be feasible to get a decently accurate measurement
of calorie expenditure using only a heart rate monitor.
The planned usage for this mask is both for racing and to also analyze work while playin VR games.

*Note: This is still a WIP and some steps I haven't fully gotten up to yet, therefore I still aim to do deeper research to make sure my
understanding and calculations are correct.
