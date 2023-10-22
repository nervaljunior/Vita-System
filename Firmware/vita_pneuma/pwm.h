#ifndef PWM_H_
#define PWM_H_

#ifdef MOTOR_PWM
  #define MAX_PWM_VALUE 255
  void pwm_init(volatile long *pulsePosition, volatile long *finalPosition, volatile char *pulseDirection);
  void pwm_stop();



  #ifndef ACCELERATE_EVERY_PULSE
    void pwm_startRun();
    // freq changes the number of rotation/minute as shown here:
    // https://docs.google.com/spreadsheets/d/1eAW1s3_Mi-JtDXamgpoyo9Gr9R0HBD_HItr-XjfLl98/edit?usp=sharing
    void pwm_changeFreq(unsigned char freq);
  #else
    void pwm_startRun(unsigned char startFreqSpeed, unsigned char targetFreqSpeed);
  #endif


  bool pwm_isRunning();
  void pwm_configure_prescaler(unsigned char prescaler);

  extern volatile unsigned long g_cnt_pulses;
  enum {
    PWM_PRESCALE_1 = 0x01,
    PWM_PRESCALE_8 = 0x02,
    PWM_PRESCALE_32 = 0x03,
    PWM_PRESCALE_64 = 0x04,
    PWM_PRESCALE_128 = 0x05,
    PWM_PRESCALE_256 = 0x06,
    PWM_PRESCALE_1024 = 0x07
  };
#endif

#endif // PWM_H_
