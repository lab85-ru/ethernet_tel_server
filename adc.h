#ifndef ADC_H_
#define ADC_H_

//#include <stdint.h>

void adc_channel_setup(uint8_t adc_channel);
void adc_init(void);
void adc_start(void);

#endif /* ADC_H_ */
