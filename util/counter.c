/*-------------------------------------------------------*/
/* util/counter.c       ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 歷史軌跡					 */
/* create : 03/03/03					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#define OUTFILE_COUNTER		"gem/@/@-counter"
#define FN_RUN_COUNTER		"run/var/counter"


typedef struct
{
  time_t uptime;		/* 更新時間 */

  int total_acct;		/* 站上總共有多少使用者註冊 */
  int online_usr;		/* 線上同時有多少使用者 */
  int today_usr;		/* 今日總共有幾個人次上站 */
  int online_usr_every_hour[24];/* 今日各整點線上有多少使用者 */
  int total_brd;		/* 站上總共有多少看板 */

  char ident[12];		/* 留白以擴充用 */
}      COUNTER;


#define break_record(new, old)	(new > old + old / 20)	/* 當新紀錄超過舊紀錄 5% 以上時，就改寫紀錄 */


int
main()
{
  UCACHE *ushm;
  COUNTER counter;

  int num;
  char *fname, date[20];
  FILE *fp;
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);

  if (!(fp = fopen(OUTFILE_COUNTER, "a+")))
    return -1;

  fname = FN_RUN_COUNTER;

  memset(&counter, 0, sizeof(COUNTER));
  rec_get(fname, &counter, sizeof(COUNTER), 0);

  counter.uptime = time(&now);
  ptime = localtime(&now);
  /* 【%02d/%02d/%02d %02d:%02d】 */
  sprintf(date, "\xA1\x69%02d/%02d/%02d %02d:%02d\xA1\x6A", 
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);


  /* 註冊人數 */
  num = rec_num(FN_SCHEMA, sizeof(SCHEMA));
  if (break_record(num, counter.total_acct))
  {
    /* ★ %s \033[31m本站註冊人數\033[m狂賀超過 \033[1;31m%d\033[m 人\n */
    fprintf(fp, "\xA1\xB9 %s \033[31m\xA5\xBB\xAF\xB8\xB5\xF9\xA5\x55\xA4\x48\xBC\xC6\033[m\xA8\x67\xB6\x50\xB6\x57\xB9\x4C \033[1;31m%d\033[m \xA4\x48\n", date, num);
    counter.total_acct = num;
  }

  /* 線上人數 */
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
  num = ushm->count;
  if (break_record(num, counter.online_usr))
  {
    /* ◎ %s \033[32m同時線上人數\033[m首次達到 \033[1;32m%d\033[m 人\n */
    fprintf(fp, "\xA1\xB7 %s \033[32m\xA6\x50\xAE\xC9\xBD\x75\xA4\x57\xA4\x48\xBC\xC6\033[m\xAD\xBA\xA6\xB8\xB9\x46\xA8\xEC \033[1;32m%d\033[m \xA4\x48\n", date, num);
    counter.online_usr = num;
  }
  counter.online_usr_every_hour[ptime->tm_hour] = num;	/* 本小時的線上人數，就拿這次 sample 值 */

  /* 本日上站人次 */
  if (ptime->tm_hour == 23)	/* 每天計算一次本日上站人次 */
  {
    int i;

    /* itoc.註解: 這個值是不正確的，因為只是拿每個小時 sample 的和，
       而且還假設每個人平均在站上時間是 60 分鐘 */
    num = 0;
    for (i = 0; i < 24; i++)
      num += counter.online_usr_every_hour[i];

    if (break_record(num, counter.today_usr))
    {
      /* ◆ %s \033[33m單日上站人次\033[m正式突破 \033[1;33m%d\033[m 人\n */
      fprintf(fp, "\xA1\xBB %s \033[33m\xB3\xE6\xA4\xE9\xA4\x57\xAF\xB8\xA4\x48\xA6\xB8\033[m\xA5\xBF\xA6\xA1\xAC\xF0\xAF\x7D \033[1;33m%d\033[m \xA4\x48\n", date, num);
      counter.today_usr = num;
    }
  }

  /* 看板個數 */
  num = rec_num(FN_BRD, sizeof(BRD));
  if (break_record(num, counter.total_brd))
  {
    /* ☆ %s \033[34m本站看板個數\033[m宣佈高達 \033[1;34m%d\033[m 個\n */
    fprintf(fp, "\xA1\xB8 %s \033[34m\xA5\xBB\xAF\xB8\xAC\xDD\xAA\x4F\xAD\xD3\xBC\xC6\033[m\xAB\xC5\xA7\x47\xB0\xAA\xB9\x46 \033[1;34m%d\033[m \xAD\xD3\n", date, num);
    counter.total_brd = num;
  }


  rec_put(fname, &counter, sizeof(COUNTER), 0, NULL);

  fclose(fp);
  return 0;
}
