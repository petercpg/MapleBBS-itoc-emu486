/*-------------------------------------------------------*/
/* util/showACCT.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 顯示使用者資料				 */
/* create : 01/07/16					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#undef	SHOW_PASSWORD		/* 顯示密碼 (很耗時間) */


#ifdef SHOW_PASSWORD

#if 0
#  define GUESS_LEN	3	/* 測三碼(含)以下的所有密碼組合 (最多是 PSWDLEN) */
#  define GUESS_START	' '
#  define GUESS_END	0x7f
#else
#  define GUESS_LEN	6	/* 測六碼(含)以下的數字密碼組合 (最多是 PSWDLEN) */
#  define GUESS_START	'0'
#  define GUESS_END	'9'
#endif


static inline void
showpasswd(passwd)
  char *passwd;
{

  int i, index;
  char guess[PSWDLEN + 1];

  /* 無論是什麼 encrypt 的方法，都硬從 GUESS_START 開始 try 到 GUESS_END */

  memset(guess, 0, sizeof(guess));
  index = 0;
  guess[index] = GUESS_START;

  while (index < GUESS_LEN)
  {
    if (!chkpasswd(passwd, guess))
    {
      /* 密碼: %s \n */
      printf("\xB1\x4B\xBD\x58: %s \n", guess);
      return;
    }

    for (i = index; i >= 0; i--)
    {
      if (guess[i] < GUESS_END)
      {
	guess[i]++;
	break;
      }
      else     /* 進位 */
      {
	guess[i] = GUESS_START;
      }
    }

    if (i < 0)	/* 所有 index 位都試完了 */
    {
      index++;
      guess[index] = GUESS_START;
      /* 掃描密碼中，請稍待...（已完成 %d 位密碼之掃描）\n */
      printf("\xB1\xBD\xB4\x79\xB1\x4B\xBD\x58\xA4\xA4\xA1\x41\xBD\xD0\xB5\x79\xAB\xDD...\xA1\x5D\xA4\x77\xA7\xB9\xA6\xA8 %d \xA6\xEC\xB1\x4B\xBD\x58\xA4\xA7\xB1\xBD\xB4\x79\xA1\x5E\n", index);
    }
  }
  /* 密碼長度超過 %d 位\n */
  printf("\xB1\x4B\xBD\x58\xAA\xF8\xAB\xD7\xB6\x57\xB9\x4C %d \xA6\xEC\n", GUESS_LEN);
}
#endif


static char *
_bitmsg(str, level)
  char *str;
  int level;
{
  int cc;
  int num;
  static char msg[33];

  num = 0;
  while (cc = *str)
  {
    msg[num] = (level & 1) ? cc : '-';
    level >>= 1;
    str++;
    num++;
  }
  msg[num] = 0;

  return msg;
}


static inline void
showACCT(acct)
  ACCT *acct;
{
  char msg1[40], msg2[40], msg3[40], msg4[40], msg5[40], msg6[40];

  strcpy(msg1, _bitmsg(STR_PERM, acct->userlevel));
  strcpy(msg2, _bitmsg(STR_UFO, acct->ufo));
  strcpy(msg3, Btime((acct->firstlogin)));
  strcpy(msg4, Btime((acct->lastlogin)));
  strcpy(msg5, Btime((acct->tcheck)));
  strcpy(msg6, Btime((acct->tvalid)));

  printf("> ------------------------------------------------------------------------------------------ \n"
    /* 編號: %-15d [ID]: %-15s 姓名: %-15s 暱稱: %-15s \n */
    "\xBD\x73\xB8\xB9: %-15d [ID]: %-15s \xA9\x6D\xA6\x57: %-15s \xBC\xCA\xBA\xD9: %-15s \n" 
    /* 權限: %-37s 設定: %-37s \n */
    "\xC5\x76\xAD\xAD: %-37s \xB3\x5D\xA9\x77: %-37s \n" 
    /* 簽名: %-37d 性別: %-15.2s \n */
    "\xC3\xB1\xA6\x57: %-37d \xA9\xCA\xA7\x4F: %-15.2s \n" 
    /* 銀幣: %-15d 金幣: %-15d 生日: %02d/%02d/%02d \n */
    "\xBB\xC8\xB9\xF4: %-15d \xAA\xF7\xB9\xF4: %-15d \xA5\xCD\xA4\xE9: %02d/%02d/%02d \n" 
    /* 上站: %-15d 文章: %-15d 發信: %-15d \n */
    "\xA4\x57\xAF\xB8: %-15d \xA4\xE5\xB3\xB9: %-15d \xB5\x6F\xAB\x48: %-15d \n" 
    /* 首次: %-37s 上次: %-30s \n */
    "\xAD\xBA\xA6\xB8: %-37s \xA4\x57\xA6\xB8: %-30s \n" 
    /* 檢查: %-37s 通過: %-30s \n */
    "\xC0\xCB\xAC\x64: %-37s \xB3\x71\xB9\x4C: %-30s \n" 
    /* 登入: %-30s \n */
    "\xB5\x6E\xA4\x4A: %-30s \n" 
    /* 信箱: %-60s \n */
    "\xAB\x48\xBD\x63: %-60s \n", 
    acct->userno, acct->userid, acct->realname, acct->username, 
    msg1, msg2,
    /* ？♂♀ */
    acct->signature, "\xA1\x48\xA1\xF1\xA1\xF0" + (acct->sex << 1),
    acct->money, acct->gold, acct->year, acct->month, acct->day, 
    acct->numlogins, acct->numposts, acct->numemails, 
    msg3, msg4, 
    msg5, msg6, 
    acct->lasthost, 
    acct->email);

#ifdef SHOW_PASSWORD
  showpasswd(acct->passwd);
#endif
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int i;

  if (argc < 2)
  {
    printf("Usage: %s UserID1 [UserID2] ...\n", argv[0]);
    return -1;
  }

  chdir(BBSHOME);

  for (i = 1; i < argc; i++)
  {
    ACCT acct;
    char fpath[64];

    usr_fpath(fpath, argv[i], FN_ACCT);
    if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", argv[i]);
      continue;
    }

    showACCT(&acct);
  }
  printf("> ------------------------------------------------------------------------------------------ \n");

  return 0;
}
