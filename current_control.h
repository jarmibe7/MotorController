#ifndef CURRENT_CONTROL__H__
#define CURRENT_CONTROL__H__

float set_torque(float tor);
void set_pwm_dc(int dc);
void set_curr_kp(float kp);
void set_curr_ki(float ki);
void set_curr_kd(float kd);
float get_curr_kp();
float get_curr_ki();
float get_curr_kd();

void Current_Control_Startup(void);
static void pwm_setup(void);
static void current_controller_setup(void);
void make_waveform();
void send_curr_data();


#endif // CURRENT_CONTROL__H__