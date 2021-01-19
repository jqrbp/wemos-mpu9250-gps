#include <stdio.h>
#include "SparkFunMPU9250-DMP.h"
#include "webUtils.h"

MPU9250_DMP imu;
static char imuTxtBuffer[75];

float qw,qx,qy,qz;
float ax,ay,az;
float heading;

bool print_flag = false;

void printIMUData(void)
{  
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
}

void imu_loop(void) {
    // Check for new data in the FIFO
    if ( imu.fifoAvailable() )
    {
        // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
        if ( imu.dmpUpdateFifo() == INV_SUCCESS)
        {
            imu.updateCompass();
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

            heading = imu.computeCompassHeading();
            // Serial.println("accel=" + String(imu.getAccelSens()) + "," + String(imu.ax)+ "," + String(imu.ay)+ "," + String(imu.az));
            
            print_flag = true;
        } else {
          Serial.println("error: failed to dmp update fifo");
        }
    }

    printIMUData();
}

void imu_setup(void) {
  // Call imu.begin() to verify communication and initialize
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
  } else {
    Serial.println("Unable to communicate with MPU-9250");
    Serial.println("Check connections, and try again.");
    Serial.println();
  }
}