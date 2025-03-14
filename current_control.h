#ifndef CURRENT_CONTROL__H__
#define CURRENT_CONTROL__H__

void set_pwm_dc(int dc);

void Current_Control_Startup(void);
static void pwm_setup(void);
static void current_controller_setup(void);


#endif // CURRENT_CONTROL__H__