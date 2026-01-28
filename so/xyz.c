/*-------------------------------------------------------*/
/* so/xyz.c             ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 雜七雜八的外掛				 */
/* create : 01/03/01					 */
/* update :   /  /  					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_TIP

/* ----------------------------------------------------- */
/* 每日小秘訣						 */
/* ----------------------------------------------------- */

int
x_tip()
{
  int i, j;
  char msg[128];
  FILE *fp;

  if (!(fp = fopen(FN_ETC_TIP, "r")))
    return XEASY;

  fgets(msg, 128, fp);
  j = atoi(msg);		/* 第一行記錄總篇數 */
  i = time(0) % j + 1;
  j = 0;

  while (j < i)			/* 取第 i 個 tip */
  {
    fgets(msg, 128, fp);
    if (msg[0] == '#')
      j++;
  }

  move(12, 0);
  clrtobot();
  fgets(msg, 128, fp);
  /* \033[1;36m每日小祕訣：\033[m\n */
  prints("\033[1;36m\xA8\x43\xA4\xE9\xA4\x70\xAF\xA6\xB3\x5A\xA1\x47\033[m\n");
  prints("            %s", msg);
  fgets(msg, 128, fp);
  prints("            %s", msg);
  vmsg(NULL);
  fclose(fp);
  return 0;
}
#endif	/* HAVE_TIP */


#ifdef HAVE_LOVELETTER 

/* ----------------------------------------------------- */
/* 情書產生器						 */
/* ----------------------------------------------------- */

int
x_loveletter()
{
  FILE *fp;
  int start_show;	/* 1:開始秀 */
  int style;		/* 0:開頭 1:正文 2:結尾 */
  int line;
  char buf[128];
  char header[3][5] = {"head", "body", "foot"};	/* 開頭、正文、結尾 */
  int num[3];

  /* etc/loveletter 前段是#head 中段是#body 後段是#foot */
  /* 行數上限：#head五行  #body八行  #foot五行 */

  if (!(fp = fopen(FN_ETC_LOVELETTER, "r")))
    return XEASY;

  /* 前三行記錄篇數 */
  fgets(buf, 128, fp);
  num[0] = atoi(buf + 5);
  num[1] = atoi(buf + 5);
  num[2] = atoi(buf + 5);

  /* 決定要選第幾篇 */
  line = time(0);
  num[0] = line % num[0];
  num[1] = (line >> 1) % num[1];
  num[2] = (line >> 2) % num[2];

  /* 情書產生器 */
  vs_bar("\xB1\xA1\xAE\xD1\xB2\xA3\xA5\xCD\xBE\xB9");

  start_show = style = line = 0;

  while (fgets(buf, 128, fp))
  {
    if (*buf == '#')
    {
      if (!strncmp(buf + 1, header[style], 4))  /* header[] 長度都是 5 bytes */
	num[style]--;

      if (num[style] < 0)	/* 已經 fget 到要選的這篇了 */
      {
	outc('\n');
	start_show = 1;
	style++;
      }
      else
      {
	start_show = 0;
      }
      continue;
    }

    if (start_show)
    {
      if (line >= (b_lines - 5))	/* 超過螢幕大小了 */
	break;

      outs(buf);
      line++;
    }
  }

  fclose(fp);
  vmsg(NULL);

  return 0;
}
#endif	/* HAVE_LOVELETTER */


/* ----------------------------------------------------- */
/* 密碼忘記，重設密碼					 */
/* ----------------------------------------------------- */


