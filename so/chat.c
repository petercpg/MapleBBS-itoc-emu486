/*-------------------------------------------------------*/
/* so/chat.c            ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : chat client for xchatd			 */
/* create : 95/03/29					 */
/* update : 95/12/15					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


static char chatroom[IDLEN];	/* Chat-Room Name */
static int chatline;		/* Where to display message now */
static char chatopic[48];
static FILE *frec;


#define	stop_line	(b_lines - 2)


extern char *bmode();


static void
chat_topic()
{
  move(0, 0);
  /* [1;37;46m %s室：%-14s[45m 話題：%-48s[m */
  prints("[1;37;46m %s\xAB\xC7\xA1\x47%-14s[45m \xB8\xDC\xC3\x44\xA1\x47%-48s[m",
    /* 錄音 */
    /* 談天 */
    (frec ? "\xBF\xFD\xAD\xB5" : "\xBD\xCD\xA4\xD1"), chatroom, chatopic);
}


static void
printchatline(msg)
  char *msg;
{
  int line;

  line = chatline;
  move(line, 0);
  outs(msg);
  outc('\n');

  if (frec)
    fprintf(frec, "%s\n", msg);

  if (++line == stop_line)
    line = 2;
  move(line, 0);
  /* → */
  outs("\xA1\xF7");
  clrtoeol();
  chatline = line;
}


static void
chat_record()
{
  FILE *fp;
  time_t now;
  char buf[80];

  time(&now);

  if (fp = frec)
  {
    /* %s\n結束：%s\n */
    fprintf(fp, "%s\n\xB5\xB2\xA7\xF4\xA1\x47%s\n", msg_seperator, Btime(now));
    fclose(fp);
    frec = NULL;
    /* ◆ 錄音完畢！ */
    printchatline("\xA1\xBB \xBF\xFD\xAD\xB5\xA7\xB9\xB2\xA6\xA1\x49");
  }
  else
  {
#ifdef EVERY_Z
    /* Thor.980602: 由於 tbf_ask() 需問檔名，此時會用到 igetch()，
       為了防止 I_OTHERDATA 造成當住，在此用 every_Z() 的方式，
       先保存 vio_fd，待問完後再還原 */

    vio_save();		/* Thor.980602: 暫存 vio_fd */
#endif

    usr_fpath(buf, cuser.userid, tbf_ask());

#ifdef EVERY_Z
    vio_restore();	/* Thor.980602: 還原 vio_fd */
#endif

    move(b_lines, 0);
    clrtoeol();

    fp = fopen(buf, "a");
    if (fp)
    {
      /* 主題: %s\n包廂: %s\n錄音: %s (%s)\n開始: %s\n%s\n */
      fprintf(fp, "\xA5\x44\xC3\x44: %s\n\xA5\x5D\xB4\x5B: %s\n\xBF\xFD\xAD\xB5: %s (%s)\n\xB6\x7D\xA9\x6C: %s\n%s\n",
	chatopic, chatroom, cuser.userid, cuser.username,
	Btime(now), msg_seperator);
      /* ◆ 開始錄音囉！ */
      printchatline("\xA1\xBB \xB6\x7D\xA9\x6C\xBF\xFD\xAD\xB5\xC5\x6F\xA1\x49");
      frec = fp;
    }
    else
    {
      /* ◆ 錄音機故障了，請通知站長維修 */
      printchatline("\xA1\xBB \xBF\xFD\xAD\xB5\xBE\xF7\xAC\x47\xBB\xD9\xA4\x46\xA1\x41\xBD\xD0\xB3\x71\xAA\xBE\xAF\xB8\xAA\xF8\xBA\xFB\xAD\xD7");
    }
  }
  bell();
  chat_topic();
}


static void
chat_clear()
{
  int line;

  for (line = 2; line < stop_line; line++)
  {
    move(line, 0);
    clrtoeol();
  }
  chatline = stop_line - 1;
  printchatline("");
}


static void
print_chatid(chatid)
  char *chatid;
{
  move(b_lines - 1, 0);
  outs(chatid);
  outc(':');
}


