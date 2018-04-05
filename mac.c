#ifndef DEBUG
#define DEBUG 0
#endif

void mac_read(unsigned char *mac)
{

    *(mac + 0) = 00;
    *(mac + 1) = 01;
    *(mac + 2) = 02;
    *(mac + 3) = 03;
    *(mac + 4) = 04;
    *(mac + 5) = 05;

}
