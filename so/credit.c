/*-------------------------------------------------------*/
/* credit.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 記帳本，記錄生活中的收入支出		 */
/* create : 99/12/18                                     */
/* update : 02/01/26					 */
/* author : wildcat@wd.twbbs.org			 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_CREDIT

/* ----------------------------------------------------- */
/* credit.c 中運用的資料結構                             */
/* ----------------------------------------------------- */

typedef struct
{
  int year;			/* 年 */
  char month;			/* 月 */
  char day;			/* 日 */

  char flag;			/* 支出/收入 */
  int money;			/* 金額 */
  char useway;			/* 類別(食衣住行育樂) */
  char desc[112];		/* 說明 */		/* 這太長了，保留給其他欄位使用 */
}      CREDIT;


#define CREDIT_OUT	0x1	/* 支出 */
#define CREDIT_IN	0x2	/* 收入 */

#define CREDIT_OTHER	0	/* 其他 */
#define CREDIT_EAT	1	/* 食 */
#define CREDIT_WEAR	2	/* 衣 */
#define CREDIT_LIVE	3	/* 住 */
#define CREDIT_MOVE	4	/* 行 */
#define CREDIT_EDU	5	/* 育 */
#define CREDIT_PLAY	6	/* 樂 */

static char fpath[64];		/* FN_CREDIT 檔案路徑 */


static void
credit_head()
{
  /* 記帳手札 */
  vs_head("\xB0\x4F\xB1\x62\xA4\xE2\xA5\xBE", str_site);
  prints(NECKER_CREDIT, d_cols, "");
}


static void
credit_body(page)
  int page;
{
  CREDIT credit;
  /* 其他 */
  /* [食] */
  /* [衣] */
  /* [住] */
  /* [行] */
  /* [育] */
  /* [樂] */
  char *way[] = {"\xA8\xE4\xA5\x4C", "[\xAD\xB9]", "[\xA6\xE7]", "[\xA6\xED]", "[\xA6\xE6]", "[\xA8\x7C]", "[\xBC\xD6]"};
  int fd;

  move(1, 65);
  /* 第 %2d 頁 */
  prints("\xB2\xC4 %2d \xAD\xB6", page + 1);

  move(3, 0);
  clrtobot();

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    int pos, n;

    pos = page * XO_TALL;	/* 一頁有 XO_TALL 筆 */
    n = XO_TALL;

    while (n)
    {
      lseek(fd, (off_t) (sizeof(CREDIT) * pos), SEEK_SET);
      if (read(fd, &credit, sizeof(CREDIT)) == sizeof(CREDIT))
      {
	n--;
	pos++;
	prints("%6d %04d/%02d/%02d %s %8d %4s %.*s\n", 
	  pos, credit.year, credit.month, credit.day, 
	  /* \033[1;32m支出\033[m */
	  /* \033[1;31m收入\033[m */
	  credit.flag == CREDIT_OUT ? "\033[1;32m\xA4\xE4\xA5\x58\033[m" : "\033[1;31m\xA6\xAC\xA4\x4A\033[m",
	  credit.money, 
	  credit.flag == CREDIT_OUT ? way[credit.useway] : "    ",
	  d_cols + 46, credit.desc);
      }
      else
      {
        break;
      }
    }

    close(fd);
  }
}


static int
credit_add()
{
  CREDIT credit;
  char buf[80];

  move(3, 0);
  clrtobot();

  memset(&credit, 0, sizeof(CREDIT));

  /* 收支 (1)收入 (2)支出 [2]  */
  if (vget(5, 0, "\xA6\xAC\xA4\xE4 (1)\xA6\xAC\xA4\x4A (2)\xA4\xE4\xA5\x58 [2] ", buf, 3, DOECHO) == '1')
    credit.flag = CREDIT_IN;
  else
    credit.flag = CREDIT_OUT;
    
  /* 時間 (年份)  */
  vget(6, 0, "\xAE\xC9\xB6\xA1 (\xA6\x7E\xA5\xF7) ", buf, 5, DOECHO);
  credit.year = atoi(buf);

  /* 時間 (月份)  */
  vget(7, 0, "\xAE\xC9\xB6\xA1 (\xA4\xEB\xA5\xF7) ", buf, 3, DOECHO);
  credit.month = atoi(buf);

  /* 時間 (日期)  */
  vget(8, 0, "\xAE\xC9\xB6\xA1 (\xA4\xE9\xB4\xC1) ", buf, 3, DOECHO);
  credit.day = atoi(buf);

  /* 金錢 (元)  */
  vget(9, 0, "\xAA\xF7\xBF\xFA (\xA4\xB8) ", buf, 9, DOECHO);
  credit.money = atoi(buf);

  if (credit.flag == CREDIT_OUT)	/* 支出才有記錄用途 */
  {
    int useway;

    /* 用途 0)其他 1)食 2)衣 3)住 4)行 5)育 6)樂 [0]  */
    useway = vget(10, 0, "\xA5\xCE\xB3\x7E 0)\xA8\xE4\xA5\x4C 1)\xAD\xB9 2)\xA6\xE7 3)\xA6\xED 4)\xA6\xE6 5)\xA8\x7C 6)\xBC\xD6 [0] ", buf, 3, DOECHO) - '0';
    if (useway > 6 || useway < 0)
      useway = 0;
    credit.useway = useway;
  }

  /* 說明： */
  vget(11, 0, "\xBB\xA1\xA9\xFA\xA1\x47", credit.desc, 51, DOECHO);

  rec_add(fpath, &credit, sizeof(CREDIT));
  return 1;
}