static inline int
chat_send(fd, buf)
  int fd;
  char *buf;
{
  int len;

  len = strlen(buf);
  return (send(fd, buf, len, 0) == len);
}


static inline int
chat_recv(fd, chatid)
  int fd;
  char *chatid;
{
  static char buf[512];
  static int bufstart = 0;
  int cc, len;
  char *bptr, *str;

  bptr = buf;
  cc = bufstart;
  len = sizeof(buf) - cc - 1;
  if ((len = recv(fd, bptr + cc, len, 0)) <= 0)
    return -1;
  cc += len;

  for (;;)
  {
    len = strlen(bptr);

    if (len >= cc)
    {				/* wait for trailing data */
      memcpy(buf, bptr, len);
      bufstart = len;
      break;
    }
    if (*bptr == '/')
    {
      str = bptr + 1;
      fd = *str++;

      if (fd == 'c')
      {
	chat_clear();
      }
      else if (fd == 'n')
      {
	str_ncpy(chatid, str, 9);
         
	/* Thor.980819: 順便換一下 mateid 好了... */
	str_ncpy(cutmp->mateid, str, sizeof(cutmp->mateid));

	print_chatid(chatid);
	clrtoeol();
      }
      else if (fd == 'r')
      {
	str_ncpy(chatroom, str, sizeof(chatroom));
	chat_topic();
      }
      else if (fd == 't')
      {
	str_ncpy(chatopic, str, sizeof(chatopic));
	chat_topic();
      }
    }
    else
    {
      printchatline(bptr);
    }

    cc -= ++len;
    if (cc <= 0)
    {
      bufstart = 0;
      break;
    }
    bptr += len;
  }

  return 0;
}


static void
chat_pager(arg)
  char *arg;
{
  cuser.ufo ^= UFO_PAGER;
  cutmp->ufo ^= UFO_PAGER;
  /* Thor.980805: 解決ufo 同步問題 */

  /* ◆ 您的呼叫器已經%s了! */
  sprintf(arg, "\xA1\xBB \xB1\x7A\xAA\xBA\xA9\x49\xA5\x73\xBE\xB9\xA4\x77\xB8\x67%s\xA4\x46!",
    /* 關閉 */
    /* 打開 */
    cuser.ufo & UFO_PAGER ? "\xC3\xF6\xB3\xAC" : "\xA5\xB4\xB6\x7D");
  printchatline(arg);
}


#if 0
/* Thor.980727: 和 /flag 衝key */
static void
chat_write(arg)
  char *arg;
{
  int uno;
  UTMP *up;
  char *str;
  CallMsg cmsg;

  strtok(arg, STR_SPACE);
  if ((str = strtok(NULL, STR_SPACE)) && (uno = acct_userno(str)) > 0)
  {
    cmsg.recver = uno;		/* 先記下 userno 作為 check */
    if (up = utmp_find(uno))
    {
      if (can_override(up))
      {
	if (str = strtok(NULL, "\n"))	/* Thor.980725:抓整句話 */
	{			/* Thor.980724: 從 my_write 改過來 */
	  int len;
	  char buf[80];
	  extern char fpmsg[];
	  /* Thor.980722: msg file加上自己說的話 */

	  sprintf(fpmsg + 4, "%s-", cuser.userid);
	  /* Thor.980722: 借用 len當一下fd :p */
	  len = open(fpmsg, O_WRONLY | O_CREAT | O_APPEND, 0600);
	  /* 給%s：%s\n */
	  sprintf(buf, "\xB5\xB9%s\xA1\x47%s\n", up->userid, str);
	  write(len, buf, strlen(buf));
	  close(len);

	  sprintf(buf, "%s(%s", cuser.userid, cuser.username);
	  len = strlen(str);
	  buf[71 - len] = '\0';
	  /* \033[1;33;46m★ %s) \033[37;45m %s \033[m */
	  sprintf(cmsg.msg, "\033[1;33;46m\xA1\xB9 %s) \033[37;45m %s \033[m", buf, str);

	  cmsg.caller = cutmp;
	  cmsg.sender = cuser.userno;

	  if (do_write(up, &cmsg))
	    /* ◆ 對方已經離去 */
	    printchatline("\xA1\xBB \xB9\xEF\xA4\xE8\xA4\x77\xB8\x67\xC2\xF7\xA5\x68");
	}
	else
	{
	  /* ◆ 別只眨眼，說些話吧！ */
	  printchatline("\xA1\xBB \xA7\x4F\xA5\x75\xAF\x77\xB2\xB4\xA1\x41\xBB\xA1\xA8\xC7\xB8\xDC\xA7\x61\xA1\x49");
	}
      }
      else
      {
	/* ◆ 對方把耳朵摀住說：『我沒聽到……我沒聽到……』 */
	printchatline("\xA1\xBB \xB9\xEF\xA4\xE8\xA7\xE2\xA6\xD5\xA6\xB7\xDD\xB3\xA6\xED\xBB\xA1\xA1\x47\xA1\x79\xA7\xDA\xA8\x53\xC5\xA5\xA8\xEC\xA1\x4B\xA1\x4B\xA7\xDA\xA8\x53\xC5\xA5\xA8\xEC\xA1\x4B\xA1\x4B\xA1\x7A");
      }
    }
    else
    {
      /* ◆ 對方不在站上 */
      printchatline("\xA1\xBB \xB9\xEF\xA4\xE8\xA4\xA3\xA6\x62\xAF\xB8\xA4\x57");
    }
  }
  else
  {
    printchatline(err_uid);
  }
}


