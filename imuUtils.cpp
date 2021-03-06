#include <stdio.h>
#include "MPU9250-DMP.h"
#include "webUtils.h"
#include <LittleFS.h>
#include "fileUtils.h"
#include "gpsUtilsWrapper.h"

extern "C" {
#include "quaternion.h"
}

MPU9250_DMP imu;
static char imuTxtBuffer[190];

float qw,qx,qy,qz;
float ax,ay,az, mx, my, mz;

int intPin = 14;

bool print_flag = false;
uint32_t printTime = 0;
const uint32_t printTimeLimit = 500;
bool serial_debug_imu_flag = false;
bool imu_update_fail_flag = false;
bool imu_update_fail_log_flag = true;
int update_compass_flag = INV_SUCCESS;

//calibration variables
bool magCalibFlag = false;
uint32_t magCalibTime = 0;
uint32_t magCalibDT = 0;
uint16_t magCalibIdx = 0;
uint16_t magRate = 8;

char magCalibParamFile[18] = "magCalibParam.txt";
float magBias[3] = {-52.0f, 322.0f, -165.0f};
float magScale[3] = {1.03, 0.95, 1.03};
int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};

void toggle_serial_debug_imu_flag(void) {
  serial_debug_imu_flag = !serial_debug_imu_flag;
}

void printIMUDataRaw(void)
{  
  // After calling update() the ax, ay, az, gx, gy, gz, mx,
  // my, mz, time, and/or temerature class variables are all
  // updated. Access them by placing the object. in front:

  // Use the calcAccel, calcGyro, and calcMag functions to
  // convert the raw sensor readings (signed 16-bit values)
  // to their respective units.
  float accelX = imu.calcAccel(imu.ax);
  float accelY = imu.calcAccel(imu.ay);
  float accelZ = imu.calcAccel(imu.az);
  float gyroX = imu.calcGyro(imu.gx);
  float gyroY = imu.calcGyro(imu.gy);
  float gyroZ = imu.calcGyro(imu.gz);
  float magX = imu.calcMag(imu.mx);
  float magY = imu.calcMag(imu.my);
  float magZ = imu.calcMag(imu.mz);
  
  Serial.print("imu raw:");
  Serial.print(String(imu.dmpEuler[VEC3_X]) + "," + String(imu.dmpEuler[VEC3_Y]) + "," + String(imu.dmpEuler[VEC3_Z]) + "\t");
  Serial.print(String(imu.ax)+ "," + String(imu.ay)+ "," + String(imu.az) + "\t");
  Serial.print(String(imu.mx)+ "," + String(imu.my)+ "," + String(imu.mz)+"\t");
  Serial.print(String(update_compass_flag) +"\t" + String(imu.heading, 2));
  Serial.println();

  Serial.println("Accel: " + String(accelX) + ", " +
              String(accelY) + ", " + String(accelZ) + " g");
  Serial.println("Gyro: " + String(gyroX) + ", " +
              String(gyroY) + ", " + String(gyroZ) + " dps");
  Serial.println("Mag: " + String(magX) + ", " +
              String(magY) + ", " + String(magZ) + " uT");
  Serial.println("Time: " + String(imu.time) + " ms");
  Serial.println();
}

void printIMUData(void)
{  
  if (millis() - printTime > printTimeLimit) {
    if (print_flag) {
      sprintf(imuTxtBuffer, "{\"loc1\":[%.5f,%.5f],\"loc2\":[%.5f,%.5f],\"qw\":%.3f,\"qx\":%.3f,\"qy\":%.3f,\"qz\":%.3f,\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,\"mx\":%.2f,\"my\":%.2f,\"mz\":%.2f,\"h\":%.2f}", 
        gpsUtilsWrapper_get_latitude(1),gpsUtilsWrapper_get_longitude(1),gpsUtilsWrapper_get_latitude(2),gpsUtilsWrapper_get_longitude(2),
        (float)imu.fusedQuat[QUAT_W], (float)imu.fusedQuat[QUAT_X], (float)imu.fusedQuat[QUAT_Y], (float)imu.fusedQuat[QUAT_Z],
        ax, ay, az, mx, my, mz, imu.heading);
      SSEBroadcastTxt(imuTxtBuffer);
      print_flag = false;
    }

    if (serial_debug_imu_flag) {
      if (imu_update_fail_flag) {
        Serial.println("imu fifo update failed");
        if (imu_update_fail_log_flag) {
          appendFile("log.txt", "imu fifo update failed\r\n");
          imu_update_fail_log_flag = false;
        }
        return;
      }
      printIMUDataRaw();
    }
  }
}

