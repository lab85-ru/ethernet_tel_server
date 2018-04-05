/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include "printf_hal.h"
#include "main.h"
#include "init_hardware.h"
#include "env_config.h"
#include "task_uip.h"
#include "time_hal.h"
#include "uptime.h"
   
#ifndef DEBUG_CGI
#define DEBUG_CGI 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

extern const int k1;
extern const int k2;
   
extern int table_mu[];
extern int table_di[];
extern struct uip_eth_addr mac_addr;
extern char txt_buf[];
extern char html_buf[];
extern device_status_t device_status;

extern const char txt_device_ver_soft[];
extern const char txt_device_ver_hard[];
extern const char txt_device_name[];
extern const char *git_commit_str;

extern const char ERROR_R_ENV[];
extern const char ERROR_W_ENV[];

static volatile uint8_t pin = 0;  // значение вывода
static volatile uint8_t gpio = 0; // значение порта ввода-вывода
//static int i_cfg;
static volatile int mu,di;

//static int mu; // коэффициент Множителя, для расчет соотношения делителя напряжения
//static int di; // коэффициент Делителя, для расчет соотношения делителя напряжения

HTTPD_CGI_CALL(cgi_status_adc, "status_adc", status_adc);
HTTPD_CGI_CALL(cgi_status_i,   "status_i",   status_i);
HTTPD_CGI_CALL(cgi_status_o,   "status_o",   status_o);
HTTPD_CGI_CALL(cgi_set_o,      "set_o",      set_o);
HTTPD_CGI_CALL(cgi_cfg_mac,    "cfg_mac",    cfg_mac);
HTTPD_CGI_CALL(cgi_cfg_get,    "cfg_get",    cfg_get);
HTTPD_CGI_CALL(cgi_cfg_ip,     "cfg_ip",     cfg_ip);
HTTPD_CGI_CALL(cgi_cfg_adc,    "cfg_adc",    cfg_adc);
HTTPD_CGI_CALL(cgi_cfg_i,      "cfg_i",      cfg_i);
HTTPD_CGI_CALL(cgi_cfg_o,      "cfg_o",      cfg_o);
HTTPD_CGI_CALL(cgi_cfg_txt,    "cfg_txt",    cfg_txt);
HTTPD_CGI_CALL(cgi_ver_dev,    "ver_dev",    ver_dev);

static const struct httpd_cgi_call *calls[] = { 
    &cgi_status_adc, 
    &cgi_status_i, 
    &cgi_status_o, 
    &cgi_set_o, 
    &cgi_cfg_mac, 
    &cgi_cfg_get, 
    &cgi_cfg_ip, 
    &cgi_cfg_adc, 
    &cgi_cfg_i, 
    &cgi_cfg_o, 
    &cgi_cfg_txt, 
    &cgi_ver_dev, 
    NULL 
};

uint8_t http_get_parameters_parse(char *par,uint8_t mx)
{
  uint8_t count=0;
	uint8_t i=0;
  for(;par[i]&&i<mx;i++)
  {
    if(par[i]=='=') 
    {
      count++;
      par[i]=0;
    } else if(par[i]=='&')
      par[i]=0;
  }
  return count;
}

char * http_get_parameter_name(char *par,uint8_t cnt,uint8_t mx)
{
  uint8_t i,j;
  cnt*=2;
  for(i=0,j=0;j<mx&&i<cnt;j++)
    if(!par[j]) i++;

  return j==mx?"":par+j;
}

char * http_get_parameter_value(char *par,uint8_t cnt,uint8_t mx)
{
  uint8_t i,j;
  cnt*=2;
  cnt++;
  for(i=0,j=0;j<mx&&i<cnt;j++)
    if(!par[j]) i++;

  return j==mx?"":par+j;
}


