#ifndef POSITION_CONTROL__H__
#define POSITION_CONTROL__H__

void set_angle(int ang);
void set_pos_kp(float kp);
void set_pos_ki(float ki);
void set_pos_kd(float kd);
float get_pos_kp();
float get_pos_ki();
float get_pos_kd();

void Position_Control_Startup(void);
void read_traj();
void send_pos_data();

#endif // POSITION_CONTROL__H__