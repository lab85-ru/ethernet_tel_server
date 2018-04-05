#ifndef TASK_UIP_H_
#define TASK_UIP_H_

void vTask_uIP(void *pvParameters);
void load_i2c_mul_div(void);
int calc_volt_rdiv(char *buf, const int adc_in, const int k1, const int k2, const int mu, const int di);

#endif