static int
printuserent(uentp)
  user_info *uentp;
{
  static char uline[80];
  static int cnt;
  char pline[30];
  int cloak;

  if (!uentp)
  {
    if (cnt)
      printchatline(uline);
    memset(uline, 0, 80);
    return cnt = 0;
  }
  cloak = uentp->ufo & UFO_CLOAK;
  if (cloak && !HAS_PERM(PERM_SEECLOAK))
    return 0;

  sprintf(pline, " %-13s%c%-10s", uentp->userid,
    cloak ? '#' : ' ', bmode(uentp, 1));
  if (cnt < 2)
    /* │ */
    strcat(pline, "\xA2\x78");
  strcat(uline, pline);
  if (++cnt == 3)
  {
    printchatline(uline);
    memset(uline, 0, 80);
    cnt = 0;
  }
  return 0;
}


static void
chat_users()
{				/* 因為人數動輒上百，意義不大 */
  printchatline("");
  /* 【  */
  /* 遊客列表 】 */
  printchatline("\xA1\x69 " BBSNAME "\xB9\x43\xAB\xC8\xA6\x43\xAA\xED \xA1\x6A");
  printchatline(MSG_CHAT_ULIST);

  if (apply_ulist(printuserent) == -1)
    /* 空無一人 */
    printchatline("\xAA\xC5\xB5\x4C\xA4\x40\xA4\x48");

  printuserent(NULL);
}
#endif


struct chat_command
{
  char *cmdname;		/* Char-room command length */
  void (*cmdfunc) ();		/* Pointer to function */
};


struct chat_command chat_cmdtbl[] = 
{
  {"pager", chat_pager},
  {"tape", chat_record},

#if 0
  /* Thor.980727: 和 /flag 衝key */
  {"fire", chat_write},

  {"users", chat_users},
#endif

  {NULL, NULL}
};


static inline int
chat_cmd_match(buf, str)
  char *buf;
  char *str;
{
  int c1, c2;

  for (;;)
  {
    c1 = *str++;
    if (!c1)
      break;

    c2 = *buf++;
    if (!c2 || c2 == ' ' || c2 == '\n')
      break;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;

    if (c1 != c2)
      return 0;
  }

  return 1;
}


static inline int
chat_cmd(fd, buf)
  int fd;
  char *buf;
{
  struct chat_command *cmd;
  char *key;

  buf++;
  for (cmd = chat_cmdtbl; key = cmd->cmdname; cmd++)
  {
    if (chat_cmd_match(buf, key))
    {
      cmd->cmdfunc(buf);
      return '/';
    }
  }

  return 0;
}


