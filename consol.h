/*
 * consol.h
 *
 *  Created on: 29.10.2010
 *      Author: Администратор
 */

#ifndef CONSOL_H_
#define CONSOL_H_


#define KEY_ENTER 		(0x0d)
#define KEY_BACKSPACE 		(0x08)

typedef enum{
	GET_CHAR,
	OUT_PROMT,
	OUT_ERROR,
	CMD_ANALYS_LOW,
	CMD_ANALYS_SET,
	WRITE_CFG
}consol_state_e;



const char promt[] = "DEVICE>";
const char error[] = "\n\rERROR\n\r";



const char cmd_help1[] 	        = "help";
const char cmd_help2[] 	        = "?";
const char cmd_view[] 	        = "view";
const char cmd_sn[]	 	= "sn";
const char cmd_txt[]	 	= "txt";
const char cmd_ipdevice[]	= "ipdevice";
const char cmd_ipgw[]	    = "ipgw";
const char cmd_ipmask[]	        = "ipmask";
const char cmd_version[]	= "version";
const char cmd_root[]		= "root";	// print help command for ROOT
const char cmd_reboot[]		= "reboot";	// reboot sigma

const char STR_CIFRA[] = "0123456789";

const char txt_help[]= {
"\n\r----------------------------------------\n\r"
"------------------ MENU ----------------\n\r"
"help or ?   - This help.\n\r"
"view        - Views all variables.\n\r"
"version     - View version hard+soft.\n\r"
"ipdevice    - Set IP this device.\n\r"
"ipgw        - Set IP default gateway.\n\r"
"ipmask      - Set IP MASK.\n\r"
"udpport     - Set UDP port.\n\r"
};

const char txt_root_help[]= {
"\n---root command--------------------------\n\r"
"sn          - Set serial numer.\n\r"
"txt         - Set text.\n\r"
"reboot      - reboot device.\n\r"
};


#endif /* CONSOL_H_ */