void imu_set_calib_flag(bool _flag) {
  magCalibFlag = _flag;
}

int magCal_nonblocking(float * dest1, float * dest2) {
	uint16_t sample_count = 0;
	int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};

	if(magCalibTime == 0) {
		Serial.println("Mag Calibration: Wave device in a figure eight until done!");
    magRate = imu.getCompassSampleRate();
		if(magRate ==0) {
			Serial.println("cannot read compass sample rate");
			return 1;
		};
		Serial.print("Mag sample rate: "); Serial.println(magRate, DEC);
		for (uint8_t i = 0; i < 3; i++) {
			mag_max[i] = -32767;
			mag_min[i] = 32767;
		}
		magCalibTime = millis();
		magCalibDT = 4000;
	} else {
		if(millis() - magCalibTime >= magCalibDT) {
			// shoot for ~fifteen seconds of mag data
			if(magRate <= 10) sample_count = 128;  // at 8 Hz ODR, new mag data is available every 125 ms
			else sample_count = 1500;  // at 100 Hz ODR, new mag data is available every 10 ms
			
			if(magCalibIdx < sample_count) {
				if(imu.updateCompass() != INV_SUCCESS) {
					Serial.println("cannot get mag data");
          return 0;
				} // Read the mag data
        mag_temp[0] = imu.mx;
        mag_temp[1] = imu.my;
        mag_temp[2] = imu.mz;
				for (int jj = 0; jj < 3; jj++) {
					if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
					if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
				}
				Serial.print(".");
				if(magRate <= 10) magCalibDT = 135;  // at 8 Hz ODR, new mag data is available every 125 ms
				else magCalibDT = 12;  // at 100 Hz ODR, new mag data is available every 10 ms
				
        sprintf(imuTxtBuffer,  "{\"mcx\":%lu,\"mci\":%lu,\"mx\":%d,\"my\":%d,\"mz\":\"%d\"}", 
          sample_count, magCalibIdx, mag_temp[0], mag_temp[1], mag_temp[2]);
        SSEBroadcastTxt(imuTxtBuffer);

        magCalibIdx++;
				magCalibTime = millis();
			} else {
        Serial.println(".");
				//    Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
				//    Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
				//    Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);

				// Get hard iron correction
				mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
				mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
				mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts

				dest1[0] = (float) mag_bias[0];  // save mag biases in G for main program
				dest1[1] = (float) mag_bias[1]; 
				dest1[2] = (float) mag_bias[2];

				// Get soft iron correction estimate
				mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
				mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
				mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts

				float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
				avg_rad /= 3.0;

				dest2[0] = avg_rad/((float)mag_scale[0]);
				dest2[1] = avg_rad/((float)mag_scale[1]);
				dest2[2] = avg_rad/((float)mag_scale[2]);

				Serial.println("Mag Calibration done!");
				Serial.print("bias: "); Serial.print(dest1[0]); Serial.print(", "); Serial.print(dest1[1]); Serial.print(", "); Serial.print(dest1[2]); Serial.println();
        Serial.print("scale: "); Serial.print(dest2[0]); Serial.print(", "); Serial.print(dest2[1]); Serial.print(", "); Serial.print(dest2[2]); Serial.println();
				sprintf(imuTxtBuffer,  "{\"mbx\":%.2f,\"mby\":%.2f,\"mbz\":\"%.2f\",\"msx\":%.2f,\"msy\":%.2f,\"msz\":\"%.2f\"}", 
          dest1[0], dest1[1], dest1[2], dest2[0], dest2[1], dest2[2]);
        SSEBroadcastTxt(imuTxtBuffer);
        sprintf(imuTxtBuffer,  "%d\n%d\n%d\n%d\n%d\n%d\n", 
          (int)(dest1[0] * 10000.0f), (int)(dest1[1] * 10000.0f), (int)(dest1[2] * 10000.0f), 
          (int)(dest2[0] * 10000.0f), (int)(dest2[1] * 10000.0f), (int)(dest2[2] * 10000.0f));
        writeFile(magCalibParamFile, imuTxtBuffer);
        magCalibFlag = false;
				magCalibTime = 0;
				magCalibIdx = 0;
			}
		}
	}
	return 0;
}

void imu_send_mag_calib(void) {
  sprintf(imuTxtBuffer,  "{\"mbx\":%.2f,\"mby\":%.2f,\"mbz\":\"%.2f\",\"msx\":%.2f,\"msy\":%.2f,\"msz\":\"%.2f\"}", 
          magBias[0], magBias[1], magBias[2], magScale[0], magScale[1], magScale[2]);
  SSEBroadcastTxt(imuTxtBuffer);
}

