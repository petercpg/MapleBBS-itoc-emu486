#include <stdio.h>
#include <time.h>
#include "config.h"


void
str_stamp(str, chrono)
  char *str;
  time32_t chrono;
{
  time_t t_chrono = chrono;
  struct tm *ptime;

  ptime = localtime(&t_chrono);
  sprintf(str, "%02d/%02d/%02d",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
}
