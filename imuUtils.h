#ifndef __imu_Utils_h__
#define __imu_Utils_h__

void toggle_serial_debug_imu_flag(void);
void imu_setup(void);
void imu_loop(void);
void imu_set_calib_flag(bool _flag);

#endif // def(__imu_Utils_h__)