void imu_loop(void) {
  if (magCalibFlag) {
      magCal_nonblocking(magBias,magScale);
  } else {
    update_compass_flag = imu.updateCompass();
    if (update_compass_flag == INV_SUCCESS) {
      mx = (float)imu.mx - magBias[0] * magScale[0];// imu.calcMag(imu.mx);
      my = (float)imu.my - magBias[1] * magScale[1];// imu.calcMag(imu.my);
      mz = (float)imu.mz - magBias[2] * magScale[2];
    }

    // Check for new data in the FIFO
    if ( imu.fifoAvailable() )
    {
        // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
        if ( imu.dmpUpdateFifo() == INV_SUCCESS)
        {          
            ax = imu.calcAccel(imu.ax);
            ay = imu.calcAccel(imu.ay);
            az = imu.calcAccel(imu.az);

            imu.computeFusedCompass(mx, my, mz);

            imu_update_fail_flag = false;
            print_flag = true;
        } else {
          imu_update_fail_flag = true;
        }
    }
  }
  printIMUData();
}

void imu_setup(void) {
  // get magBias and magScale value from file
  File f;
  f=LittleFS.open(magCalibParamFile,"r");
  if(f){
    Serial.println("Loading magnetometer calibration parameter from file system.");
    String mod=f.readString(); //read the file to a String
    int index_1=mod.indexOf('\n',0); //locate the first line break
    int index_2=mod.indexOf('\n',index_1+1); //locate the second line break
    int index_3=mod.indexOf('\n',index_2+1); //locate the third line break

    magBias[0]=(float)mod.substring(0,index_1-1).toInt() / 10000.0f;
    magBias[1]=(float)mod.substring(index_1+1,index_2-1).toInt() / 10000.0f;
    magBias[2]=(float)mod.substring(index_2+1,index_3-1).toInt() / 10000.0f;

    index_1=mod.indexOf('\n',index_3+1);
    index_2=mod.indexOf('\n',index_1+1);
    index_3=mod.indexOf('\n',index_2+1);

    magScale[0]=(float)mod.substring(0,index_1-1).toInt() / 10000.0f;
    magScale[1]=(float)mod.substring(index_1+1,index_2-1).toInt() / 10000.0f;
    magScale[2]=(float)mod.substring(index_2+1,index_3-1).toInt() / 10000.0f;
    f.close();
  }

  // Call imu.begin() to verify communication and initialize
  pinMode(intPin, INPUT_PULLUP);
  if (imu.begin(400000) == INV_SUCCESS)
  {
    Serial.println("\r\nStarting MPU-9250");
    imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | // Enable 6-axis quat
                DMP_FEATURE_GYRO_CAL | // Use gyro calibration
                // DMP_FEATURE_SEND_CAL_GYRO | // send calibrated gyro values
                DMP_FEATURE_SEND_RAW_ACCEL, // send raw accelerometer
                10); // Set DMP FIFO rate to 10 Hz
    // DMP_FEATURE_LP_QUAT can also be used. It uses the 
    // accelerometer in low-power mode to estimate quat's.
    // DMP_FEATURE_LP_QUAT and 6X_LP_QUAT are mutually exclusive
    imu.resetFifo();
     // Enable all sensors, and set sample rates to 4Hz.
    // (Slow so we can see the interrupt work.)
    // imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    // imu.setSampleRate(4); // Set accel/gyro sample rate to 4Hz
    // imu.setCompassSampleRate(4); // Set mag rate to 4Hz

    // // Use enableInterrupt() to configure the MPU-9250's 
    // // interrupt output as a "data ready" indicator.
    // imu.enableInterrupt();

    // // The interrupt level can either be active-high or low.
    // // Configure as active-low, since we'll be using the pin's
    // // internal pull-up resistor.
    // // Options are INT_ACTIVE_LOW or INT_ACTIVE_HIGH
    // imu.setIntLevel(INT_ACTIVE_LOW);

    // // The interrupt can be set to latch until data has
    // // been read, or to work as a 50us pulse.
    // // Use latching method -- we'll read from the sensor
    // // as soon as we see the pin go LOW.
    // // Options are INT_LATCHED or INT_50US_PULSE
    // imu.setIntLatched(INT_LATCHED);
  } else {
    Serial.println("Unable to communicate with MPU-9250");
    Serial.println("Check connections, and try again.");
    Serial.println();
  }

  printTime = millis();
}