/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  printf_dos("\n ERROR: cgi or html: Not Implemented\r\n");
  PSOCK_SEND_STR(&s->sout,"cgi or html: Not Implemented\r\n");
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
  const struct httpd_cgi_call **f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; *f != NULL; ++f) {
    if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
      return (*f)->function;
    }
  }
  return nullfunction;
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(status_adc(struct httpd_state *s, char *ptr))
{
// <th>№</th><th>U вх.АЦП</th><th>* K</th><th>U дел.</th><th style="width:250Px;">Описание</th>
//                              N          U вх          К              U дел        Описание
//    const char s1[]={"\n<tr><td>%d</td><td>%d.%d</td> <td>%d</td> <td>%d.%d</td>  <td>%s</td></tr>\n"};
    const char s1[]={"\n<tr><td>%d</td><td>%s</td>"};   // N U вх
    const char s2[]={"<td>%s</td>"};                    // К
    const char s3[]={"<td>%s</td>"};                    // U дел
    const char s4[]={"<td>%s</td></tr>\n"};             // Описание

    static int i_adc = 0;

    PSOCK_BEGIN(&s->sout);

    memset(html_buf, 0, HTML_BUF_SIZE);

    i_adc=0;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=1;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=2;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=3;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=4;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=5;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=6;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    i_adc=7;
    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, 1, 1);
    xsnprintf( html_buf, HTML_BUF_SIZE, s1, i_adc, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, 1, 1, 1, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s2, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    calc_volt_rdiv(txt_buf, (int)device_status.adc_converted_value[i_adc], k1, k2, table_mu[i_adc], table_di[i_adc]);
    xsnprintf( html_buf, HTML_BUF_SIZE, s3, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    if (env_read_value(R_ADC_0 + i_adc, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
    xsnprintf( html_buf, HTML_BUF_SIZE, s4, txt_buf );
    PSOCK_SEND_STR(&s->sout, html_buf);

    PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(status_i(struct httpd_state *s, char *ptr))
{
  int i = 0;
  const char s1[]={"\n<tr><td>%d</td><td>%s</td><td>%s</td></tr>\n"};
  
  PSOCK_BEGIN(&s->sout);

  memset(html_buf, 0, HTML_BUF_SIZE);

  gpio = device_status.gpio_input; 

  i = 0;
  if (env_read_value(R_INPUT_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 1;
  if (env_read_value(R_INPUT_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 2;
  if (env_read_value(R_INPUT_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 3;
  if (env_read_value(R_INPUT_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 4;
  if (env_read_value(R_INPUT_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 5;
  if (env_read_value(R_INPUT_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 6;
  if (env_read_value(R_INPUT_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i = 7;
  if (env_read_value(R_INPUT_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(status_o(struct httpd_state *s, char *ptr))
{
  int i;
  const char s1[] = {"\n<tr><td>%d</td><td>%s</td><td>%s</td></tr>\n"};

  PSOCK_BEGIN(&s->sout);

  memset(html_buf, 0, HTML_BUF_SIZE);
  gpio = device_status.gpio_output;

  i=0;
  if (env_read_value(R_OUTPUT_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=1;
  if (env_read_value(R_OUTPUT_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=2;
  if (env_read_value(R_OUTPUT_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=3;
  if (env_read_value(R_OUTPUT_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=4;
  if (env_read_value(R_OUTPUT_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=5;
  if (env_read_value(R_OUTPUT_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=6;
  if (env_read_value(R_OUTPUT_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=7;
  if (env_read_value(R_OUTPUT_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, pin ? "1" : "0", txt_buf );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(set_o(struct httpd_state *s, char *ptr))
{
  int i;
  const char s1[] = {"\n<tr><td>%d</td><td>%s</td><td>%s</td><td><input type=\"checkbox\" name=\"o%d\" value=\"1\"  %s></td></tr>\n"};
  uint8_t pcount;
  
  PSOCK_BEGIN(&s->sout);
  
  //check if there are parameters passed 
  if(s->param[0] && (pcount=http_get_parameters_parse(s->param,sizeof(s->param)))>0) {
        gpio = 0;
		//walk through parameters
		for(i=0;i<pcount;i++) {
		    static char *pname,*pval;
		    pname=http_get_parameter_name(s->param,i,sizeof(s->param));
		    pval =http_get_parameter_value(s->param,i,sizeof(s->param));

                    printf_dos("name=%s val=%s\n",pname, pval);

                        if (strcmp(pname, "o0") == 0){
                            gpio |= 1 << 0;
                        }
                        if (strcmp(pname, "o1") == 0){
                            gpio |= 1 << 1;
                        }
                        if (strcmp(pname, "o2") == 0){
                            gpio |= 1 << 2;
                        }
                        if (strcmp(pname, "o3") == 0){
                            gpio |= 1 << 3;
                        }
                        if (strcmp(pname, "o4") == 0){
                            gpio |= 1 << 4;
                        }
                        if (strcmp(pname, "o5") == 0){
                            gpio |= 1 << 5;
                        }
                        if (strcmp(pname, "o6") == 0){
                            gpio |= 1 << 6;
                        }
                        if (strcmp(pname, "o7") == 0){
                            gpio |= 1 << 7;
                        }
                        
                        if (strcmp(pname, "o_clr") == 0){ // сбросс всех значений в 0 (глобальный сброс)
                            gpio = 0;
                            break;
                        }

		}
                device_status.gpio_output = gpio;
                output_gpio_byte(gpio); // вывод в порт новых значений
                printf_dos("\nGPIO = 0x%x\n", gpio);
                printf_dos("\ndevice_status.gpio_output = 0x%x\n", device_status.gpio_output);
  }

  memset(html_buf, 0, HTML_BUF_SIZE);

  gpio = device_status.gpio_output;

  i=0;
  if (env_read_value(R_OUTPUT_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=1;
  if (env_read_value(R_OUTPUT_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=2;
  if (env_read_value(R_OUTPUT_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=3;
  if (env_read_value(R_OUTPUT_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=4;
  if (env_read_value(R_OUTPUT_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i, pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=5;
  if (env_read_value(R_OUTPUT_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i, pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i=6;
  if (env_read_value(R_OUTPUT_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));   

  i=7;
  if (env_read_value(R_OUTPUT_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  pin = gpio & (1<<i);
  xsnprintf( html_buf, HTML_BUF_SIZE, s1, i, \
  pin ? "1" : "0", txt_buf, i,  pin ? "checked" : "");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_mac(struct httpd_state *s, char *ptr))
{
  int i;
  
  PSOCK_BEGIN(&s->sout);

  memset(html_buf, 0, HTML_BUF_SIZE);

  xsnprintf(html_buf, HTML_BUF_SIZE, "<table><tr><th class=\"id_w1\"> MAC:</th><td class=\"id_w5\">");
  for(i=0;i<6;i++) xsnprintf( &html_buf[ strlen(html_buf) ], HTML_BUF_SIZE-strlen(html_buf), "%02X%c", mac_addr.addr[i], i==5 ? ' ' : '-');
  xsnprintf( &html_buf[ strlen(html_buf) ], HTML_BUF_SIZE-strlen(html_buf), "</td></tr></table>\n");
  
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}

/*******************************************************************************
* Convert char char('F') to bin(0x0F)
* 
*******************************************************************************/
static int conv_char_to_bin(char in, uint8_t *out)
{
    if (in>='0' && in<='9'){
       *out = in & 0x0f;
       return 0;
    }

    if (in>='A' && in<='F'){
        *out = in - 0x37;
        return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void conv_get_to_str(char *buf, const size_t buf_size, char *in)
{
    int j;
    uint8_t hi,lo;
    char c;

    memset(buf, 0, buf_size);

    for(j=0; j<strlen(in); j++){
        if (strlen(buf) == buf_size - 1){
            printf_d("WARNING: conv_get_to_str txt_buf == TXT_BUF_SIZE - 1. Overfull input string ?!\n");
            break;
        }
                
        //printf_dos("j = %d\n", j);
        if (*(in+j) == '+'){
            c = ' ';
            //printf_dos("c = %c\n", c);
            buf[strlen((const char*)buf)] = c;
            continue;

            }else if (*(in+j) == '%'){

                conv_char_to_bin( *(in + j + 1), &hi);
                conv_char_to_bin( *(in + j + 2), &lo);

                c = (hi<<4) | lo;
                //printf_dos("c = %c\n", c);
                buf[strlen((const char*)buf)] = c;
                j = j + 2;
                continue;
        }else{ // все остальные символы, в ascii кодировке-латиница

            c = *(in+j);
            //printf_dos("c = %c\n", c);
            buf[strlen((const char*)buf)] = c;
        } // if
    }// for
}


static
PT_THREAD(cfg_get(struct httpd_state *s, char *ptr))
{
  int i;
  uint8_t pcount;

  PSOCK_BEGIN(&s->sout);
  
  //check if there are parameters passed 
  if(s->param[0] && (pcount=http_get_parameters_parse(s->param,sizeof(s->param)))>0) {
    //walk through parameters
    for(i=0;i<pcount;i++) {
      static char *pname,*pval;
      pname=http_get_parameter_name(s->param,i,sizeof(s->param));
      pval =http_get_parameter_value(s->param,i,sizeof(s->param));

      if(DEBUG_CGI) printf_dos("name=%s val=%s\n",pname, pval);
      conv_get_to_str(txt_buf, TXT_BUF_SIZE, pval);
      if(DEBUG_CGI) printf_dos("conv val = %s\n", txt_buf);

      if (strcmp(pname, "txt") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_TXT, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// TXT ------------------------------------------------------------------------------------------

      if (strcmp(pname, "adc_0") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_0, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_0 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_1") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_1, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_1 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_2") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_2, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_2 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_3") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_3, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_3 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_4") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_4, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_4 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_5") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_5, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_5 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_6") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_6, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_6 ----------------------------------------------------------------------------------------
      if (strcmp(pname, "adc_7") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_ADC_7, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_7 ----------------------------------------------------------------------------------------

      if (strcmp(pname, "mul0") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_0, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_0 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul1") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_1, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_1 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul2") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_2, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_2 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul3") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_3, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_3 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul4") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_4, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_4 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul5") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_5, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_5 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul6") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_6, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_6 mul ------------------------------------------------------------------------------------
      if (strcmp(pname, "mul7") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_MUL_7, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_7 mul ------------------------------------------------------------------------------------

      if (strcmp(pname, "div0") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_0, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_0 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div1") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_1, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_1 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div2") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_2, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_2 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div3") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_3, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_3 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div4") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_4, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_4 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div5") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_5, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_5 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div6") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_6, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_6 div ------------------------------------------------------------------------------------
      if (strcmp(pname, "div7") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_DIV_7, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// ADC_7 div ------------------------------------------------------------------------------------



      if (strcmp(pname, "in_0") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_0, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_0 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_1") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_1, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_1 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_2") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_2, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_2 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_3") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_3, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_3 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_4") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_4, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_4 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_5") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_5, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_5 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_6") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_6, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_6 --------------------------------------------------------------------------------------
      if (strcmp(pname, "in_7") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_INPUT_7, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// INPUT_7 --------------------------------------------------------------------------------------
      
      if (strcmp(pname, "out_0") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_0, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_0 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_1") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_1, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_1 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_2") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_2, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_2 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_3") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_3, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_3 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_4") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_4, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_4 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_5") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_5, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_5 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_6") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_6, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_6 -------------------------------------------------------------------------------------
      if (strcmp(pname, "out_7") == 0){ // format %CF%21+%21%21+12345+hello
          if (env_write_value(R_OUTPUT_7, txt_buf) != 0 ) printf_dos(ERROR_W_ENV);
      }// OUTPUT_7 -------------------------------------------------------------------------------------

    }// for





  // Обновляем значения в таблице
  load_i2c_mul_div();

  }// if

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_ip(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if(env_read_value(R_IP_DEVICE, txt_buf)){printf_dos("ERROR: cfg_ip R_IP_DEVICE\n");txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, "<table><tr><th class=\"id_w1\"> IP адрес устройства:</th><td class=\"id_w5\">%s</td></tr></table>\n", txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if(env_read_value(R_IP_MASK, txt_buf)){printf_dos("ERROR: cfg_ip R_IP_MASK\n");txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, "<table><tr><th class=\"id_w1\"> Маска сети:</th><td class=\"id_w5\">%s</td></tr></table>\n", txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if(env_read_value(R_IP_GW, txt_buf)){printf_dos("ERROR: cfg_ip R_IP_GW\n");txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, "<table><tr><th class=\"id_w1\"> IP адрес роутера(gateway):</th><td class=\"id_w5\">%s</td></tr></table>\n", txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_adc(struct httpd_state *s, char *ptr))
{
  const char s1[] = {"<tr><th class=\"id_w3\">АЦП %d</th><form action='/config.shtml' method='GET'><td><input type=\"number\" size=\"5\" min=\"1\" max=\"65535\" name=\"mul%d\" class=\"id_w6\" value=\"%d\"></td>"};
  const char s2[] = {"<td><input type=\"number\" size=\"5\" min=\"1\" max=\"65535\" name=\"div%d\" class=\"id_w6\" value=\"%d\"></td>"};
  const char s20[] = {"<td class=\"id_w3\">%s</td>"};
  const char s3[] = {"<td><input type=\"text\" name=\"adc_%d\" class=\"id_w4\" maxlength=\"30\" value=\"%s\"></td><td><input type=\"submit\" value=\"Сохранить\"></td></form></tr>\n"};

  static int i_cfg;
  
  PSOCK_BEGIN(&s->sout);

  i_cfg = 0;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 1;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 2;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 3;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 4;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 5;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));
  
  i_cfg = 6;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_cfg = 7;
  mu = table_mu[i_cfg];
  di = table_di[i_cfg];
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_cfg, i_cfg, mu);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  xsnprintf(html_buf, HTML_BUF_SIZE, s2, i_cfg, di);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  calc_volt_rdiv(txt_buf, 1, 1, 1, mu, di);
  xsnprintf(html_buf, HTML_BUF_SIZE, s20, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if (env_read_value(R_ADC_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s3, i_cfg, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_i(struct httpd_state *s, char *ptr))
{
  static int i_i;
  const char s1[] = {"<tr><th class=\"id_w3\"> Вх. %d</th><form action='/config.shtml' method='GET'><td><input type=\"text\" name=\"in_%d\" class=\"id_w4\" maxlength=\"30\" value=\"%s\"></td><td><input type=\"submit\" value=\"Сохранить\"></td></form></tr>\n"};

  PSOCK_BEGIN(&s->sout);

  i_i = 0;
  if (env_read_value(R_INPUT_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 1;
  if (env_read_value(R_INPUT_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 2;
  if (env_read_value(R_INPUT_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 3;
  if (env_read_value(R_INPUT_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 4;
  if (env_read_value(R_INPUT_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 5;
  if (env_read_value(R_INPUT_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 6;
  if (env_read_value(R_INPUT_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_i = 7;
  if (env_read_value(R_INPUT_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_i, i_i, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_o(struct httpd_state *s, char *ptr))
{
  static int i_o;
  const char s1[] = {"<tr><th class=\"id_w3\"> Вых. %d</th><form action='/config.shtml' method='GET'><td><input type=\"text\" name=\"out_%d\" class=\"id_w4\" maxlength=\"30\" value=\"%s\"></td><td><input type=\"submit\" value=\"Сохранить\"></td></form></tr>\n"};
 
  PSOCK_BEGIN(&s->sout);

  i_o = 0;
  if (env_read_value(R_OUTPUT_0, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 1;
  if (env_read_value(R_OUTPUT_1, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 2;
  if (env_read_value(R_OUTPUT_2, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 3;
  if (env_read_value(R_OUTPUT_3, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 4;
  if (env_read_value(R_OUTPUT_4, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 5;
  if (env_read_value(R_OUTPUT_5, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 6;
  if (env_read_value(R_OUTPUT_6, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  i_o = 7;
  if (env_read_value(R_OUTPUT_7, txt_buf)){printf_dos(ERROR_R_ENV); txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, s1, i_o, i_o, txt_buf);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cfg_txt(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if(env_read_value(R_TXT, txt_buf)){printf_dos("ERROR: cfg_txt R_TXT\n");txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, "%s", txt_buf);
  
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(ver_dev(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  xsnprintf(html_buf, HTML_BUF_SIZE, "<p>================================================================================<br>\n");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  if(env_read_value(R_TXT, txt_buf)){printf_dos("ERROR: ver_dev R_TXT\n");txt_buf[0] = '\0';}
  xsnprintf(html_buf, HTML_BUF_SIZE, "Устройство: %s <br>  %s     %s<br>RVER:%s<br>", txt_buf, txt_device_ver_soft, txt_device_ver_hard, git_commit_str);
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  uptime_data_t ut; // время работы системы
  uptime(time_get_sec_counter(), &ut);
  xsnprintf(html_buf, HTML_BUF_SIZE, "System uptime: %02d Years, %03d Days, %02d:%02d:%02d<br>", ut.years, ut.days, ut.hours, ut.min, ut.sec );
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));
  xsnprintf(html_buf, HTML_BUF_SIZE, "================================================================================</p><br>\n");
  PSOCK_SEND(&s->sout, html_buf, strlen(html_buf));

  PSOCK_END(&s->sout);
}







/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** @} */
