#ifndef __ENV_CONFIG_H_
#define __ENV_CONFIG_H_


// === i2c ====================================================================================
// peremenii lihat posledovatelno !!!
#define SN_LEN		 5	// serial numer len
#define TXT_LEN		 60	// TXT_ELN

#define SETPORTLAN_LEN	 8	// char-txt maximum 12345 + \0
#define SETIPLAN_LEN	 16	// char-txt maximum 123.456.789.123 + \0

#define TXT_REM_LEN	 32	// коментарий к входам выходам
#define TXT_MULDIV_LEN   6      // 12345 +\0 (множительи делитель для расчета вх. напряжения на вх. делителя)




// номер записи по порядку
#define R_SN        0
#define R_TXT       1
#define R_IP_DEVICE 2
#define R_IP_GW     3
#define R_IP_MASK   4
#define R_UDP_PORT  5

#define R_ADC_0     6
#define R_ADC_1     7
#define R_ADC_2     8
#define R_ADC_3     9
#define R_ADC_4     10
#define R_ADC_5     11
#define R_ADC_6     12
#define R_ADC_7     13

#define R_INPUT_0   14
#define R_INPUT_1   15
#define R_INPUT_2   16
#define R_INPUT_3   17
#define R_INPUT_4   18
#define R_INPUT_5   19
#define R_INPUT_6   20
#define R_INPUT_7   21

#define R_OUTPUT_0  22
#define R_OUTPUT_1  23
#define R_OUTPUT_2  24
#define R_OUTPUT_3  25
#define R_OUTPUT_4  26
#define R_OUTPUT_5  27
#define R_OUTPUT_6  28
#define R_OUTPUT_7  29

#define R_MUL_0 30
#define R_DIV_0 31
#define R_MUL_1 32
#define R_DIV_1 33
#define R_MUL_2 34
#define R_DIV_2 35
#define R_MUL_3 36
#define R_DIV_3 37
#define R_MUL_4 38
#define R_DIV_4 39
#define R_MUL_5 40
#define R_DIV_5 41
#define R_MUL_6 42
#define R_DIV_6 43
#define R_MUL_7 44
#define R_DIV_7 45



typedef struct {
	uint16_t name; // номер записи
	uint16_t size; // размер записи
} values_st;

typedef struct {
	uint16_t adr; // адрес записи
	uint16_t size; // размер записи
} values_adr_size_st;

void print_env(void);
uint8_t env_read_value(const uint8_t rec, char *buf);
uint8_t env_write_value(const uint8_t rec, char *buf);


#endif /*ENV_CONFIG_H_*/
