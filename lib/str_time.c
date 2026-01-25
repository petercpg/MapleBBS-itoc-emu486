#include <stdio.h>
#include <time.h>
#include "config.h"


static char datemsg[40];


char *
Atime(clock)    /* Thor.990125: \xbc\xbc\xbc\xcb ARPANET \xae\xc9\xb6\xa1\xb1\xec\xa6\xa1 */
  time32_t clock;
{
  time_t t = clock;
  /* ARPANET format: Thu, 11 Feb 1999 06:00:37 +0800 (CST) */
  /* strftime(datemsg, 40, "%a, %d %b %Y %T %Z", localtime(clock)); */
  /* Thor.990125: time zone\xaa\xba\xb0\xec\xb6\xc1\xad\xdd\xa4\xa3\xa4\x40,ARPANET\xb1\xec\xa6\xa1\xac\x4f\xa4\x40\xad\xdd\xad\x6e,\xab\xed\xab\xed\xb5\xb9,\xa6\x50sendmail*/
  strftime(datemsg, 40, "%a, %d %b %Y %T +0800 (CST)", localtime(&t));
  return (datemsg);
}


char *
Btime(clock)    /* BBS \xae\xc9\xb6\xa1\xb1\xec\xa6\xa1 */
  time32_t clock;
{
  time_t t_clock = clock;
  struct tm *t = localtime(&t_clock);

  sprintf(datemsg, "%d/%02d/%02d %.3s %02d:%02d:%02d",
    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
    "SunMonTueWedThuFriSat" + (t->tm_wday * 3),
    t->tm_hour, t->tm_min, t->tm_sec);
  return (datemsg);
}


char *
Now()
{
  time_t now;

  time(&now);
  return Btime((time32_t)now);
}