int
x_password()
{
  int i;
  ACCT acct;
  FILE *fp;
  char fpath[80], email[60], passwd[PSWDLEN + 1];
  time_t now;

  /* 當其他使用者忘記密碼時，重送新密碼至該使用者的信箱 */
  vmsg("\xB7\xED\xA8\xE4\xA5\x4C\xA8\xCF\xA5\xCE\xAA\xCC\xA7\xD1\xB0\x4F\xB1\x4B\xBD\x58\xAE\xC9\xA1\x41\xAD\xAB\xB0\x65\xB7\x73\xB1\x4B\xBD\x58\xA6\xDC\xB8\xD3\xA8\xCF\xA5\xCE\xAA\xCC\xAA\xBA\xAB\x48\xBD\x63");

  if (acct_get(msg_uid, &acct) > 0)
  {
    time(&now);

    if (acct.lastlogin > now - 86400 * 10)
    {
      /* 該使用者必須十天以上未上站方可重送密碼 */
      vmsg("\xB8\xD3\xA8\xCF\xA5\xCE\xAA\xCC\xA5\xB2\xB6\xB7\xA4\x51\xA4\xD1\xA5\x48\xA4\x57\xA5\xBC\xA4\x57\xAF\xB8\xA4\xE8\xA5\x69\xAD\xAB\xB0\x65\xB1\x4B\xBD\x58");
      return 0;
    }

    /* 請輸入認證時的 Email： */
    vget(b_lines - 2, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xBB\x7B\xC3\xD2\xAE\xC9\xAA\xBA Email\xA1\x47", email, 40, DOECHO);

    if (str_cmp(acct.email, email))
    {
      /* 這不是該使用者認證時用的 Email */
      vmsg("\xB3\x6F\xA4\xA3\xAC\x4F\xB8\xD3\xA8\xCF\xA5\xCE\xAA\xCC\xBB\x7B\xC3\xD2\xAE\xC9\xA5\xCE\xAA\xBA Email");
      return 0;
    }

    if (not_addr(email) || !mail_external(email))
    {
      vmsg(err_email);
      return 0;
    }

    /* 請輸入真實姓名： */
    vget(b_lines - 1, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57\xA1\x47", fpath, RNLEN + 1, DOECHO);
    if (strcmp(acct.realname, fpath))
    {
      /* 這不是該使用者的真實姓名 */
      vmsg("\xB3\x6F\xA4\xA3\xAC\x4F\xB8\xD3\xA8\xCF\xA5\xCE\xAA\xCC\xAA\xBA\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57");
      return 0;
    }

    /* 資料正確，請確認是否產生新密碼(Y/N)？[N]  */
    if (vans("\xB8\xEA\xAE\xC6\xA5\xBF\xBD\x54\xA1\x41\xBD\xD0\xBD\x54\xBB\x7B\xAC\x4F\xA7\x5F\xB2\xA3\xA5\xCD\xB7\x73\xB1\x4B\xBD\x58(Y/N)\xA1\x48[N] ") != 'y')
      return 0;

    /* %s 改了 %s 的密碼 */
    sprintf(fpath, "%s \xA7\xEF\xA4\x46 %s \xAA\xBA\xB1\x4B\xBD\x58", cuser.userid, acct.userid);
    blog("PASSWD", fpath);

    /* 亂數產生 A~Z 組合的密碼八碼 */
    for (i = 0; i < PSWDLEN; i++)
      passwd[i] = rnd(26) + 'A';
    passwd[PSWDLEN] = '\0';

    /* 重新 acct_load 載入一次，避免對方在 vans() 時登入會有洗錢的效果 */
    if (acct_load(&acct, acct.userid) >= 0)
    {
      str_ncpy(acct.passwd, genpasswd(passwd), PASSLEN + 1);
      acct_save(&acct);
    }

    sprintf(fpath, "tmp/sendpass.%s", cuser.userid);
    if (fp = fopen(fpath, "w"))
    {
      /* %s 為您申請了新密碼\n\n */
      fprintf(fp, "%s \xAC\xB0\xB1\x7A\xA5\xD3\xBD\xD0\xA4\x46\xB7\x73\xB1\x4B\xBD\x58\n\n", cuser.userid);
      fprintf(fp, BBSNAME "ID : %s\n\n", acct.userid);
      /* 新密碼 : %s\n */
      fprintf(fp, BBSNAME "\xB7\x73\xB1\x4B\xBD\x58 : %s\n", passwd);
      fclose(fp);

      /* 新密碼 */
      bsmtp(fpath, BBSNAME "\xB7\x73\xB1\x4B\xBD\x58", email, 0);
      unlink(fpath);

      /* 新密碼已寄到該認證信箱 */
      vmsg("\xB7\x73\xB1\x4B\xBD\x58\xA4\x77\xB1\x48\xA8\xEC\xB8\xD3\xBB\x7B\xC3\xD2\xAB\x48\xBD\x63");
    }
  }

  return 0;
}
