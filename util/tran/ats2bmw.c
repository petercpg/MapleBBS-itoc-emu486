/*-------------------------------------------------------*/
/* util/transbmw.c                   			 */
/*-------------------------------------------------------*/
/* target : WD 至 Maple 3.02 水球記錄轉換           	 */
/* create : 02/01/22                     		 */
/* update :   /  /                   			 */
/* author : itoc.bbs@bbs.ee.nctu.edu.tw          	 */
/*-------------------------------------------------------*/
/* syntax : transbmw                     		 */
/*-------------------------------------------------------*/


#if 0

   1. 修改 transbmw()

   ps. 使用前請先行備份，use on ur own risk. 程式拙劣請包涵 :p
   ps. 感謝 lkchu 的 Maple 3.02 for FreeBSD

#endif


#include "ats.h"


/* ----------------------------------------------------- */
/* 3.02 functions                    			 */
/* ----------------------------------------------------- */


static void
_mail_self(userid, fpath, owner, title)		/* itoc.011115: 寄檔案給自己 */
  char *userid;		/* 收件者 */
  char *fpath;		/* 檔案路徑 */
  char *owner;		/* 寄件人 */
  char *title;		/* 郵件標題 */
{
  HDR fhdr;
  char folder[64];

  usr_fpath(folder, userid, FN_DIR);
  close(hdr_stamp(folder, HDR_LINK, &fhdr, fpath));
  str_ncpy(fhdr.owner, owner, sizeof(fhdr.owner));
  str_ncpy(fhdr.title, title, sizeof(fhdr.title));
  fhdr.xmode = 0;
  rec_add(folder, &fhdr, sizeof(fhdr));
}


/* ----------------------------------------------------- */
/* 轉換主程式                                            */
/* ----------------------------------------------------- */


static void
transbmw(userid)
  char *userid;
{
  ACCT acct;
  int fd;
  char buf[64],buf2[64],buf3[64];


  /* 開始備份 %-15s 水球記錄＆好友名單＆我的最愛\n */
  printf("\xB6\x7D\xA9\x6C\xB3\xC6\xA5\xF7 %-15s \xA4\xF4\xB2\x79\xB0\x4F\xBF\xFD\xA1\xAE\xA6\x6E\xA4\xCD\xA6\x57\xB3\xE6\xA1\xAE\xA7\xDA\xAA\xBA\xB3\xCC\xB7\x52\n",acct.userid);
  /* sob 的 usr 目錄有分大小寫，所以要先取得大小寫 */
  usr_fpath(buf, userid, FN_ACCT);
  if ((fd = open(buf, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);
  }
  else
  {
    return;
  }

  sprintf(buf, OLD_BBSHOME"/home/%s/writelog", acct.userid);	/* 舊的水球記錄 */

  if (dashf(buf))
    /* [備份] */
    /* 水球記錄 */
    _mail_self(acct.userid, buf, "[\xB3\xC6\xA5\xF7]", "\xA4\xF4\xB2\x79\xB0\x4F\xBF\xFD");

  sprintf(buf2, OLD_BBSHOME"/home/%s/overrides", acct.userid);    /* 舊的好友名單 */

  if (dashf(buf2))
    /* [備份] */
    /* 好友名單 */
    _mail_self(acct.userid, buf2, "[\xB3\xC6\xA5\xF7]", "\xA6\x6E\xA4\xCD\xA6\x57\xB3\xE6");

  sprintf(buf3, OLD_BBSHOME"/home/%s/favor_boards", acct.userid);  /* 舊的我的最愛 */

  if (dashf(buf3))
    /* [備份] */
    /* 我的最愛 */
    _mail_self(acct.userid, buf3, "[\xB3\xC6\xA5\xF7]", "\xA7\xDA\xAA\xBA\xB3\xCC\xB7\x52");
}


int  
main(argc, argv)
  int argc;
  char *argv[];
{
  char c;
  char buf[64];
  struct dirent *de;
  DIR *dirp;

  /* argc == 1 轉全部使用者 */
  /* argc == 2 轉某特定使用者 */

  if (argc > 2)
  {
    printf("Usage: %s [target_user]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (argc == 2)
  {
    transbmw(argv[1]);
    exit(1);
  }

  /* 轉換使用者水球記錄 */
  for (c = 'a'; c <= 'z'; c++)
  {
    sprintf(buf, "usr/%c", c);

    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      transbmw(str);
    }

    closedir(dirp);    
  }
  return 0;
}