static int
credit_delete()
{
  int pos;
  char buf[4];

  /* 要刪除第幾筆資料： */
  vget(b_lines, 0, "\xAD\x6E\xA7\x52\xB0\xA3\xB2\xC4\xB4\x58\xB5\xA7\xB8\xEA\xAE\xC6\xA1\x47", buf, 4, DOECHO);
  pos = atoi(buf);
  
  if (rec_num(fpath, sizeof(CREDIT)) < pos)
  {
    /* 您搞錯囉，沒有這筆資料 */
    vmsg("\xB1\x7A\xB7\x64\xBF\xF9\xC5\x6F\xA1\x41\xA8\x53\xA6\xB3\xB3\x6F\xB5\xA7\xB8\xEA\xAE\xC6");
    return 0;
  }

  rec_del(fpath, sizeof(CREDIT), pos - 1, NULL);
  return 1;
}


static int
credit_count()
{
  CREDIT *credit;
  struct stat st;
  int fd;
  int way[7], moneyin, moneyout;

  if ((fd = open(fpath, O_RDONLY)) >= 0 && !fstat(fd, &st) && st.st_size > 0)
  {
    memset(way, 0, sizeof(way));
    moneyin = 0;

    mgets(-1);
    while (credit = mread(fd, sizeof(CREDIT)))
    {
      if (credit->flag == CREDIT_OUT)	/* 支出才有記錄用途 */
	way[credit->useway] += credit->money;
      else
        moneyin += credit->money;
    }
    close(fd);

    moneyout = 0;
    for (fd = 0; fd <= 6; fd++)
      moneyout += way[fd];

    move(3, 0);
    clrtobot();

    move(7, 0);
    /*       \033[1;31m總收入  %12d 元\033[m\n */
    prints("      \033[1;31m\xC1\x60\xA6\xAC\xA4\x4A  %12d \xA4\xB8\033[m\n", moneyin);
    /*       \033[1;32m總支出  %12d 元\033[m\n\n */
    prints("      \033[1;32m\xC1\x60\xA4\xE4\xA5\x58  %12d \xA4\xB8\033[m\n\n", moneyout);

    /* 花在  \033[1;36m [食]   %12d 元    \033[32m [衣]   %12d 元\033[m\n */
    prints("\xAA\xE1\xA6\x62  \033[1;36m [\xAD\xB9]   %12d \xA4\xB8    \033[32m [\xA6\xE7]   %12d \xA4\xB8\033[m\n", way[CREDIT_EAT], way[CREDIT_WEAR]);
    /*       \033[1;31m [住]   %12d 元    \033[33m [行]   %12d 元\033[m\n */
    prints("      \033[1;31m [\xA6\xED]   %12d \xA4\xB8    \033[33m [\xA6\xE6]   %12d \xA4\xB8\033[m\n", way[CREDIT_LIVE], way[CREDIT_MOVE]);
    /*       \033[1;35m [育]   %12d 元    \033[37m [樂]   %12d 元\033[m\n */
    prints("      \033[1;35m [\xA8\x7C]   %12d \xA4\xB8    \033[37m [\xBC\xD6]   %12d \xA4\xB8\033[m\n", way[CREDIT_EDU], way[CREDIT_PLAY]);
    /*       \033[1;34m 其他   %12d 元\033[m */
    prints("      \033[1;34m \xA8\xE4\xA5\x4C   %12d \xA4\xB8\033[m", way[CREDIT_OTHER]);

    vmsg(NULL);
    return 1;
  }

  /* 您沒有記帳記錄 */
  vmsg("\xB1\x7A\xA8\x53\xA6\xB3\xB0\x4F\xB1\x62\xB0\x4F\xBF\xFD");
  return 0;
}


int
main_credit()
{
  int page, redraw;
  char buf[3];

  credit_head();

  usr_fpath(fpath, cuser.userid, FN_CREDIT);
  page = 0;
  redraw = 1;

  for (;;)
  {
    if (redraw)
      credit_body(page);

    /* 記帳手札 C)換頁 1)新增 2)刪除 3)全刪 4)總計 Q)離開 [Q]  */
    switch (vans("\xB0\x4F\xB1\x62\xA4\xE2\xA5\xBE C)\xB4\xAB\xAD\xB6 1)\xB7\x73\xBC\x57 2)\xA7\x52\xB0\xA3 3)\xA5\xFE\xA7\x52 4)\xC1\x60\xAD\x70 Q)\xC2\xF7\xB6\x7D [Q] "))
    {
    case 'c':
      /* 跳到第幾頁： */
      vget(b_lines, 0, "\xB8\xF5\xA8\xEC\xB2\xC4\xB4\x58\xAD\xB6\xA1\x47", buf, 3, DOECHO);
      redraw = atoi(buf) - 1;
      
      if (page != redraw && redraw >= 0 && 
        redraw <= (rec_num(fpath, sizeof(CREDIT)) - 1) / XO_TALL)
      {
        page = redraw;
        redraw = 1;
      }
      else
      {
        redraw = 0;
      }
      break;

    case '1':
      redraw = credit_add();
      break;

    case '2':
      redraw = credit_delete();
      break;

    case '3':
      if (vans(MSG_SURE_NY) == 'y')
      {
        unlink(fpath);
        return 0;
      }
      break;

    case '4':
      redraw = credit_count();
      break;

    default:
      return 0;
    }
  }
}
#endif				/* HAVE_CREDIT */
