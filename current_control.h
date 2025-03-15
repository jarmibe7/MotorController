#ifndef CURRENT_CONTROL__H__
#define CURRENT_CONTROL__H__

void set_pwm_dc(int dc);
void set_curr_kp(float kp);
void set_curr_ki(float ki);
float get_curr_kp();
float get_curr_ki();

void Current_Control_Startup(void);
static void pwm_setup(void);
static void current_controller_setup(void);
void make_waveform();
void send_plot_data();


#endif // CURRENT_CONTROL__H__