extern char lastcmd[MAXLASTCMD][80];

#define CHAT_YPOS	10


int
t_chat()
{
  ACCT acct;
  int ch, cfd, cmdpos, cmdcol;
  char *ptr, buf[80], chatid[9];
  struct sockaddr_in sin;
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif
  
#ifdef CHAT_SECURE
  extern char passbuf[];
#endif

#ifdef EVERY_Z
  /* Thor.980725: 為 talk & chat 可用 ^z 作準備 */
  if (vio_holdon())
  {
    /* 您講話講一半還沒講完耶 */
    vmsg("\xB1\x7A\xC1\xBF\xB8\xDC\xC1\xBF\xA4\x40\xA5\x62\xC1\xD9\xA8\x53\xC1\xBF\xA7\xB9\xAD\x43");
    return -1;
  }
#endif

#if     defined(__OpenBSD__)

  if (!(h = gethostbyname(str_host)))
    return -1;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
            
#else

  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  /* sin.sin_addr.s_addr = INADDR_LOOPBACK; */
  /* sin.sin_addr.s_addr = INADDR_ANY; */
  /* for FreeBSD 4.x */  
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  cfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cfd < 0)
    return -1;

  if (connect(cfd, (struct sockaddr *) & sin, sizeof sin))
  {
    close(cfd);
    blog("CHAT ", "connect");
    return -1;
  }

  for (;;)
  {
    /* 請輸入聊天代號： */
    ch = vget(b_lines, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9\xA1\x47", chatid, 9, DOECHO);
    if (ch == '/')
    {
      continue;
    }
    else if (!ch)
    {
      /* str_ncpy(chatid, cuser.userid, sizeof(chatid)); */
      close(cfd);	/* itoc.010322: 大部分都是誤按到 Talk->Chat 改成預設為離開 */
      return 0;
    }
    else
    {
      /* itoc.010528: 不可以用別人的 id 做為聊天代號 */
      if (acct_load(&acct, chatid) >= 0 && acct.userno != cuser.userno)
      {
	/* 抱歉這個代號有人註冊為 id，所以您不能當成聊天代號 */
	vmsg("\xA9\xEA\xBA\x70\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xA6\xB3\xA4\x48\xB5\xF9\xA5\x55\xAC\xB0 id\xA1\x41\xA9\xD2\xA5\x48\xB1\x7A\xA4\xA3\xAF\xE0\xB7\xED\xA6\xA8\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9");
	continue;
      }
      /* Thor.980911: chatid中不可以空白, 防止 parse錯誤 */
      for(ch = 0; ch < 8; ch++)
      {
        if (chatid[ch] == ' ')
          break;
        else if (!chatid[ch]) /* Thor.980921: 如果0的話就結束 */
          ch = 8;
      }
      if (ch < 8) 
        continue;
    }

#ifdef CHAT_SECURE	/* Thor.980729: secured chat room */

#if 0
    作者  opus (人生有味是清歡)                                站內 sysopplan
    標題  Re: 關於 chatroom
    時間  Wed Jul 30 03:14:56 1997
    ─────────────────────────────────────

    > passwd 是連續的非空白字元嗎??

    三個參數 userid + chatid + passwd 中, userid/chatid 皆不含 space, 
    而 passwd 可包含 space, 所以必須將它擺在第三個位置以便 token-parsing。

    > 另外, ACCT 中的 passwd 是PASSLEN自動補空白還是要手動加?
    > 會自動補滿嗎?

    Unix 的 crypt 最多只取前 14 個字, 所以不加亦可。
    這個地方 參照 user login 的地方寫就好了。

    --
    ※ Origin: 楓橋驛站(bbs.cs.nthu.edu.tw) From: thcs-8.cs.nthu.edu.tw
#endif

    /* Thor.980819: 拿掉userno */
    /* Thor.980730: passwd 改為最後的參數 */
    /* Thor.980813: passwd 改為真正的 password */
    /* Thor.980813: xchatd中, chatid 自動跳過空白, 所以有空白會 invalid login */
#if 0
    sprintf(buf, "/! %d %s %s %s\n",
      cuser.userno, cuser.userid, chatid, cuser.passwd);
      cuser.userno, cuser.userid, chatid, passbuf);
#endif
    sprintf(buf, "/! %s %s %s\n",
      cuser.userid, chatid, passbuf);

#else
    sprintf(buf, "/! %d %d %s %s\n",
      cuser.userno, cuser.userlevel, cuser.userid, chatid);
#endif

    chat_send(cfd, buf);
    if (recv(cfd, buf, 3, 0) != 3)
      return 0;

    if (!strcmp(buf, CHAT_LOGIN_OK))
      break;
    else if (!strcmp(buf, CHAT_LOGIN_EXISTS))
      /* 這個代號已經有人用了 */
      ptr = "\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xA4\x77\xB8\x67\xA6\xB3\xA4\x48\xA5\xCE\xA4\x46";
    else if (!strcmp(buf, CHAT_LOGIN_INVALID))
      /* 這個代號是錯誤的 */
      ptr = "\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xAC\x4F\xBF\xF9\xBB\x7E\xAA\xBA";
    else if (!strcmp(buf, CHAT_LOGIN_BOGUS))
    {				/* Thor: 禁止相同二人進入 */
      close(cfd);
      /* 請勿派遣「分身」進入談天室 */
      vmsg("\xBD\xD0\xA4\xC5\xAC\xA3\xBB\xBA\xA1\x75\xA4\xC0\xA8\xAD\xA1\x76\xB6\x69\xA4\x4A\xBD\xCD\xA4\xD1\xAB\xC7");
      return 0;
    }
    move(b_lines - 1, 0);
    outs(ptr);
    clrtoeol();
    bell();
  }

  clear();
  move(1, 0);
  outs(msg_seperator);
  move(stop_line, 0);
  outs(msg_seperator);
  print_chatid(chatid);
  memset(ptr = buf, 0, sizeof(buf));
  chatline = 2;
  cmdcol = 0;
  cmdpos = -1;

  add_io(cfd, 60);

  strcpy(cutmp->mateid, chatid);

  for (;;)
  {
    move(b_lines - 1, cmdcol + CHAT_YPOS);
    ch = vkey();

    if (ch == I_OTHERDATA)
    {				/* incoming */
      if (chat_recv(cfd, chatid) == -1)
	break;
      continue;
    }

    if (isprint2(ch))
    {
      if (cmdcol < 68)
      {
	if (ptr[cmdcol])
	{			/* insert */
	  int i;

	  for (i = cmdcol; ptr[i] && i < 68; i++);
	  ptr[i + 1] = '\0';
	  for (; i > cmdcol; i--)
	    ptr[i] = ptr[i - 1];
	}
	else
	{			/* append */
	  ptr[cmdcol + 1] = '\0';
	}
	ptr[cmdcol] = ch;
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(&ptr[cmdcol++]);
      }
      continue;
    }

    if (ch == '\n')
    {
#ifdef EVERY_BIFF
      /* Thor.980805: 有人在旁邊按enter才需要check biff */
      static int old_biff;
      int biff = HAS_STATUS(STATUS_BIFF);
      if (biff && !old_biff)
        /* ◆ 噹! 郵差來按鈴了! */
        printchatline("\xA1\xBB \xBE\xB4! \xB6\x6C\xAE\x74\xA8\xD3\xAB\xF6\xB9\x61\xA4\x46!");
      old_biff = biff;
#endif
      if (ch = *ptr)
      {
	if (ch == '/')
	  ch = chat_cmd(cfd, ptr);

        /* Thor.980602: 有個要注意的小地方, 原本如果是『/』, 
                        會秀出 /help的畫面,  
                        現在打 /, 會變成 /p 切換 pager */

        /* Thor.980925: 保留 ptr 最原始樣, 不加 /n */
	for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
	  strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
	strcpy(lastcmd[0], ptr);

	if (ch != '/')
	{
	  strcat(ptr, "\n");
	  if (!chat_send(cfd, ptr))
	    break;
	}
	if (*ptr == '/' && ptr[1] == 'b')
	  break;

#if 0
	for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
	  strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
	strcpy(lastcmd[0], ptr);
#endif

	*ptr = '\0';
	cmdcol = 0;
	cmdpos = -1;
	move(b_lines - 1, CHAT_YPOS);
	clrtoeol();
      }
      continue;
    }

    if (ch == KEY_BKSP)
    {
      if (cmdcol)
      {
	ch = cmdcol;
	cmdcol--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 判斷現在刪除的位置是否為漢字的後半段，若是刪二字元 */
	if ((cuser.ufo & UFO_ZHC) && cmdcol && IS_ZHC_LO(ptr, cmdcol))
	  cmdcol--;
#endif
	strcpy(ptr + cmdcol, ptr + ch);
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(ptr + cmdcol);
	clrtoeol();
      }
      continue;
    }

    if (ch == KEY_DEL)
    {
      if (ptr[cmdcol])
      {
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 判斷現在刪除的位置是否為漢字的前半段，若是刪二字元 */
	if ((cuser.ufo & UFO_ZHC) && ptr[cmdcol + 1] && IS_ZHC_HI(ptr[cmdcol]))
	  ch = 2;
	else
#endif
	  ch = 1;
	strcpy(ptr + cmdcol, ptr + cmdcol + ch);
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(ptr + cmdcol);
	clrtoeol();
      }
      continue;
    }

    if (ch == Ctrl('D'))
    {
      chat_send(cfd, "/b\n");	/* /bye 離開 */
      break;
    }

    if (ch == Ctrl('C'))	/* itoc.註解: 清除 input 整行 */
    {
      *ptr = '\0';
      cmdcol = 0;
      move(b_lines - 1, CHAT_YPOS);
      clrtoeol();
      continue;
    }

    if (ch == KEY_HOME || ch == Ctrl('A'))
    {
      cmdcol = 0;
      continue;
    }

    if (ch == KEY_END || ch == Ctrl('E'))
    {
      cmdcol = strlen(ptr);
      continue;
    }
    
    if (ch == KEY_LEFT)
    {
      if (cmdcol)
      {
	cmdcol--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 左移時碰到漢字移雙格 */
	if ((cuser.ufo & UFO_ZHC) && cmdcol && IS_ZHC_LO(ptr, cmdcol))
	  cmdcol--;
#endif
      }
      continue;
    }

    if (ch == KEY_RIGHT)
    {
      if (ptr[cmdcol])
      {
	cmdcol++;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 右移時碰到漢字移雙格 */
	if ((cuser.ufo & UFO_ZHC) && ptr[cmdcol] && IS_ZHC_HI(ptr[cmdcol - 1]))
	  cmdcol++;
#endif
      }
      continue;
    }

#ifdef EVERY_Z
    /* Thor: Chat 中按 ctrl-z */
    if (ch == Ctrl('Z'))
    {
      char buf[IDLEN + 1];
      screenline slt[T_LINES];

      /* Thor.980731: 暫存 mateid, 因為出去時可能會用掉 mateid */
      strcpy(buf, cutmp->mateid);

      vio_save();	/* Thor.980727: 暫存 vio_fd */
      vs_save(slt);
      every_Z(0);
      vs_restore(slt);
      vio_restore();	/* Thor.980727: 還原 vio_fd */

      /* Thor.980731: 還原 mateid, 因為出去時可能會用掉 mateid */
      strcpy(cutmp->mateid, buf);
      continue;
    }
#endif

    if (ch == KEY_DOWN)
    {
      cmdpos += MAXLASTCMD - 2;
      ch = KEY_UP;
    }

    if (ch == KEY_UP)
    {
      cmdpos++;
      cmdpos %= MAXLASTCMD;
      strcpy(ptr, lastcmd[cmdpos]);
      move(b_lines - 1, CHAT_YPOS);
      outs(ptr);
      clrtoeol();
      cmdcol = strlen(ptr);
    }
  }

  if (frec)
    chat_record();

  close(cfd);
  add_io(0, 60);
  cutmp->mateid[0] = '\0';
  return 0;
}
