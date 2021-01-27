#include <stdio.h>
#include "SparkFunMPU9250-DMP.h"
#include "webUtils.h"
#include "fileUtils.h"

MPU9250_DMP imu;
static char imuTxtBuffer[140];

float qw,qx,qy,qz;
float ax,ay,az, mx, my, mz;
float heading;

int intPin = 14;

bool print_flag = false;
uint32_t printTime = 0;
const uint32_t printTimeLimit = 500;
bool serial_debug_imu_flag = true;
bool imu_update_fail_flag = false;
bool imu_update_fail_log_flag = true;

//calibration variables
bool magCalibFlag = false;
uint32_t magCalibTime = 0;
uint32_t magCalibDT = 0;
uint16_t magCalibIdx = 0;
uint16_t magRate = 8;

float magBias[3] = {-52.0f, 322.0f, -165.0f};
float magScale[3] = {1.03, 0.95, 1.03};
int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};

void toggle_serial_debug_imu_flag(void) {
  serial_debug_imu_flag = !serial_debug_imu_flag;
}

void printIMUData(void)
{  
  if (millis() - printTime > printTimeLimit) {
    if (print_flag) {
      sprintf(imuTxtBuffer, "{\"qw\":%.4f,\"qx\":%.4f,\"qy\":%.4f,\"qz\":%.4f,\"r\":%.2f,\"p\":%.2f,\"y\":%.2f,\"ax\":%.4f,\"ay\":%.4f,\"az\":%.4f,\"h\":%.2f}", 
        qw, qx, qy, qz, imu.roll, imu.pitch, imu.yaw, ax, ay, az, heading);
      SSEBroadcastTxt(imuTxtBuffer);
      print_flag = false;
    }
    // SSE_add_char(imuTxtBuffer);
    // Serial.println(imuTxtBuffer);
    // Serial.println("Q: " + String(q0, 4) + ", " +
    //                   String(q1, 4) + ", " + String(q2, 4) + 
    //                   ", " + String(q3, 4));
    // Serial.println("R/P/Y: " + String(imu.roll) + ", "
    //           + String(imu.pitch) + ", " + String(imu.yaw));
    // Serial.println("Time: " + String(imu.time) + " ms");
    // Serial.println();
    if (serial_debug_imu_flag) {
      if (imu_update_fail_flag) {
        Serial.println("imu fifo update failed");
        if (imu_update_fail_log_flag) {
          appendFile("log.txt", "imu fifo update failed\r\n");
          imu_update_fail_log_flag = false;
        }
      } else {
        Serial.println("imu raw:"+String(imu.ax)+ "," + String(imu.ay)+ "," + String(imu.az)+"," + String(imu.mx)+ "," + String(imu.my)+ "," + String(imu.mz));
      }
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
				Serial.println(".");
				if(magRate <= 10) magCalibDT = 135;  // at 8 Hz ODR, new mag data is available every 125 ms
				else magCalibDT = 12;  // at 100 Hz ODR, new mag data is available every 10 ms
				magCalibIdx++;
				magCalibTime = millis();
			} else {
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
				magCalibFlag = false;
				magCalibTime = 0;
				magCalibIdx = 0;
			}
		}
	}
	return 0;
}  

void imu_loop(void) {
  if (magCalibFlag) {
      magCal_nonblocking(magBias,magScale);
  } else {
    if (imu.updateCompass() == INV_SUCCESS) {
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
            // computeEulerAngles can be used -- after updating the
            // quaternion values -- to estimate roll, pitch, and yaw
            imu.computeEulerAngles();
            // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
            // are all updated.
            // Quaternion values are, by default, stored in Q30 long
            // format. calcQuat turns them into a float between -1 and 1
            qw = imu.calcQuat(imu.qw);
            qx = imu.calcQuat(imu.qx);
            qy = imu.calcQuat(imu.qy);
            qz = imu.calcQuat(imu.qz);

            ax = imu.calcAccel(imu.ax);
            ay = imu.calcAccel(imu.ay);
            az = imu.calcAccel(imu.az);

            heading = imu.calcCompassHeadingTilt(-ay, ax, az, mx, -my, mz);
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
  // Call imu.begin() to verify communication and initialize
  pinMode(intPin, INPUT);
  if (imu.begin(4000000) == INV_SUCCESS)
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
  } else {
    Serial.println("Unable to communicate with MPU-9250");
    Serial.println("Check connections, and try again.");
    Serial.println();
  }

  printTime = millis();
}