Recorder - Sensor Data Sampling Example
-----------------------------------------------

Application for B-U585I-IOT02A board demonstrates sampling of sensors (temperature, humidity, pressure, accelerometer, gyroscope and magnetometer).
Data is captured from the sensors and then printed out via UART.
In addition, if RECORDER_ENABLED is defined, samples are also send via Wifi (UDP socket) to the PC, where simple Python script Recorder.py is capturing and saving the data to the .csv file.

Individual sensor can be enabled or disabled via following defines:
 - #define SENSOR_TEPM_ENABLED      1
 - #define SENSOR_HUM_ENABLED       1
 - #define SENSOR_PRESS_ENABLED     1
 - #define SENSOR_ACC_ENABLED       1
 - #define SENSOR_GYRO_ENABLED      1
 - #define SENSOR_MAG_ENABLED       1

Application thread "read_sensors", is reading the statuses of the sensors. When new sensor data is available, the sensor data is read and printed to the uart.


## Recorder
Main recorder functionality is implemented in Recorder.c/.h and exports two functions:
- int32_t Recorder_Initialize (const uint8_t * cfg_str, uint32_t len)  
Function Recorder_Initialize initializes the recorder interface, creates recorder worker thread and also sends "configuration" string to the PC. Configuration string is provided as a parameter from the application and it is basically first line of the CSV file, which holds the names of the columns.
(Example of the command string: "Timestamp, Humidity, Pressure, Acc_x, Acc_y, Acc_z")
- int32_t Recorder_Write (uint8_t * data, uint32_t sz)  
Recorder_Write function saves data samples to the internal circular buffer.

Recorder worker thread "thr_Recorder" is checking if there is new data available in the internal circular buffer and sends this data to PC.

Recorder interface layer is separated and implemented in Recorder_IF.c/.h. Current implementation uses WiFi and iot UDP socket to transfer the data to the PC.

### Configuration
Recorder is enabled with RECORDER_ENABLED define.

Configuration (in Recorder_IF.c):
 - SSID     (WiFi ssid)
 - PASS     (WiFi Password)
 - SECURITY (WiFi Security)
 - IP       (IP of the host PC, where Recorder.py is executed)

### Workflow
- Do the configuration as described above and build the application
- Run the Python script Recorder.py on PC (make sure that B-U585I-IOT02A are in the same network)
- Run the application on B-U585I-IOT02A
- Python script will log 128 data samples (currently hardcoded limitation) and save them to csv.
