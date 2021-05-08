# wemo-mpu9250-gps

Interfacing MPU9250 (and gps module) to WEMOS D1 MINI PRO

## Requirement for embedding the webpages to the board

To embed the html static page, the board must have at least 4MB Flash.
The pages can be uploaded to the board using Arduino's LittleFS add-ons:
https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases

## References

It is almost impossible to develop this project without helps / references, especially from:

1. [Magnetometer Calibration:kriswiner-mpu9250] (https://github.com/kriswiner/MPU9250)
2. [Digital Motion Processing:sparkfun-mpu9250-dmp] (https://github.com/sparkfun/SparkFun_MPU-9250-DMP_Arduino_Library)
3. [Tilt Compensation Based on DMP's Quaternion:linux-mpu9150] https://github.com/mlaurijsse/linux-mpu9150

## Axes definition

For tilt-compensation, axes alignment between modules and calculation angles is very important.
This work uses this axes alignment:

1. Accelerometer & gyroscope / DMP axes: 
   - X axis direction (right wing down) = euler roll positive, 
   - Y axis (nose down) = euler pitch negative,
   - Z axis (counter clockwise) = euler yaw negative

2. Magnetometer:
   - X axis (nose down) = euler pitch negative,
   - Y axis (right wing down) = euler roll positive,
   - Z axis (clockwise) = euler yaw positive

Summary:
   - X-accel = Y-magnetometer => (positive)Roll
   - Y-accel = X-magnetometer => (negative)Pitch
   - Z-accel = (negative)Z-magnetometer => (negative)Yaw
