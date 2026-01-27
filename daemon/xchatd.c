/*-------------------------------------------------------*/
/* xchatd.c     ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : super KTV daemon for chat server             */
/* create : 95/03/29                                     */
/* update : 97/10/20                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "xchat.h"


#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/resource.h>


#define	SERVER_USAGE
#define WATCH_DOG
#undef	DEBUG			/* 程式除錯之用 */
#undef	MONITOR			/* 監督 chatroom 活動以解決糾紛 */
#undef	STAND_ALONE		/* 不搭配 BBS 獨立執行 */


#ifdef	DEBUG
#define	MONITOR
#endif

static int gline;

#ifdef  WATCH_DOG
#define MYDOG  gline = __LINE__
#else
#define MYDOG			/* NOOP */
#endif


#define CHAT_PIDFILE    "run/chat.pid"
#define CHAT_LOGFILE    "run/chat.log"
#define	CHAT_INTERVAL	(60 * 30)
#define SOCK_QLEN	3


/* name of the main room (always exists) */


#define	MAIN_NAME	"main"
/* 有緣千里來相會 */
#define	MAIN_TOPIC	"\xA6\xB3\xBD\x74\xA4\x64\xA8\xBD\xA8\xD3\xAC\xDB\xB7\x7C"


#define ROOM_LOCKED	1
#define ROOM_SECRET	2
#define ROOM_OPENTOPIC  4
#define ROOM_ALL	(NULL)


#define LOCKED(room)	(room->rflag & ROOM_LOCKED)
#define SECRET(room)	(room->rflag & ROOM_SECRET)
#define OPENTOPIC(room) (room->rflag & ROOM_OPENTOPIC)


#define RESTRICTED(usr)	(usr->uflag == 0)	/* guest */
#define CHATSYSOP(usr)	(usr->uflag & PERM_ALLCHAT)
#define	PERM_ROOMOP	PERM_CHAT	/* Thor: 借 PERM_CHAT 為 PERM_ROOMOP */
#define	PERM_CHATOP	PERM_DENYCHAT	/* Thor: 借 PERM_DENYCHAT 為 PERM_CHATOP */
/* #define ROOMOP(usr)  (usr->uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
/* Thor.980603: PERM_ALLCHAT 改為 default 沒有 roomop, 但可以自己取得 chatop */
#define ROOMOP(usr)	(usr->uflag & (PERM_ROOMOP | PERM_CHATOP))
#define CLOAK(usr)	(usr->uflag & PERM_CLOAK)


/* ----------------------------------------------------- */
/* ChatRoom data structure                               */
/* ----------------------------------------------------- */


typedef struct ChatRoom ChatRoom;
typedef struct ChatUser ChatUser;
typedef struct UserList UserList;
typedef struct ChatCmd ChatCmd;
typedef struct ChatAction ChatAction;


struct ChatUser
{
  ChatUser *unext;
  ChatRoom *room;
  UserList *ignore;
  int sock;			/* user socket */
  int userno;
  int uflag;
  int clitype;			/* Xshadow: client type. 1 for common client,
				 * 0 for bbs only client */
  time_t tbegin;
  time_t uptime;
  int sno;
  int xdata;
  int retry;

  int isize;			/* current size of ibuf */
  char ibuf[80];		/* buffer for non-blocking receiving */
  char userid[IDLEN + 1];	/* real userid */
  char chatid[9];		/* chat id */
  char rhost[30];		/* host address */
};


struct ChatRoom
{
  ChatRoom *next, *prev;
  UserList *invite;
  char name[IDLEN + 1];
  char topic[48];		/* Let the room op to define room topic */
  int rflag;			/* ROOM_LOCKED, ROOM_SECRET, ROOM_OPENTOPIC */
  int occupants;		/* number of users in room */
};


struct UserList
{
  UserList *next;
  int userno;
  char userid[0];
};


struct ChatCmd
{
  char *cmdstr;
  void (*cmdfunc) ();
  int exact;
};


static ChatRoom mainroom, *roompool;
static ChatUser *mainuser, *userpool;
static fd_set mainfset;
static int totaluser;		/* current number of connections */
static struct timeval zerotv;	/* timeval for selecting */
static int common_client_command;


#ifdef STAND_ALONE
static int userno_inc = 0;	/* userno auto-incrementer */
#endif


/* ◆ 您不是這間聊天室的 Op */
static char msg_not_op[] = "\xA1\xBB \xB1\x7A\xA4\xA3\xAC\x4F\xB3\x6F\xB6\xA1\xB2\xE1\xA4\xD1\xAB\xC7\xAA\xBA Op";
/* ◆ 目前沒有人使用 [%s] 這個聊天代號 */
static char msg_no_such_id[] = "\xA1\xBB \xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA4\x48\xA8\xCF\xA5\xCE [%s] \xB3\x6F\xAD\xD3\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9";
/* ◆ [%s] 不在這間聊天室 */
static char msg_not_here[] = "\xA1\xBB [%s] \xA4\xA3\xA6\x62\xB3\x6F\xB6\xA1\xB2\xE1\xA4\xD1\xAB\xC7";


#define	FUZZY_USER	((ChatUser *) -1)


/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


static FILE *flog;


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  fprintf(flog, "%02d/%02d %02d:%02d:%02d %-13s%s\n",
    p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
}


static inline void
log_init()
{
  FILE *fp;

  /* --------------------------------------------------- */
  /* log daemon's PID					 */
  /* --------------------------------------------------- */

  if (fp = fopen(CHAT_PIDFILE, "w"))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(CHAT_LOGFILE, "a");
  logit("START", "chat daemon");
}


#ifdef	DEBUG
static char chatbuf[256];	/* general purpose buffer */


static void
debug_list(list)
  UserList *list;
{
  char buf[80];
  int i = 0;

  if (!list)
  {
    logit("DEBUG_L", "NULL");
    return;
  }
  while (list)
  {
    sprintf(buf, "%d) list: %p userno: %d next: %p", i++, list, list->userno, list->next);
    logit("DEBUG_L", buf);

    list = list->next;
  }
  logit("DEBUG_L", "end");
}


static void
debug_user()
{
  ChatUser *user;
  int i;
  char buf[80];

  sprintf(buf, "mainuser: %p userpool: %p", mainuser, userpool);
  logit("DEBUG_U", buf);
  for (i = 0, user = mainuser; user; user = user->unext)
  {
    /* MYDOG; */
    sprintf(buf, "%d) %p %-6d %s %s", ++i, user, user->userno, user->userid, user->chatid);
    logit("DEBUG_U", buf);
  }
}


static void
debug_room()
{
  ChatRoom *room;
  int i;
  char buf[80];

  i = 0;
  room = &mainroom;

  sprintf(buf, "mainroom: %p roompool: %p", mainroom, roompool);
  logit("DEBUG_R", buf);
  do
  {
    MYDOG;
    sprintf(buf, "%d) %p %s %d", ++i, room, room->name, room->occupants);
    logit("DEBUG_R", buf);
  } while (room = room->next);
}


static void
log_user(cu)
  ChatUser *cu;
{
  static int log_num;

  if (cu)
  {
    if (log_num > 100 && log_num < 150)
    {
      sprintf(chatbuf, "%d: %p <%d>", log_num, cu, gline);
      logit("travese user ", chatbuf);
    }
    else if (log_num == 100)
    {
      sprintf(chatbuf, "BOOM !! at line %d", gline);
      logit("travese user ", chatbuf);
    }
    log_num++;
  }
  else
    log_num = 0;
}
#endif				/* DEBUG */


/* ----------------------------------------------------- */
/* string routines                                       */
/* ----------------------------------------------------- */


static int
valid_chatid(id)
  char *id;
{
  int ch, len;

  for (len = 0; ch = *id; id++)
  { /* Thor.980921: 空白為不合理chatid, 怕getnext判斷錯誤等等 */
    if (ch == '/' || ch == '*' || ch == ':' || ch ==' ')
      return 0;
    if (++len > 8)
      return 0;
  }
  return len;
}


/* itoc.註解: 由於改採 MUD-like 的部分 match 即可 */
/* 所以 MUD-like 的 action 盡量不要用英文縮寫，並不要有重覆的 */

static int		/* 0: fit */
str_belong(s1, s2)	/* itoc.010321: 讓 mud-like 指令部分 match 即可，和 mud 一樣 */
  uschar *s1;		/* ChatAction 裡的小寫 verb */
  uschar *s2;		/* user input command 大小寫均可 */
{
  int c1, c2;
  int num = 0;

  for (;;)
  {
    c1 = *s1;
    c2 = *s2;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;	/* 換小寫 */

    if (num >= 2)	/* 至少要有二字元相同 */
    {
      if (!c1 || !c2)	/* 完全 match 或部分 match 皆可 (s1包含s2 或 s2包含s1均算) */
        return 0;
    }

    if (c1 > c2)	/* itoc.010927: 不同的回傳值 */
      return 1;
    else if (c1 < c2)
      return -1;

    s1++;
    s2++;
    num++;
  }
}


/* ----------------------------------------------------- */
/* match strings' similarity case-insensitively          */
/* ----------------------------------------------------- */
/* str_match(keyword, string)				 */
/* ----------------------------------------------------- */
/* 0 : equal            ("foo", "foo")                   */
/* -1 : mismatch        ("abc", "xyz")                   */
/* ow : similar         ("goo", "good")                  */
/* ----------------------------------------------------- */


static int
str_match(s1, s2)
  uschar *s1;		/* lower-case (sub)string */
  uschar *s2;
{
  int c1, c2;

  for (;;)
  {
    c1 = *s1;
    c2 = *s2;

    if (!c1)
      return c2;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;	/* 換小寫 */

    if (c1 != c2)
      return -1;

    s1++;
    s2++;
  }
}


/* ----------------------------------------------------- */
/* search user/room by its ID                            */
/* ----------------------------------------------------- */


static ChatUser *
cuser_by_userid(userid)
  char *userid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, userid);
  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (!str_cmp(buf, cu->userid))
      break;
  }
  return cu;
}


static ChatUser *
cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, chatid);

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (!str_cmp(buf, cu->chatid))
      break;
  }
  return cu;
}


static ChatUser *
fuzzy_cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu, *xuser;
  int mode;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, chatid);
  xuser = NULL;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;

    mode = str_match(buf, cu->chatid);
    if (mode == 0)
      return cu;

    if (mode > 0)
    {
      if (xuser)
	return FUZZY_USER;	/* 符合者大於 2 人 */

      xuser = cu;
    }
  }
  return xuser;
}


static ChatRoom *
croom_by_roomid(roomid)
  char *roomid;
{
  ChatRoom *room;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, roomid);
  room = &mainroom;
  do
  {
    if (!str_cmp(buf, room->name))
      break;
  } while (room = room->next);
  return room;
}


/* ----------------------------------------------------- */
/* UserList routines                                     */
/* ----------------------------------------------------- */


static void
list_free(list)
  UserList **list;
{
  UserList *user, *next;

  for (user = *list, *list = NULL; user; user = next)
  {
    next = user->next;
    free(user);
  }
}


static void
list_add(list, user)
  UserList **list;
  ChatUser *user;
{
  UserList *node;
  char *userid;
  int len;

  len = strlen(userid = user->userid) + 1;
  if (node = (UserList *) malloc(sizeof(UserList) + len))
  {
    node->next = *list;
    node->userno = user->userno;
    memcpy(node->userid, userid, len);
    *list = node;
  }
}


static int
list_delete(list, userid)
  UserList **list;
  char *userid;
{
  UserList *node;
  char buf[80]; /* Thor.980727: 輸入一次最長才 80 */

  str_lower(buf, userid);

  while (node = *list)
  {
    if (!str_cmp(buf, node->userid))
    {
      *list = node->next;
      free(node);
      return 1;
    }
    list = &node->next;
  }

  return 0;
}


static int
list_belong(list, userno)
  UserList *list;
  int userno;
{
  while (list)
  {
    if (userno == list->userno)
      return 1;
    list = list->next;
  }
  return 0;
}


/* ------------------------------------------------------ */
/* non-blocking socket routines : send message to users   */
/* ------------------------------------------------------ */


static void
do_send(nfds, wset, msg)
  int nfds;
  fd_set *wset;
  char *msg;
{
  int len, sr;

#if 1
  /* Thor: for future reservation bug */
  zerotv.tv_sec = 0;
  zerotv.tv_usec = 0;
#endif

  sr = select(nfds + 1, NULL, wset, NULL, &zerotv);

  if (sr > 0)
  {
    len = strlen(msg) + 1;
    do
    {
      if (FD_ISSET(nfds, wset))
      {
	send(nfds, msg, len, 0);
	if (--sr <= 0)
	  return;
      }
    } while (--nfds > 0);
  }
}


static void
send_to_room(room, msg, userno, number)
  ChatRoom *room;
  char *msg;
  int userno;
  int number;
{
  ChatUser *cu;
  fd_set wset;
  int sock, max;
  int clitype;			/* 分為 bbs client 及 common client 兩次處理 */
  char *str, buf[256];

  for (clitype = (number == MSG_MESSAGE || !number) ? 0 : 1;
    clitype < 2; clitype++)
  {
    FD_ZERO(&wset);
    max = -1;

    for (cu = mainuser; cu; cu = cu->unext)
    {
      if (cu->userno && (cu->clitype == clitype) &&
	(room == ROOM_ALL || room == cu->room) &&
	(!userno || !list_belong(cu->ignore, userno)))
      {
	sock = cu->sock;

	FD_SET(sock, &wset);

	if (max < sock)
	  max = sock;
      }
    }

    if (max <= 0)
      continue;

    if (clitype)
    {
      str = buf;

      if (*msg)
	sprintf(str, "%3d %s", number, msg);
      else
	sprintf(str, "%3d", number);
    }
    else
    {
      str = msg;
    }

    do_send(max, &wset, str);
  }
}


static void
send_to_user(user, msg, userno, number)
  ChatUser *user;
  char *msg;
  int userno;
  int number;
{
  int sock;

#if 0
  if (!user->userno || (!user->clitype && number && number != MSG_MESSAGE))
#endif
  /* Thor.980911: 如果查user->userno則在login_user的error message會無法送回 */
  if (!user->clitype && number != MSG_MESSAGE)
    return;

  if ((sock = user->sock) <= 0)
    return;

  if (!userno || !list_belong(user->ignore, userno))
  {
    fd_set wset;
    char buf[256];

    FD_ZERO(&wset);
    FD_SET(sock, &wset);

    if (user->clitype)
    {
      if (*msg)
	sprintf(buf, "%3d %s", number, msg);
      else
	sprintf(buf, "%3d", number);
      msg = buf;
    }

    do_send(sock, &wset, msg);
  }
}


/* ----------------------------------------------------- */


static void
room_changed(room)
  ChatRoom *room;
{
  if (room)
  {
    char buf[256];

    sprintf(buf, "= %s %d %d %s",
      room->name, room->occupants, room->rflag, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


static void
user_changed(cu)
  ChatUser *cu;
{
  if (cu)
  {
    ChatRoom *room;
    char buf[256];

    room = cu->room;
    sprintf(buf, "= %s %s %s %s%s",
      cu->userid, cu->chatid, room->name, cu->rhost,
      ROOMOP(cu) ? " Op" : "");
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
  }
}


static void
exit_room(user, mode, msg)
  ChatUser *user;
  int mode;
  char *msg;
{
  ChatRoom *room;
  char buf[128];

  if (!(room = user->room))
    return;

  user->room = NULL;
  /* user->uflag &= ~(PERM_ROOMOP | PERM_ALLCHAT); */
  user->uflag &= ~PERM_ROOMOP;
  /* Thor.980601: 離開房間時只清 room op, 不清 sysop, chatroom 因天生具有 */

  if (--room->occupants > 0)
  {
    char *chatid;

    chatid = user->chatid;
    switch (mode)
    {
    case EXIT_LOGOUT:

      /* ◆ %s 離開了 ... %.50s */
      sprintf(buf, "\xA1\xBB %s \xC2\xF7\xB6\x7D\xA4\x46 ... %.50s", chatid, (msg && *msg) ? msg : "");
      break;

    case EXIT_LOSTCONN:

      /* ◆ %s 成了斷線的風箏囉 */
      sprintf(buf, "\xA1\xBB %s \xA6\xA8\xA4\x46\xC2\x5F\xBD\x75\xAA\xBA\xAD\xB7\xBA\xE5\xC5\x6F", chatid);
      break;

    case EXIT_KICK:

      /* ◆ 哈哈！%s 被踢出去了 */
      sprintf(buf, "\xA1\xBB \xAB\xA2\xAB\xA2\xA1\x49%s \xB3\x51\xBD\xF0\xA5\x58\xA5\x68\xA4\x46", chatid);
      break;
    }

    if (!CLOAK(user))
      send_to_room(room, buf, 0, MSG_MESSAGE);

    sprintf(buf, "- %s", user->userid);
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
    room_changed(room);
  }
  else if (room != &mainroom)
  {
    ChatRoom *next;

    fprintf(flog, "room-\t[%d] %s\n", user->sno, room->name);
    sprintf(buf, "- %s", room->name);

    room->prev->next = next = room->next;
    if (next)
      next->prev = room->prev;

    list_free(&room->invite);

    /* free(room); */

    /* 回收 */
    room->next = roompool;
    roompool = room;

    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


/* ----------------------------------------------------- */
/* chat commands                                         */
/* ----------------------------------------------------- */


#ifndef STAND_ALONE
/* ----------------------------------------------------- */
/* BBS server side routines                              */
/* ----------------------------------------------------- */


/* static */
int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, FN_ACCT);
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}


static void
chat_query(cu, msg)
  ChatUser *cu;
  char *msg;
{
  FILE *fp;
  ACCT acct;
  char buf[256];

  /* Thor.980617: 可先查是否為空字串 */
  if (*msg && acct_load(&acct, msg) >= 0)
  {
    /* %s(%s) 共上站 %d 次，文章 %d 篇 */
    sprintf(buf, "%s(%s) \xA6\x40\xA4\x57\xAF\xB8 %d \xA6\xB8\xA1\x41\xA4\xE5\xB3\xB9 %d \xBD\x67",
      acct.userid, acct.username, acct.numlogins, acct.numposts);
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    /* 最近(%s)從(%s)上站 */
    sprintf(buf, "\xB3\xCC\xAA\xF1(%s)\xB1\x71(%s)\xA4\x57\xAF\xB8", Btime(acct.lastlogin),
      /* 外太空 */
      (acct.lasthost[0] ? acct.lasthost : "\xA5\x7E\xA4\xD3\xAA\xC5"));
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    usr_fpath(buf, acct.userid, FN_PLANS);
    if (fp = fopen(buf, "r"))
    {
      int i;

      i = 0;
      while (fgets(buf, sizeof(buf), fp))
      {
	buf[strlen(buf) - 1] = 0;
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	if (++i >= MAXQUERYLINES)
	  break;
      }
      fclose(fp);
    }
  }
  else
  {
    sprintf(buf, msg_no_such_id, msg);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}
#endif


static void
chat_clear(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (cu->clitype)
    send_to_user(cu, "", 0, MSG_CLRSCR);
  else
    send_to_user(cu, "/c", 0, MSG_MESSAGE);
}


static void
chat_date(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[128];

  /* ◆ 標準時間: %s */
  sprintf(buf, "\xA1\xBB \xBC\xD0\xB7\xC7\xAE\xC9\xB6\xA1: %s", Now());
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_topic(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *room;
  char *topic, buf[128];

  room = cu->room;

  if (!ROOMOP(cu) && !OPENTOPIC(room))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    /* ※ 請指定話題 */
    send_to_user(cu, "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\x77\xB8\xDC\xC3\x44", 0, MSG_MESSAGE);
    return;
  }

  topic = room->topic;
  str_ncpy(topic, msg, sizeof(room->topic));

  if (cu->clitype)
  {
    send_to_room(room, topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/t%s", topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }

  room_changed(room);

  if (!CLOAK(cu))
  {
    /* ◆ %s 將話題改為 \033[1;32m%s\033[m */
    sprintf(buf, "\xA1\xBB %s \xB1\x4E\xB8\xDC\xC3\x44\xA7\xEF\xAC\xB0 \033[1;32m%s\033[m", cu->chatid, topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_version(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  sprintf(buf, "[Version] MapleBBS-3.10-20040726-PACK.itoc + Xchat-%d.%d",
    XCHAT_VERSION_MAJOR, XCHAT_VERSION_MINOR);
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_nick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *chatid, *str, buf[128];
  ChatUser *xuser;

  chatid = nextword(&msg);
  chatid[8] = '\0';
  if (!valid_chatid(chatid))
  {
    /* ※ 這個聊天代號是不正確的 */
    send_to_user(cu, "\xA1\xB0 \xB3\x6F\xAD\xD3\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9\xAC\x4F\xA4\xA3\xA5\xBF\xBD\x54\xAA\xBA", 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(chatid);
  if (xuser != NULL && xuser != cu)
  {
    /* ※ 已經有人捷足先登囉 */
    send_to_user(cu, "\xA1\xB0 \xA4\x77\xB8\x67\xA6\xB3\xA4\x48\xB1\xB6\xA8\xAC\xA5\xFD\xB5\x6E\xC5\x6F", 0, MSG_MESSAGE);
    return;
  }

  /* itoc.010528: 不可以用別人的 id 做為聊天代號 */
  usr_fpath(buf, chatid, NULL);
  if (dashd(buf) && str_cmp(chatid, cu->userid))
  {
    /* ※ 抱歉這個代號有人註冊為 id，所以您不能當成聊天代號 */
    send_to_user(cu, "\xA1\xB0 \xA9\xEA\xBA\x70\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xA6\xB3\xA4\x48\xB5\xF9\xA5\x55\xAC\xB0 id\xA1\x41\xA9\xD2\xA5\x48\xB1\x7A\xA4\xA3\xAF\xE0\xB7\xED\xA6\xA8\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9", 0, MSG_MESSAGE);
    return;
  }

  str = cu->chatid;

  if (!CLOAK(cu))
  {
    /* ※ %s 將聊天代號改為 \033[1;33m%s\033[m */
    sprintf(buf, "\xA1\xB0 %s \xB1\x4E\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9\xA7\xEF\xAC\xB0 \033[1;33m%s\033[m", str, chatid);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }

  strcpy(str, chatid);

  user_changed(cu);

  if (cu->clitype)
  {
    send_to_user(cu, chatid, 0, MSG_NICK);
  }
  else
  {
    sprintf(buf, "/n%s", chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_list_rooms(cuser, msg)
  ChatUser *cuser;
  char *msg;
{
  ChatRoom *cr, *room;
  char buf[128];
  int mode;

  if (RESTRICTED(cuser))
  {
    /* ※ 您沒有權限列出現有的聊天室 */
    send_to_user(cuser, "\xA1\xB0 \xB1\x7A\xA8\x53\xA6\xB3\xC5\x76\xAD\xAD\xA6\x43\xA5\x58\xB2\x7B\xA6\xB3\xAA\xBA\xB2\xE1\xA4\xD1\xAB\xC7", 0, MSG_MESSAGE);
    return;
  }

  mode = common_client_command;

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTSTART);
  else
    /* \033[7m 談天室名稱  │人數│話題        \033[m */
    send_to_user(cuser, "\033[7m \xBD\xCD\xA4\xD1\xAB\xC7\xA6\x57\xBA\xD9  \xA2\x78\xA4\x48\xBC\xC6\xA2\x78\xB8\xDC\xC3\x44        \033[m", 0,
      MSG_MESSAGE);

  room = cuser->room;
  cr = &mainroom;

  do
  {
    if ((cr == room) || !SECRET(cr) || CHATSYSOP(cuser))
    {
      if (mode)
      {
	sprintf(buf, "%s %d %d %s",
	  cr->name, cr->occupants, cr->rflag, cr->topic);
	send_to_user(cuser, buf, 0, MSG_ROOMLIST);
      }
      else
      {
	/*  %-12s│%4d│%s */
	sprintf(buf, " %-12s\xA2\x78%4d\xA2\x78%s", cr->name, cr->occupants, cr->topic);
	if (LOCKED(cr))
	  /*  [鎖住] */
	  strcat(buf, " [\xC2\xEA\xA6\xED]");
	if (SECRET(cr))
	  /*  [秘密] */
	  strcat(buf, " [\xAF\xB5\xB1\x4B]");
	if (OPENTOPIC(cr))
	  /*  [話題] */
	  strcat(buf, " [\xB8\xDC\xC3\x44]");
	send_to_user(cuser, buf, 0, MSG_MESSAGE);
      }
    }
  } while (cr = cr->next);

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTEND);
}


static void
chat_do_user_list(cu, msg, theroom)
  ChatUser *cu;
  char *msg;
  ChatRoom *theroom;
{
  ChatRoom *myroom, *room;
  ChatUser *user;
  int start, stop, curr, mode; /* , uflag; */
  char buf[128];

  curr = 0; /* Thor.980619: initialize curr */
  start = atoi(nextword(&msg));
  stop = atoi(nextword(&msg));

  mode = common_client_command;

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTSTART);
  else
    /* \033[7m 聊天代號│使用者代號  │聊天室 \033[m */
    send_to_user(cu, "\033[7m \xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9\xA2\x78\xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9  \xA2\x78\xB2\xE1\xA4\xD1\xAB\xC7 \033[m", 0,
      MSG_MESSAGE);

  myroom = cu->room;

  /* Thor.980717: 需要先排除 cu->userno == 0 的狀況嗎? */
  for (user = mainuser; user; user = user->unext)
  {
#if 0	/* Thor.980717: 既然 cu 都空了那還進來幹麼? */
    if (!cu->userno)
      continue;
#endif

    if (!user->userno)
      continue;

    room = user->room;
    if ((theroom != ROOM_ALL) && (theroom != room))
      continue;

    /* Thor.980717: viewer check */
    if ((myroom != room) && (RESTRICTED(cu) || (room && SECRET(room) && !CHATSYSOP(cu))))
      continue;

    /* Thor.980717: viewee check */
    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))
      continue;

    curr++;
    if (start && curr < start)
      continue;
    else if (stop && (curr > stop))
      break;

    if (mode)
    {
      if (!room)
	continue;		/* Xshadow: 還沒進入任何房間的就不列出 */

      sprintf(buf, "%s %s %s %s",
	user->chatid, user->userid, room->name, user->rhost);

      /* Thor.980603: PERM_ALLCHAT 改為 default 沒有 roomop, 但可以自己取得 */
      /* if (uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
      if (ROOMOP(user))
	strcat(buf, " Op");
    }
    else
    {
      /*  %-8s│%-12s│%s */
      sprintf(buf, " %-8s\xA2\x78%-12s\xA2\x78%s",
	/* [在門口徘徊] */
	user->chatid, user->userid, room ? room->name : "[\xA6\x62\xAA\xF9\xA4\x66\xB1\x72\xAB\xDE]");
      /* Thor.980603: PERM_ALLCHAT 改為 default 沒有 roomop, 但可以自己取得 */
      /* if (uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
      /* if (uflag & (PERM_ROOMOP | PERM_CHATOP)) */
      if (ROOMOP(user))  /* Thor.980602: 統一用法 */
	strcat(buf, " [Op]");
    }

    send_to_user(cu, buf, 0, mode ? MSG_USERLIST : MSG_MESSAGE);
  }

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTEND);
}


static void
chat_list_by_room(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *whichroom;
  char *roomstr, buf[128];

  roomstr = nextword(&msg);
  if (!*roomstr)
  {
    whichroom = cu->room;
  }
  else
  {
    if (!(whichroom = croom_by_roomid(roomstr)))
    {
      /* ※ 沒有 [%s] 這個聊天室 */
      sprintf(buf, "\xA1\xB0 \xA8\x53\xA6\xB3 [%s] \xB3\x6F\xAD\xD3\xB2\xE1\xA4\xD1\xAB\xC7", roomstr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      return;
    }

    if (whichroom != cu->room && SECRET(whichroom) && !CHATSYSOP(cu))
    {
      /* ※ 無法列出在秘密聊天室的使用者 */
      send_to_user(cu, "\xA1\xB0 \xB5\x4C\xAA\x6B\xA6\x43\xA5\x58\xA6\x62\xAF\xB5\xB1\x4B\xB2\xE1\xA4\xD1\xAB\xC7\xAA\xBA\xA8\xCF\xA5\xCE\xAA\xCC", 0, MSG_MESSAGE);
      return;
    }
  }
  chat_do_user_list(cu, msg, whichroom);
}


static void
chat_list_users(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_do_user_list(cu, msg, ROOM_ALL);
}


static void
chat_chatroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
    /* 聊天室 */
    send_to_user(cu, "\xB2\xE1\xA4\xD1\xAB\xC7", 0, MSG_CHATROOM);
}


static void
chat_map_chatids(cu, whichroom)
  ChatUser *cu;			/* Thor: 還沒有作不同間的 */
  ChatRoom *whichroom;
{
  int c;
  ChatRoom *myroom, *room;
  ChatUser *user;
  char buf[128];

  myroom = cu->room;

  send_to_user(cu,
    /* \033[7m 聊天代號 使用者代號  │ 聊天代號 使用者代號  │ 聊天代號 使用者代號 \033[m */
    "\033[7m \xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9 \xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9  \xA2\x78 \xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9 \xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9  \xA2\x78 \xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9 \xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9 \033[m", 0, MSG_MESSAGE);

  for (c = 0, user = mainuser; user; user = user->unext)
  {
    if (!cu->userno)
      continue;

    room = user->room;
    if (whichroom != ROOM_ALL && whichroom != room)
      continue;

    if (myroom != room)
    {
      if (RESTRICTED(cu) ||	/* Thor: 要先check room 是不是空的 */
	(room && SECRET(room) && !CHATSYSOP(cu)))
	continue;
    }

    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))	/* Thor:隱身術 */
      continue;

    sprintf(buf + (c * 24), " %-8s%c%-12s%s",
      user->chatid, ROOMOP(user) ? '*' : ' ',
      /* │ */
      user->userid, (c < 2 ? "\xA2\x78" : "  "));

    if (++c == 3)
    {
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      c = 0;
    }
  }

  if (c > 0)
    send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_map_chatids_thisroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_map_chatids(cu, cu->room);
}


static void
chat_setroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *modestr;
  ChatRoom *room;
  char *chatid;
  int sign, flag;
  char *fstr, buf[128];

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  modestr = nextword(&msg);
  sign = 1;
  if (*modestr == '+')
  {
    modestr++;
  }
  else if (*modestr == '-')
  {
    modestr++;
    sign = 0;
  }

  if (*modestr == '\0')
  {
    send_to_user(cu,
      /* ※ 請指定狀態: {[+(設定)][-(取消)]}{[L(鎖住)][s(秘密)][t(開放話題)} */
      "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\x77\xAA\xAC\xBA\x41: {[+(\xB3\x5D\xA9\x77)][-(\xA8\xFA\xAE\xF8)]}{[L(\xC2\xEA\xA6\xED)][s(\xAF\xB5\xB1\x4B)][t(\xB6\x7D\xA9\xF1\xB8\xDC\xC3\x44)}", 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  chatid = cu->chatid;

  while (*modestr)
  {
    flag = 0;
    switch (*modestr)
    {
    case 'l':
    case 'L':
      flag = ROOM_LOCKED;
      /* 鎖住 */
      fstr = "\xC2\xEA\xA6\xED";
      break;

    case 's':
    case 'S':
      flag = ROOM_SECRET;
      /* 秘密 */
      fstr = "\xAF\xB5\xB1\x4B";
      break;

    case 't':
    case 'T':
      flag = ROOM_OPENTOPIC;
      /* 開放話題 */
      fstr = "\xB6\x7D\xA9\xF1\xB8\xDC\xC3\x44";
      break;

    default:
      /* ※ 狀態錯誤：[%c] */
      sprintf(buf, "\xA1\xB0 \xAA\xAC\xBA\x41\xBF\xF9\xBB\x7E\xA1\x47[%c]", *modestr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
    }

    /* Thor: check room 是不是空的, 應該不是空的 */

    if (flag && (room->rflag & flag) != sign * flag)
    {
      room->rflag ^= flag;

      if (!CLOAK(cu))
      {
	/* ※ 本聊天室被 %s %s [%s] 狀態 */
	sprintf(buf, "\xA1\xB0 \xA5\xBB\xB2\xE1\xA4\xD1\xAB\xC7\xB3\x51 %s %s [%s] \xAA\xAC\xBA\x41",
	  /* 設定為 */
	  /* 取消 */
	  chatid, sign ? "\xB3\x5D\xA9\x77\xAC\xB0" : "\xA8\xFA\xAE\xF8", fstr);
	send_to_room(room, buf, 0, MSG_MESSAGE);
      }
    }
    modestr++;
  }

  /* Thor.980602: 不准 Main room 鎖起 or 秘密，否則離開的就進不來，要看也看不到。
     想要踢人也踢不進 main room，不會很奇怪嗎？ */

  if (!str_cmp(MAIN_NAME, room->name))
  {
    if (room->rflag & (ROOM_LOCKED | ROOM_SECRET))
    {
      /* ※ 但天使施了『復原』的魔法 */
      send_to_room(room, "\xA1\xB0 \xA6\xFD\xA4\xD1\xA8\xCF\xAC\x49\xA4\x46\xA1\x79\xB4\x5F\xAD\xEC\xA1\x7A\xAA\xBA\xC5\x5D\xAA\x6B", 0, MSG_MESSAGE);
      room->rflag &= ~(ROOM_LOCKED | ROOM_SECRET);
    }
  }

  room_changed(room);
}


static char *chat_msg[] =
{
  /* MUD-like 社交動詞 */
  "[//]help", "MUD-like \xAA\xC0\xA5\xE6\xB0\xCA\xB5\xFC",
  /* 談天室管理員專用指令 */
  "[/h]elp op", "\xBD\xCD\xA4\xD1\xAB\xC7\xBA\xDE\xB2\x7A\xAD\xFB\xB1\x4D\xA5\xCE\xAB\xFC\xA5\x4F",
  /* 做一個動作 */
  "[/a]ct <msg>", "\xB0\xB5\xA4\x40\xAD\xD3\xB0\xCA\xA7\x40",
  /* 道別 */
  "[/b]ye [msg]", "\xB9\x44\xA7\x4F",
  /* 清除螢幕  目前時間 */
  "[/c]lear  [/d]ate", "\xB2\x4D\xB0\xA3\xBF\xC3\xB9\xF5  \xA5\xD8\xAB\x65\xAE\xC9\xB6\xA1",
  /* 忽略使用者 */
  "[/i]gnore [user]", "\xA9\xBF\xB2\xA4\xA8\xCF\xA5\xCE\xAA\xCC",
  /* 建立或加入談天室 */
  "[/j]oin <room>", "\xAB\xD8\xA5\xDF\xA9\xCE\xA5\x5B\xA4\x4A\xBD\xCD\xA4\xD1\xAB\xC7",
  /* 列出談天室使用者 */
  "[/l]ist [start [stop]]", "\xA6\x43\xA5\x58\xBD\xCD\xA4\xD1\xAB\xC7\xA8\xCF\xA5\xCE\xAA\xCC",
  /* 跟 <id> 說悄悄話 */
  "[/m]sg <id|user> <msg>", "\xB8\xF2 <id> \xBB\xA1\xAE\xA8\xAE\xA8\xB8\xDC",
  /* 將談天代號換成 <id> */
  "[/n]ick <id>", "\xB1\x4E\xBD\xCD\xA4\xD1\xA5\x4E\xB8\xB9\xB4\xAB\xA6\xA8 <id>",
  /* 切換呼叫器 */
  "[/p]ager", "\xA4\xC1\xB4\xAB\xA9\x49\xA5\x73\xBE\xB9",
  /* 查詢網友 */
  "[/q]uery <user>", "\xAC\x64\xB8\xDF\xBA\xF4\xA4\xCD",
  /* 道別 */
  "[/qui]t [msg]", "\xB9\x44\xA7\x4F",
  /* 列出一般談天室 */
  "[/r]oom", "\xA6\x43\xA5\x58\xA4\x40\xAF\xEB\xBD\xCD\xA4\xD1\xAB\xC7",
  /* 開關錄音機 */
  "[/t]ape", "\xB6\x7D\xC3\xF6\xBF\xFD\xAD\xB5\xBE\xF7",
  /* 取消忽略 */
  "[/u]nignore <user>", "\xA8\xFA\xAE\xF8\xA9\xBF\xB2\xA4",
  /* 列出本談天室使用者 */
  "[/w]ho", "\xA6\x43\xA5\x58\xA5\xBB\xBD\xCD\xA4\xD1\xAB\xC7\xA8\xCF\xA5\xCE\xAA\xCC",
  /* 列出談天室<room> 的使用者 */
  "[/w]hoin <room>", "\xA6\x43\xA5\x58\xBD\xCD\xA4\xD1\xAB\xC7<room> \xAA\xBA\xA8\xCF\xA5\xCE\xAA\xCC",
  NULL
};


static char *room_msg[] =
{
  /* 設定鎖定、秘密、開放話題 */
  "[/f]lag [+-][lst]", "\xB3\x5D\xA9\x77\xC2\xEA\xA9\x77\xA1\x42\xAF\xB5\xB1\x4B\xA1\x42\xB6\x7D\xA9\xF1\xB8\xDC\xC3\x44",
  /* 邀請 <id> 加入談天室 */
  "[/i]nvite <id>", "\xC1\xDC\xBD\xD0 <id> \xA5\x5B\xA4\x4A\xBD\xCD\xA4\xD1\xAB\xC7",
  /* 將 <id> 踢出談天室 */
  "[/kick] <id>", "\xB1\x4E <id> \xBD\xF0\xA5\x58\xBD\xCD\xA4\xD1\xAB\xC7",
  /* 將 Op 的權力轉移給 <id> */
  "[/o]p [<id>]", "\xB1\x4E Op \xAA\xBA\xC5\x76\xA4\x4F\xC2\xE0\xB2\xBE\xB5\xB9 <id>",
  /* 換個話題 */
  "[/topic] <text>", "\xB4\xAB\xAD\xD3\xB8\xDC\xC3\x44",
  /* 廣播 (站長專用) */
  "[/w]all", "\xBC\x73\xBC\xBD (\xAF\xB8\xAA\xF8\xB1\x4D\xA5\xCE)",
  NULL
};


static void
chat_help(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char **table, *str, buf[128];

  if (!str_cmp("op", nextword(&msg)))
  {
    /* 談天室管理員專用指令 */
    send_to_user(cu, "\xBD\xCD\xA4\xD1\xAB\xC7\xBA\xDE\xB2\x7A\xAD\xFB\xB1\x4D\xA5\xCE\xAB\xFC\xA5\x4F", 0, MSG_MESSAGE);
    table = room_msg;
  }
  else
  {
    table = chat_msg;
  }

  while (str = *table++)
  {
    sprintf(buf, "  %-20s- %s", str, *table++);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_private(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatUser *xuser;
  char *recipient, buf[128];

  recipient = nextword(&msg);
  xuser = (ChatUser *) fuzzy_cuser_by_chatid(recipient);
  if (xuser == NULL)		/* Thor.980724: 用 userid也可傳悄悄話 */
  {
    xuser = cuser_by_userid(recipient);
  }

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, recipient);
  }
  else if (xuser == FUZZY_USER)
  {				/* ambiguous */
    /* ※ 請指明聊天代號 */
    strcpy(buf, "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\xFA\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9");
  }
  else if (*msg)
  {
    int userno;

    userno = cu->userno;

    sprintf(buf, "\033[1m*%s*\033[m %.50s", cu->chatid, msg);
    send_to_user(xuser, buf, userno, MSG_MESSAGE);

    if (xuser->clitype)		/* Xshadow: 如果對方是用 client 上來的 */
    {
      sprintf(buf, "%s %s %.50s", cu->userid, cu->chatid, msg);
      send_to_user(xuser, buf, userno, MSG_PRIVMSG);
    }

    if (cu->clitype)
    {
      sprintf(buf, "%s %s %.50s", xuser->userid, xuser->chatid, msg);
      send_to_user(cu, buf, 0, MSG_MYPRIVMSG);
    }

    sprintf(buf, "%s> %.50s", xuser->chatid, msg);
  }
  else
  {
    /* ※ 您想對 %s 說什麼話呢？ */
    sprintf(buf, "\xA1\xB0 \xB1\x7A\xB7\x51\xB9\xEF %s \xBB\xA1\xA4\xB0\xBB\xF2\xB8\xDC\xA9\x4F\xA1\x48", xuser->chatid);
  }

  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_cloak(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (CHATSYSOP(cu))
  {
    char buf[128];

    cu->uflag ^= PERM_CLOAK;
    /* ◆ %s */
    sprintf(buf, "\xA1\xBB %s", CLOAK(cu) ? MSG_CLOAKED : MSG_UNCLOAK);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


/* ----------------------------------------------------- */


static void
arrive_room(cuser, room)
  ChatUser *cuser;
  ChatRoom *room;
{
  char *rname, buf[256];

  /* Xshadow: 不必送給自己, 反正換房間就會重新 build user list */

  sprintf(buf, "+ %s %s %s %s",
    cuser->userid, cuser->chatid, room->name, cuser->rhost);
  if (ROOMOP(cuser))
    strcat(buf, " Op");
  send_to_room(room, buf, 0, MSG_USERNOTIFY);

  room->occupants++;
  room_changed(room);

  cuser->room = room;
  rname = room->name;

  if (cuser->clitype)
  {
    send_to_user(cuser, rname, 0, MSG_ROOM);
    send_to_user(cuser, room->topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/r%s", rname);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
    sprintf(buf, "/t%s", room->topic);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
  }

  /* ※ \033[32;1m%s\033[m 進入 \033[33;1m[%s]\033[m 包廂 */
  sprintf(buf, "\xA1\xB0 \033[32;1m%s\033[m \xB6\x69\xA4\x4A \033[33;1m[%s]\033[m \xA5\x5D\xB4\x5B",
    cuser->chatid, rname);

  if (!CLOAK(cuser))
    send_to_room(room, buf, cuser->userno, MSG_MESSAGE);
  else
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
}


static int
enter_room(cuser, rname, msg)
  ChatUser *cuser;
  char *rname;
  char *msg;
{
  ChatRoom *room;
  int create;
  char buf[256];

  create = 0;
  room = croom_by_roomid(rname);

  if (room == NULL)
  {
    /* new room */

#ifdef DEBUG
    logit(cuser->userid, "create new room");
    debug_room();
#endif

    if (room = roompool)
    {
      roompool = room->next;
    }
    else
    {
      room = (ChatRoom *) malloc(sizeof(ChatRoom));
    }

    if (room == NULL)
    {
      /* ※ 無法再新闢包廂了 */
      send_to_user(cuser, "\xA1\xB0 \xB5\x4C\xAA\x6B\xA6\x41\xB7\x73\xC5\x50\xA5\x5D\xB4\x5B\xA4\x46", 0, MSG_MESSAGE);
      return 0;
    }

    memset(room, 0, sizeof(ChatRoom));
    str_ncpy(room->name, rname, sizeof(room->name));
    /* 這是一個新天地 */
    strcpy(room->topic, "\xB3\x6F\xAC\x4F\xA4\x40\xAD\xD3\xB7\x73\xA4\xD1\xA6\x61");

    sprintf(buf, "+ %s 1 0 %s", room->name, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);

    if (mainroom.next)
      mainroom.next->prev = room;
    room->next = mainroom.next;

    mainroom.next = room;
    room->prev = &mainroom;

#ifdef DEBUG
    logit(cuser->userid, "create room succeed");
    debug_room();
#endif

    create = 1;
    fprintf(flog, "room+\t[%d] %s\n", cuser->sno, rname);
  }
  else
  {
    if (cuser->room == room)
    {
      /* ※ 您本來就在 [%s] 聊天室囉 :) */
      sprintf(buf, "\xA1\xB0 \xB1\x7A\xA5\xBB\xA8\xD3\xB4\x4E\xA6\x62 [%s] \xB2\xE1\xA4\xD1\xAB\xC7\xC5\x6F :)", rname);
      send_to_user(cuser, buf, 0, MSG_MESSAGE);
      return 0;
    }

    if (!CHATSYSOP(cuser) && LOCKED(room) &&
      !list_belong(room->invite, cuser->userno))
    {
      /* ※ 內有惡犬，非請莫入 */
      send_to_user(cuser, "\xA1\xB0 \xA4\xBA\xA6\xB3\xB4\x63\xA4\xFC\xA1\x41\xAB\x44\xBD\xD0\xB2\xF6\xA4\x4A", 0, MSG_MESSAGE);
      return 0;
    }
  }

  exit_room(cuser, EXIT_LOGOUT, msg);
  arrive_room(cuser, room);

  if (create)
    cuser->uflag |= PERM_ROOMOP;

  return 0;
}


static void
cuser_free(cuser)
  ChatUser *cuser;
{
  int sock;

  sock = cuser->sock;
  shutdown(sock, 2);
  close(sock);

  FD_CLR(sock, &mainfset);

  list_free(&cuser->ignore);
  totaluser--;

  if (cuser->room)
  {
    exit_room(cuser, EXIT_LOSTCONN, NULL);
  }

  fprintf(flog, "BYE\t[%d] T%d X%d\n",
    cuser->sno, time(0) - cuser->tbegin, cuser->xdata);
}


static void
print_user_counts(cuser)
  ChatUser *cuser;
{
  ChatRoom *room;
  int num, userc, suserc, roomc, number;
  char buf[256];

  userc = suserc = roomc = 0;

  room = &mainroom;
  do
  {
    num = room->occupants;
    if (SECRET(room))
    {
      suserc += num;
      if (CHATSYSOP(cuser))
	roomc++;
    }
    else
    {
      userc += num;
      roomc++;
    }
  } while (room = room->next);

  number = (cuser->clitype) ? MSG_MOTD : MSG_MESSAGE;

  sprintf(buf,
    /* ⊙ 歡迎光臨【聊天室】，目前開了 \033[1;31m%d\033[m 間包廂 */
    "\xA1\xF3 \xC5\x77\xAA\xEF\xA5\xFA\xC1\x7B\xA1\x69\xB2\xE1\xA4\xD1\xAB\xC7\xA1\x6A\xA1\x41\xA5\xD8\xAB\x65\xB6\x7D\xA4\x46 \033[1;31m%d\033[m \xB6\xA1\xA5\x5D\xB4\x5B", roomc);
  send_to_user(cuser, buf, 0, number);

  /* ⊙ 共有 \033[1;36m%d\033[m 人來擺龍門陣 */
  sprintf(buf, "\xA1\xF3 \xA6\x40\xA6\xB3 \033[1;36m%d\033[m \xA4\x48\xA8\xD3\xC2\x5C\xC0\x73\xAA\xF9\xB0\x7D", userc);
  if (suserc)
    /*  [%d 人在秘密聊天室] */
    sprintf(buf + strlen(buf), " [%d \xA4\x48\xA6\x62\xAF\xB5\xB1\x4B\xB2\xE1\xA4\xD1\xAB\xC7]", suserc);

  send_to_user(cuser, buf, 0, number);
}


static int
login_user(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int utent;

  char *userid;
  char *chatid, *passwd;
  ChatUser *xuser;
  int level;
  /* struct hostent *hp; */

#ifndef STAND_ALONE
  ACCT acct;
#endif

  /* Xshadow.0915: common client support : /-! userid chatid password */
  /* client/server 版本依據 userid 抓 .PASSWDS 判斷 userlevel */

  userid = nextword(&msg);
  chatid = nextword(&msg);

#ifdef	DEBUG
  logit("ENTER", userid);
#endif

#ifndef STAND_ALONE
  /* Thor.980730: parse space before passwd */

  passwd = msg;

  /* Thor.980813: 跳過一空格即可, 因為反正如果chatid有空格, 密碼也不對 */
  /* 就算密碼對, 也不會怎麼樣:p */
  /* 可是如果密碼第一個字是空格, 那跳太多空格會進不來... */
  /* Thor.980910: 由於 nextword修改為後接空格填0, 傳入值則直接後移至0後,
                  所以不需作此動作 */
#if 0
  if (*passwd == ' ')
    passwd++;
#endif

  /* Thor.980729: load acct */

  if (!*userid || (acct_load(&acct, userid) < 0))
  {

#ifdef	DEBUG
    logit("noexist", userid);
#endif

    if (cu->clitype)
      /* 錯誤的使用者代號 */
      send_to_user(cu, "\xBF\xF9\xBB\x7E\xAA\xBA\xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9", 0, ERR_LOGIN_NOSUCHUSER);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  /* Thor.980813: 改用真實 password check, for C/S bbs */

  /* Thor.990214: 注意 daolib 中 非 0 代表失敗 */
  /* if (!chkpasswd(acct.passwd, passwd)) */
  if (chkpasswd(acct.passwd, passwd))
  {

#ifdef	DEBUG
    logit("fake", userid);
#endif

    if (cu->clitype)
      /* 密碼錯誤 */
      send_to_user(cu, "\xB1\x4B\xBD\x58\xBF\xF9\xBB\x7E", 0, ERR_LOGIN_PASSERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  level = acct.userlevel;
  utent = acct.userno;

#else				/* STAND_ALONE */
  level = 1;
  utent = ++userno_inc;
#endif				/* STAND_ALONE */

  /* Thor.980819: for client/server bbs */

#ifdef DEBUG
  log_user(NULL);
#endif

  for (xuser = mainuser; xuser; xuser = xuser->unext)
  {

#ifdef DEBUG
    log_user(xuser);
#endif

    if (xuser->userno == utent)
    {

#ifdef	DEBUG
      logit("enter", "bogus");
#endif

      if (cu->clitype)
	/* 請勿派遣分身進入聊天室！ */
	send_to_user(cu, "\xBD\xD0\xA4\xC5\xAC\xA3\xBB\xBA\xA4\xC0\xA8\xAD\xB6\x69\xA4\x4A\xB2\xE1\xA4\xD1\xAB\xC7\xA1\x49", 0,
	  ERR_LOGIN_USERONLINE);
      else
	send_to_user(cu, CHAT_LOGIN_BOGUS, 0, MSG_MESSAGE);
      return -1;		/* Thor: 或是0等它自己了斷? */
    }
  }


#ifndef STAND_ALONE
  /* Thor.980629: 暫時借用 invalid_chatid 濾除 沒有PERM_CHAT的人 */

  if (!valid_chatid(chatid) || !(level & PERM_CHAT) || (level & PERM_DENYCHAT))
  { /* Thor.981012: 徹底一些, 連 denychat也BAN掉, 免得 client作怪 */

#ifdef	DEBUG
    logit("enter", chatid);
#endif

    if (cu->clitype)
      /* 不合法的聊天室代號！ */
      send_to_user(cu, "\xA4\xA3\xA6\x58\xAA\x6B\xAA\xBA\xB2\xE1\xA4\xD1\xAB\xC7\xA5\x4E\xB8\xB9\xA1\x49", 0, ERR_LOGIN_NICKERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);
    return 0;
  }
#endif

#ifdef	DEBUG
  debug_user();
#endif

  if (cuser_by_chatid(chatid) != NULL)
  {
    /* chatid in use */

#ifdef	DEBUG
    logit("enter", "duplicate");
#endif

    if (cu->clitype)
      /* 這個代號已經有人使用 */
      send_to_user(cu, "\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xA4\x77\xB8\x67\xA6\xB3\xA4\x48\xA8\xCF\xA5\xCE", 0, ERR_LOGIN_NICKINUSE);
    else
      send_to_user(cu, CHAT_LOGIN_EXISTS, 0, MSG_MESSAGE);
    return 0;
  }

#ifdef DEBUG			/* CHATSYSOP 一進來就隱身 */
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | (CHATSYSOP(cu) ? 0 : PERM_CLOAK));
#else
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | PERM_CLOAK);
#endif

  /* Thor: 進來先清空 ROOMOP (同PERM_CHAT) */

  strcpy(cu->userid, userid);
  str_ncpy(cu->chatid, chatid, sizeof(cu->chatid));
  /* Thor.980921: str_ncpy與一般 strncpy有所不同, 特別注意 */

  fprintf(flog, "ENTER\t[%d] %s\n", cu->sno, userid);

  /* Xshadow: 取得 client 的來源 */

  dns_name(cu->rhost, cu->ibuf);
  str_ncpy(cu->rhost, cu->ibuf, sizeof(cu->rhost));
#if 0
  hp = gethostbyaddr(cu->rhost, sizeof(struct in_addr), AF_INET);
  str_ncpy(cu->rhost, hp ? hp->h_name : inet_ntoa((struct in_addr *) cu->rhost), sizeof(cu->rhost));
#endif

  cu->userno = utent;

  if (cu->clitype)
    /* 順利 */
    send_to_user(cu, "\xB6\xB6\xA7\x51", 0, MSG_LOGINOK);
  else
    send_to_user(cu, CHAT_LOGIN_OK, 0, MSG_MESSAGE);

  arrive_room(cu, &mainroom);

  send_to_user(cu, "", 0, MSG_MOTDSTART);
  print_user_counts(cu);
  send_to_user(cu, "", 0, MSG_MOTDEND);

#ifdef	DEBUG
  logit("enter", "OK");
#endif

  return 0;
}


static void
chat_act(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (*msg)
  {
    char buf[256];

    sprintf(buf, "%s \033[36m%s\033[m", cu->chatid, msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }
}


static void
chat_ignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *str, buf[256];

  if (RESTRICTED(cu))
  {
    /* ※ 您沒有 ignore 別人的權利 */
    str = "\xA1\xB0 \xB1\x7A\xA8\x53\xA6\xB3 ignore \xA7\x4F\xA4\x48\xAA\xBA\xC5\x76\xA7\x51";
  }
  else
  {
    char *ignoree;

    str = buf;
    ignoree = nextword(&msg);
    if (*ignoree)
    {
      ChatUser *xuser;

      xuser = cuser_by_userid(ignoree);

      if (xuser == NULL)
      {
	sprintf(str, msg_no_such_id, ignoree);
      }
      else if (xuser == cu || CHATSYSOP(xuser) ||
	(ROOMOP(xuser) && (xuser->room == cu->room)))
      {
	/* ◆ 不可以 ignore [%s] */
	sprintf(str, "\xA1\xBB \xA4\xA3\xA5\x69\xA5\x48 ignore [%s]", ignoree);
      }
      else
      {
	if (list_belong(cu->ignore, xuser->userno))
	{
	  /* ※ %s 已經被凍結了 */
	  sprintf(str, "\xA1\xB0 %s \xA4\x77\xB8\x67\xB3\x51\xAD\xE1\xB5\xB2\xA4\x46", xuser->chatid);
	}
	else
	{
	  list_add(&(cu->ignore), xuser);
	  /* ◆ 將 [%s] 打入冷宮了 :p */
	  sprintf(str, "\xA1\xBB \xB1\x4E [%s] \xA5\xB4\xA4\x4A\xA7\x4E\xAE\x63\xA4\x46 :p", xuser->chatid);
	}
      }
    }
    else
    {
      UserList *list;

      if (list = cu->ignore)
      {
	int len;
	char userid[16];

	/* ◆ 這些人被打入冷宮了： */
	send_to_user(cu, "\xA1\xBB \xB3\x6F\xA8\xC7\xA4\x48\xB3\x51\xA5\xB4\xA4\x4A\xA7\x4E\xAE\x63\xA4\x46\xA1\x47", 0, MSG_MESSAGE);
	len = 0;
	do
	{
	  sprintf(userid, "%-13s", list->userid);
	  strcpy(str + len, userid);
	  len += 13;
	  if (len >= 78)
	  {
	    send_to_user(cu, str, 0, MSG_MESSAGE);
	    len = 0;
	  }
	} while (list = list->next);

	if (len == 0)
	  return;
      }
      else
      {
	/* ◆ 您目前並沒有 ignore 任何人 */
	str = "\xA1\xBB \xB1\x7A\xA5\xD8\xAB\x65\xA8\xC3\xA8\x53\xA6\xB3 ignore \xA5\xF4\xA6\xF3\xA4\x48";
      }
    }
  }

  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_unignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *ignoree, *str, buf[80];

  ignoree = nextword(&msg);

  if (*ignoree)
  {
    sprintf(str = buf, (list_delete(&(cu->ignore), ignoree)) ?
      /* ◆ [%s] 不再被您冷落了 */
      "\xA1\xBB [%s] \xA4\xA3\xA6\x41\xB3\x51\xB1\x7A\xA7\x4E\xB8\xA8\xA4\x46" :
      /* ◆ 您並未 ignore [%s] 這號人物 */
      "\xA1\xBB \xB1\x7A\xA8\xC3\xA5\xBC ignore [%s] \xB3\x6F\xB8\xB9\xA4\x48\xAA\xAB", ignoree);
  }
  else
  {
    /* ◆ 請指明 user ID */
    str = "\xA1\xBB \xBD\xD0\xAB\xFC\xA9\xFA user ID";
  }
  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_join(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (RESTRICTED(cu))
  {
    /* ※ 您沒有加入其他聊天室的權限 */
    send_to_user(cu, "\xA1\xB0 \xB1\x7A\xA8\x53\xA6\xB3\xA5\x5B\xA4\x4A\xA8\xE4\xA5\x4C\xB2\xE1\xA4\xD1\xAB\xC7\xAA\xBA\xC5\x76\xAD\xAD", 0, MSG_MESSAGE);
  }
  else
  {
    char *roomid = nextword(&msg);

    if (*roomid)
      enter_room(cu, roomid, msg);
    else
      /* ※ 請指定聊天室 */
      send_to_user(cu, "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\x77\xB2\xE1\xA4\xD1\xAB\xC7", 0, MSG_MESSAGE);
  }
}


static void
chat_kick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *twit, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  twit = nextword(&msg);
  xuser = cuser_by_chatid(twit);

  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid也嘛通 */
    xuser = cuser_by_userid(twit);
  }

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (CHATSYSOP(xuser))
  {
    /* ◆ 不可以 kick [%s] */
    sprintf(buf, "\xA1\xBB \xA4\xA3\xA5\x69\xA5\x48 kick [%s]", twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  exit_room(xuser, EXIT_KICK, (char *) NULL);

  if (room == &mainroom)
    xuser->uptime = 0;		/* logout_user(xuser); */
  else
    enter_room(xuser, MAIN_NAME, (char *) NULL);
    /* Thor.980602: 其實踢就踢,不要show出xxx離開了的訊息比較好 */
}


static void
chat_makeop(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *newop, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  /* Thor.980603: PERM_ALLCHAT 改為 default 沒有 roomop, 但可以自己取得 */

  newop = nextword(&msg);

  room = cu->room;

  if (!*newop && CHATSYSOP(cu))
  {
    /* Thor.980603: PERM_ALLCHAT 改為 default 沒有 roomop, 但可以自己取得 */
    cu->uflag ^= PERM_CHATOP;

    user_changed(cu);
    if (!CLOAK(cu))
    {
      /* ※ 天使 將 Op 權力授予 %s */
      sprintf(buf,ROOMOP(cu) ? "\xA1\xB0 \xA4\xD1\xA8\xCF \xB1\x4E Op \xC5\x76\xA4\x4F\xB1\xC2\xA4\xA9 %s"
                             /* ※ 天使 將 %s 的 Op 權力收回 */
                             : "\xA1\xB0 \xA4\xD1\xA8\xCF \xB1\x4E %s \xAA\xBA Op \xC5\x76\xA4\x4F\xA6\xAC\xA6\x5E", cu->chatid);
      send_to_room(room, buf, 0, MSG_MESSAGE);
    }

    return;
  }

  /* if (!ROOMOP(cu)) */
  if (!(cu->uflag & PERM_ROOMOP)) /* Thor.980603: chat room總管不能轉移 Op 權力 */
  {
    /* ◆ 您不能轉移 Op 的權力 */
    send_to_user(cu, "\xA1\xBB \xB1\x7A\xA4\xA3\xAF\xE0\xC2\xE0\xB2\xBE Op \xAA\xBA\xC5\x76\xA4\x4F" /* msg_not_op */, 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(newop);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid 嘛也通 */
    xuser = cuser_by_userid(newop);
  }
#endif

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, newop);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (cu == xuser)
  {
    /* ※ 您早就已經是 Op 了啊 */
    send_to_user(cu, "\xA1\xB0 \xB1\x7A\xA6\xAD\xB4\x4E\xA4\x77\xB8\x67\xAC\x4F Op \xA4\x46\xB0\xDA", 0, MSG_MESSAGE);
    return;
  }

  /* room = cu->room; */

  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  cu->uflag &= ~PERM_ROOMOP;
  xuser->uflag |= PERM_ROOMOP;

  user_changed(cu);
  user_changed(xuser);

  if (!CLOAK(cu))
  {
    /* ※ %s 將 Op 權力轉移給 %s */
    sprintf(buf, "\xA1\xB0 %s \xB1\x4E Op \xC5\x76\xA4\x4F\xC2\xE0\xB2\xBE\xB5\xB9 %s",
      cu->chatid, xuser->chatid);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_invite(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *invitee, buf[80];
  ChatUser *xuser;
  ChatRoom *room;
  UserList **list;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  invitee = nextword(&msg);
  xuser = cuser_by_chatid(invitee);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid 嘛也通 */
    xuser = cuser_by_userid(invitee);
  }
#endif

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, invitee);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;		/* Thor: 是否要 check room 是否 NULL ? */
  list = &(room->invite);

  if (list_belong(*list, xuser->userno))
  {
    /* ※ %s 已經接受過邀請了 */
    sprintf(buf, "\xA1\xB0 %s \xA4\x77\xB8\x67\xB1\xB5\xA8\xFC\xB9\x4C\xC1\xDC\xBD\xD0\xA4\x46", xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }
  list_add(list, xuser);

  /* ※ %s 邀請您到 [%s] 聊天室 */
  sprintf(buf, "\xA1\xB0 %s \xC1\xDC\xBD\xD0\xB1\x7A\xA8\xEC [%s] \xB2\xE1\xA4\xD1\xAB\xC7",
    cu->chatid, room->name);
  send_to_user(xuser, buf, 0, MSG_MESSAGE);
  /* ※ %s 收到您的邀請了 */
  sprintf(buf, "\xA1\xB0 %s \xA6\xAC\xA8\xEC\xB1\x7A\xAA\xBA\xC1\xDC\xBD\xD0\xA4\x46", xuser->chatid);
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_broadcast(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  if (!CHATSYSOP(cu))
  {
    /* ※ 您沒有在聊天室廣播的權力! */
    send_to_user(cu, "\xA1\xB0 \xB1\x7A\xA8\x53\xA6\xB3\xA6\x62\xB2\xE1\xA4\xD1\xAB\xC7\xBC\x73\xBC\xBD\xAA\xBA\xC5\x76\xA4\x4F!", 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    /* ※ 請指定廣播內容 */
    send_to_user(cu, "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\x77\xBC\x73\xBC\xBD\xA4\xBA\xAE\x65", 0, MSG_MESSAGE);
    return;
  }

  /* \033[1m※  */
  /* 談天室廣播中 [%s].....\033[m */
  sprintf(buf, "\033[1m\xA1\xB0 " BBSNAME "\xBD\xCD\xA4\xD1\xAB\xC7\xBC\x73\xBC\xBD\xA4\xA4 [%s].....\033[m",
    cu->chatid);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
  /* ◆ %s */
  sprintf(buf, "\xA1\xBB %s", msg);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
}


static void
chat_bye(cu, msg)
  ChatUser *cu;
  char *msg;
{
  exit_room(cu, EXIT_LOGOUT, msg);
  cu->uptime = 0;
  /* logout_user(cu); */
}


/* --------------------------------------------- */
/* MUD-like social commands : action             */
/* --------------------------------------------- */


#if 0	/* itoc.010816: 重新翻修一些不太適當的 action 敘述 */
  1. 注意按字母排列。
  2. 請愛用全形標點符號。
  3. 三類 action 不能有重覆。
  4. 由於 action 採用「部分比對」，故最好不要有指令包含另一指令所有關鍵字的狀況。
     （例如 fire/fireball，kiss/kissbye，no/nod，tea/tear/tease，drive/drivel，love/lover）
     （有這樣的情形也不會怎麼樣，只是使用者容易搞混）
  5. 由於 action 部分比對至少 2 bytes，故不要用 //1 //2 這類只有一個字的 action。
  6. 由於 action 採用部分比對，故指令不必用縮寫。
  7. 統一 action message 最後不要加句點。
  8. 修正錯字。（是 adore，不是 aodre 啊 :p）
  9. 減少重覆的字眼。（不要老是「死去活來」啊 :p）
#endif


struct ChatAction
{
  char *verb;			/* 動詞 */
  char *chinese;		/* 中文翻譯 */
  char *part1_msg;		/* 介詞 */
  char *part2_msg;		/* 動作 */
};


static ChatAction *
action_fit(action, actnum, cmd)		/* 找看看是哪個 ChatAction */
  ChatAction *action;
  int actnum;
  char *cmd;
{
  ChatAction *pos, *locus, *mid;	/* locus:左指標 mid:中指標 pos:右指標 */
  int cmp;

  /* itoc.010927: 由於 ChatAction 都是按字母排序的，所以可以用 binary search */
  /* itoc.010928.註解:由於是 binary search 所以雖然 recline 排在 recycle 前面
    但是打 //rec 時卻可能出現 //recycle 的效果，判定優先次序端賴 binary 的順序 */

  locus = action;
  pos = action + actnum - 1;		/* 最後一個是 NULL，但不可能被檢查到 */

  while (1)
  {
    if (pos <= locus + 1)
      break;

    mid = locus + ((pos - locus) >> 1);

    if (!(cmp = str_belong(mid->verb, cmd)))	/* itoc.010321: MUD-like match */
      return mid;
    else if (cmp < 0)
      locus = mid;
    else
      pos = mid;
  }

  /* 特例: 如果右指標停留在 1，要檢查第 0 個 */
  if (pos == action + 1)
  {
    if (!str_belong(action->verb, cmd))		/* itoc.010321: MUD-like match */
      return action;
  }

  return NULL;
}


/* itoc.010805.註解:  //adore sysop   itoc 對 sysop 的景仰有如滔滔江水，連綿不絕… */

#define ACTNUM_PARTY	110

static ChatAction party_data[ACTNUM_PARTY] =
{
  {
    /* 景仰 */
    /* 對 */
    /* 的景仰有如滔滔江水，連綿不絕… */
    "adore", "\xB4\xBA\xA5\xF5", "\xB9\xEF", "\xAA\xBA\xB4\xBA\xA5\xF5\xA6\xB3\xA6\x70\xB7\xCA\xB7\xCA\xA6\xBF\xA4\xF4\xA1\x41\xB3\x73\xBA\xF8\xA4\xA3\xB5\xB4\xA1\x4B"
  },
  {
    /* 阿魯巴 */
    /* 把 */
    /* 架上柱子阿到死！ */
    "aluba", "\xAA\xFC\xBE\x7C\xA4\xDA", "\xA7\xE2", "\xAC\x5B\xA4\x57\xAC\x57\xA4\x6C\xAA\xFC\xA8\xEC\xA6\xBA\xA1\x49"
  },
  {
    /* 阿魯巴 */
    /* 把 */
    /* 架上柱子阿到死！ */
    "aruba", "\xAA\xFC\xBE\x7C\xA4\xDA", "\xA7\xE2", "\xAC\x5B\xA4\x57\xAC\x57\xA4\x6C\xAA\xFC\xA8\xEC\xA6\xBA\xA1\x49"
  },
  {
    /* 吠叫 */
    /* 汪汪！對 */
    /* 大聲吠叫 */
    "bark", "\xA7\x70\xA5\x73", "\xA8\x4C\xA8\x4C\xA1\x49\xB9\xEF", "\xA4\x6A\xC1\x6E\xA7\x70\xA5\x73"
  },
  {
    /* 啃咬 */
    /* 把 */
    /* 咬得死去活來 */
    "bite", "\xB0\xD9\xAB\x72", "\xA7\xE2", "\xAB\x72\xB1\x6F\xA6\xBA\xA5\x68\xAC\xA1\xA8\xD3"
  },
  {
    /* 一刀 */
    /* 一刀把 */
    /* 送上西天 */
    "blade", "\xA4\x40\xA4\x4D", "\xA4\x40\xA4\x4D\xA7\xE2", "\xB0\x65\xA4\x57\xA6\xE8\xA4\xD1"
  },
  {
    /* 祝福 */
    /* 祝福 */
    /* 心想事成 */
    "bless", "\xAF\xAC\xBA\xD6", "\xAF\xAC\xBA\xD6", "\xA4\xDF\xB7\x51\xA8\xC6\xA6\xA8"
  },
  {
    /* 眨眼 */
    /* 對著 */
    /* 眨眨眼，不知暗示著什麼 */
    "blink", "\xAF\x77\xB2\xB4", "\xB9\xEF\xB5\xDB", "\xAF\x77\xAF\x77\xB2\xB4\xA1\x41\xA4\xA3\xAA\xBE\xB7\x74\xA5\xDC\xB5\xDB\xA4\xB0\xBB\xF2"
  },
  {
    /* 主機板 */
    /* 把 */
    /* 抓去跪主機板 */
    "board", "\xA5\x44\xBE\xF7\xAA\x4F", "\xA7\xE2", "\xA7\xEC\xA5\x68\xB8\xF7\xA5\x44\xBE\xF7\xAA\x4F"
  },
  {
    /* 氣功 */
    /* 雙掌微合，蓄勢待發……突然間，電光乍現，對 */
    /* 使出了Ｂｏ--Ｋａｎ */
    "bokan", "\xAE\xF0\xA5\x5C", "\xC2\xF9\xB4\x78\xB7\x4C\xA6\x58\xA1\x41\xBB\x57\xB6\xD5\xAB\xDD\xB5\x6F\xA1\x4B\xA1\x4B\xAC\xF0\xB5\x4D\xB6\xA1\xA1\x41\xB9\x71\xA5\xFA\xA5\x45\xB2\x7B\xA1\x41\xB9\xEF", "\xA8\xCF\xA5\x58\xA4\x46\xA2\xD0\xA2\xF7--\xA2\xD9\xA2\xE9\xA2\xF6"
  },
  {
    /* 鞠躬 */
    /* 畢躬畢敬的向 */
    /* 鞠躬 */
    "bow", "\xC1\xF9\xB0\x60", "\xB2\xA6\xB0\x60\xB2\xA6\xB7\x71\xAA\xBA\xA6\x56", "\xC1\xF9\xB0\x60"
  },
  {
    /* 幕之內 */
    /* 開始輪擺式移位，對 */
    /* 作肝臟攻擊 */
    "box", "\xB9\xF5\xA4\xA7\xA4\xBA", "\xB6\x7D\xA9\x6C\xBD\xFC\xC2\x5C\xA6\xA1\xB2\xBE\xA6\xEC\xA1\x41\xB9\xEF", "\xA7\x40\xA8\x78\xC5\xA6\xA7\xF0\xC0\xBB"
  },
  {
    /* 掰掰 */
    /* 向 */
    /* 說掰掰 */
    "bye", "\xD9\x54\xD9\x54", "\xA6\x56", "\xBB\xA1\xD9\x54\xD9\x54"
  },
  {
    /* 丟蛋糕 */
    /* 拿出一個蛋糕，往 */
    /* 的臉上砸去 */
    "cake", "\xA5\xE1\xB3\x4A\xBF\x7C", "\xAE\xB3\xA5\x58\xA4\x40\xAD\xD3\xB3\x4A\xBF\x7C\xA1\x41\xA9\xB9", "\xAA\xBA\xC1\x79\xA4\x57\xAF\x7B\xA5\x68"
  },
  {
    /* 呼喚 */
    /* 大聲的呼喚，啊～ */
    /* 啊～人在哪裡啊啊～啊 */
    "call", "\xA9\x49\xB3\xEA", "\xA4\x6A\xC1\x6E\xAA\xBA\xA9\x49\xB3\xEA\xA1\x41\xB0\xDA\xA1\xE3",	"\xB0\xDA\xA1\xE3\xA4\x48\xA6\x62\xAD\xFE\xB8\xCC\xB0\xDA\xB0\xDA\xA1\xE3\xB0\xDA"
  },
  {
    /* 輕撫 */
    /* 輕輕的撫摸著 */
    "caress", "\xBB\xB4\xBC\xBE", "\xBB\xB4\xBB\xB4\xAA\xBA\xBC\xBE\xBA\x4E\xB5\xDB", ""
  },
  {
    /* 鼓掌 */
    /* 向 */
    /* 熱烈鼓掌 */
    "clap", "\xB9\xAA\xB4\x78", "\xA6\x56", "\xBC\xF6\xAF\x50\xB9\xAA\xB4\x78"
  },
  {
    /* 抓抓 */
    /* 從貓咪樂園借了隻貓爪，把 */
    /* 抓得昏天暗地 */
    "claw", "\xA7\xEC\xA7\xEC", "\xB1\x71\xBF\xDF\xAB\x7D\xBC\xD6\xB6\xE9\xAD\xC9\xA4\x46\xB0\xA6\xBF\xDF\xA4\xF6\xA1\x41\xA7\xE2",	"\xA7\xEC\xB1\x6F\xA9\xFC\xA4\xD1\xB7\x74\xA6\x61"
  },
  {
    /* 切鬧鐘 */
    /* 切掉 */
    /* 的鬧鐘，快起床啦 */
    "clock", "\xA4\xC1\xBE\x78\xC4\xC1", "\xA4\xC1\xB1\xBC", "\xAA\xBA\xBE\x78\xC4\xC1\xA1\x41\xA7\xD6\xB0\x5F\xA7\xC9\xB0\xD5"
  },
  {
    /* 灌可樂 */
    /* 對 */
    /* 灌了一加侖的可樂 */
    "cola", "\xC4\xE9\xA5\x69\xBC\xD6", "\xB9\xEF", "\xC4\xE9\xA4\x46\xA4\x40\xA5\x5B\xA8\xDA\xAA\xBA\xA5\x69\xBC\xD6"
  },
  {
    /* 安慰 */
    /* 溫言安慰 */
    "comfort", "\xA6\x77\xBC\xA2", "\xB7\xC5\xA8\xA5\xA6\x77\xBC\xA2", ""
  },
  {
    /* 恭喜 */
    /* 從背後拿出了拉炮，呯！呯！恭喜 */
    "congratulate", "\xAE\xA5\xB3\xDF", "\xB1\x71\xAD\x49\xAB\xE1\xAE\xB3\xA5\x58\xA4\x46\xA9\xD4\xAC\xB6\xA1\x41\xCB\xE9\xA1\x49\xCB\xE9\xA1\x49\xAE\xA5\xB3\xDF", ""
  },
  {
    /* 鞭打 */
    /* 拿鞭子對 */
    /* 狠狠地抽打 */
    "cowhide", "\xC3\x40\xA5\xB4","\xAE\xB3\xC3\x40\xA4\x6C\xB9\xEF", "\xAC\xBD\xAC\xBD\xA6\x61\xA9\xE2\xA5\xB4"
  },
  {
    /* 口對口 */
    /* 對著 */
    /* 做口對口人工呼吸 */
    "cpr", "\xA4\x66\xB9\xEF\xA4\x66", "\xB9\xEF\xB5\xDB", "\xB0\xB5\xA4\x66\xB9\xEF\xA4\x66\xA4\x48\xA4\x75\xA9\x49\xA7\x6C"
  },
  {
    /* 道德 */
    /* 說： */
    /* 的道德指數不夠，滿臉戾氣 */
    "crime", "\xB9\x44\xBC\x77", "\xBB\xA1\xA1\x47", "\xAA\xBA\xB9\x44\xBC\x77\xAB\xFC\xBC\xC6\xA4\xA3\xB0\xF7\xA1\x41\xBA\xA1\xC1\x79\xA9\xD1\xAE\xF0"
  },
  {
    /* 乞憐 */
    /* 向 */
    /* 卑躬屈膝，搖尾乞憐 */
    "cringe", "\xA4\x5E\xBC\xA6", "\xA6\x56", "\xA8\xF5\xB0\x60\xA9\x7D\xBD\xA5\xA1\x41\xB7\x6E\xA7\xC0\xA4\x5E\xBC\xA6"
  },
  {
    /* 大哭 */
    /* 向 */
    /* 嚎啕大哭 */
    "cry", "\xA4\x6A\xAD\xFA", "\xA6\x56", "\xC0\x7A\xB0\xDE\xA4\x6A\xAD\xFA"
  },
  {
    /* 中古禮 */
    /* 優雅地對著 */
    /* 行中古世紀的屈膝禮。 */
    "curtsy", "\xA4\xA4\xA5\x6A\xC2\xA7", "\xC0\x75\xB6\xAE\xA6\x61\xB9\xEF\xB5\xDB", "\xA6\xE6\xA4\xA4\xA5\x6A\xA5\x40\xAC\xF6\xAA\xBA\xA9\x7D\xBD\xA5\xC2\xA7\xA1\x43"
  },
  {
    /* 跳舞 */
    /* 拉了 */
    /* 的手翩翩起舞 */
    "dance", "\xB8\xF5\xBB\x52", "\xA9\xD4\xA4\x46", "\xAA\xBA\xA4\xE2\xBD\xA1\xBD\xA1\xB0\x5F\xBB\x52"
  },
  {
    /* 毀滅 */
    /* 祭起了『極大毀滅咒文』，轟向 */
    "destroy", "\xB7\xB4\xB7\xC0", "\xB2\xBD\xB0\x5F\xA4\x46\xA1\x79\xB7\xA5\xA4\x6A\xB7\xB4\xB7\xC0\xA9\x47\xA4\xE5\xA1\x7A\xA1\x41\xC5\x46\xA6\x56", ""
  },
  {
    /* 狗腿 */
    /* 對 */
    /* 阿諛奉承，大大狗腿了一番 */
    "dogleg", "\xAA\xAF\xBB\x4C", "\xB9\xEF", "\xAA\xFC\xBD\xDB\xA9\x5E\xA9\xD3\xA1\x41\xA4\x6A\xA4\x6A\xAA\xAF\xBB\x4C\xA4\x46\xA4\x40\xB5\x66"
  },
  {
    /* 流口水 */
    /* 對著 */
    /* 流口水 */
    "drivel", "\xAC\x79\xA4\x66\xA4\xF4",	"\xB9\xEF\xB5\xDB",	"\xAC\x79\xA4\x66\xA4\xF4"
  },
  {
    /* 羨慕 */
    /* 向 */
    /* 流露出羨慕的眼光 */
    "envy", "\xB8\x72\xBC\x7D", "\xA6\x56", "\xAC\x79\xC5\x53\xA5\x58\xB8\x72\xBC\x7D\xAA\xBA\xB2\xB4\xA5\xFA"
  },
  {
    /* 晚安 */
    /* 對 */
    /* 說『晚安』 */
    "evening", "\xB1\xDF\xA6\x77", "\xB9\xEF", "\xBB\xA1\xA1\x79\xB1\xDF\xA6\x77\xA1\x7A"
  },
  {
    /* 送秋波 */
    /* 對 */
    /* 頻送秋波 */
    "eye", "\xB0\x65\xAC\xEE\xAA\x69", "\xB9\xEF", "\xC0\x57\xB0\x65\xAC\xEE\xAA\x69"
  },
  {
    /* 銬問 */
    /* 拿著火紅的鐵棒走向 */
    "fire", "\xBE\x52\xB0\xDD", "\xAE\xB3\xB5\xDB\xA4\xF5\xAC\xF5\xAA\xBA\xC5\x4B\xB4\xCE\xA8\xAB\xA6\x56", ""
  },
  {
    /* 原諒 */
    /* 接受 */
    /* 的道歉，原諒了他 */
    "forgive", "\xAD\xEC\xBD\xCC", "\xB1\xB5\xA8\xFC", "\xAA\xBA\xB9\x44\xBA\x70\xA1\x41\xAD\xEC\xBD\xCC\xA4\x46\xA5\x4C"
  },
  {
    /* 法式吻 */
    /* 把舌頭伸到 */
    /* 喉嚨裡～～～哇！一個浪漫的法國式深吻 */
    "french", "\xAA\x6B\xA6\xA1\xA7\x6B",	"\xA7\xE2\xA6\xDE\xC0\x59\xA6\xF9\xA8\xEC", "\xB3\xEF\xC4\x56\xB8\xCC\xA1\xE3\xA1\xE3\xA1\xE3\xAB\x7A\xA1\x49\xA4\x40\xAD\xD3\xAE\xF6\xBA\xA9\xAA\xBA\xAA\x6B\xB0\xEA\xA6\xA1\xB2\x60\xA7\x6B"
  },
  {
    /* 飛鳥 */
    /* 派出飛鳥一號向 */
    /* 衝過去 */
    "fuzzy", "\xAD\xB8\xB3\xBE", "\xAC\xA3\xA5\x58\xAD\xB8\xB3\xBE\xA4\x40\xB8\xB9\xA6\x56", "\xBD\xC4\xB9\x4C\xA5\x68"
  },
  {
    /* 縫嘴巴 */
    /* 把 */
    /*  的嘴巴用針縫起來 */
    "gag", "\xC1\x5F\xBC\x4C\xA4\xDA", "\xA7\xE2", " \xAA\xBA\xBC\x4C\xA4\xDA\xA5\xCE\xB0\x77\xC1\x5F\xB0\x5F\xA8\xD3"
  },
  {
    /* 傻笑 */
    /* 對著 */
    /* 傻傻的呆笑 */
    "giggle", "\xB6\xCC\xAF\xBA", "\xB9\xEF\xB5\xDB", "\xB6\xCC\xB6\xCC\xAA\xBA\xA7\x62\xAF\xBA"
  },
  {
    /* 瞪人 */
    /* 冷冷地瞪著 */
    "glare", "\xC0\xFC\xA4\x48", "\xA7\x4E\xA7\x4E\xA6\x61\xC0\xFC\xB5\xDB", ""
  },
  {
    /* 補心 */
    /* 用快乾把 */
    /* 的心黏了起來 */
    "glue", "\xB8\xC9\xA4\xDF", "\xA5\xCE\xA7\xD6\xB0\xAE\xA7\xE2", "\xAA\xBA\xA4\xDF\xC2\x48\xA4\x46\xB0\x5F\xA8\xD3"
  },
  {
    /* 告別 */
    /* 淚眼汪汪的向 */
    /* 告別 */
    "goodbye", "\xA7\x69\xA7\x4F", "\xB2\x5C\xB2\xB4\xA8\x4C\xA8\x4C\xAA\xBA\xA6\x56",	"\xA7\x69\xA7\x4F"
  },
  {
    /* 奸笑 */
    /* 對 */
    /* 露出邪惡的笑容 */
    "grin", "\xA6\x6C\xAF\xBA", "\xB9\xEF", "\xC5\x53\xA5\x58\xA8\xB8\xB4\x63\xAA\xBA\xAF\xBA\xAE\x65"
  },
  {
    /* 咆哮 */
    /* 對 */
    /* 咆哮不已 */
    "growl", "\xA9\x48\xAD\xFD", "\xB9\xEF", "\xA9\x48\xAD\xFD\xA4\xA3\xA4\x77"
  },
  {
    /* 握手 */
    /* 跟 */
    /* 握手 */
    "hand", "\xB4\xA4\xA4\xE2", "\xB8\xF2", "\xB4\xA4\xA4\xE2"
  },
  {
    /* 躲 */
    /* 躲在 */
    /* 背後 */
    "hide", "\xB8\xFA", "\xB8\xFA\xA6\x62", "\xAD\x49\xAB\xE1"
  },
  {
    /* 送醫院 */
    /* 把 */
    /* 送進醫院 */
    "hospital", "\xB0\x65\xC2\xE5\xB0\x7C", "\xA7\xE2", "\xB0\x65\xB6\x69\xC2\xE5\xB0\x7C"
  },
  {
    /* 昇龍拳 */
    /* 沉穩了身形，匯聚了內勁，對 */
    /* 使出了一記Ｈｏ--Ｒｙｕ--Ｋａｎ */
    "hrk", "\xAA\x40\xC0\x73\xAE\xB1", "\xA8\x49\xC3\xAD\xA4\x46\xA8\xAD\xA7\xCE\xA1\x41\xB6\xD7\xBB\x45\xA4\x46\xA4\xBA\xAB\x6C\xA1\x41\xB9\xEF", "\xA8\xCF\xA5\x58\xA4\x46\xA4\x40\xB0\x4F\xA2\xD6\xA2\xF7--\xA2\xE0\xA3\x42\xA2\xFD--\xA2\xD9\xA2\xE9\xA2\xF6"
  },
  {
    /* 熱擁 */
    /* 熱情的擁抱 */
    "hug", "\xBC\xF6\xBE\xD6", "\xBC\xF6\xB1\xA1\xAA\xBA\xBE\xD6\xA9\xEA", ""
  },
  {
    /* 催眠 */
    /* 拿著掛錶晃呀晃的，對 */
    /* 展開催眠 */
    "hypnoze", "\xB6\xCA\xAF\x76", "\xAE\xB3\xB5\xDB\xB1\xBE\xBF\xF6\xAE\xCC\xA7\x72\xAE\xCC\xAA\xBA\xA1\x41\xB9\xEF", "\xAE\x69\xB6\x7D\xB6\xCA\xAF\x76"
  },
  {
    /* 捅人 */
    /* 用力地捅著 */
    /* ，似乎對他很是不滿 */
    "jab", "\xD1\xB6\xA4\x48", "\xA5\xCE\xA4\x4F\xA6\x61\xD1\xB6\xB5\xDB", "\xA1\x41\xA6\xFC\xA5\x47\xB9\xEF\xA5\x4C\xAB\xDC\xAC\x4F\xA4\xA3\xBA\xA1"
  },
  {
    /* 柔道 */
    /* 抓住了 */
    /* 的衣襟，轉身……啊，是一記過肩摔 */
    "judo", "\xAC\x58\xB9\x44", "\xA7\xEC\xA6\xED\xA4\x46", "\xAA\xBA\xA6\xE7\xC3\xCC\xA1\x41\xC2\xE0\xA8\xAD\xA1\x4B\xA1\x4B\xB0\xDA\xA1\x41\xAC\x4F\xA4\x40\xB0\x4F\xB9\x4C\xAA\xD3\xBA\x4C"
  },
  {
    /* 踢人 */
    /* 把 */
    /* 踢得痛哭流涕 */
    "kick", "\xBD\xF0\xA4\x48", "\xA7\xE2", "\xBD\xF0\xB1\x6F\xB5\x68\xAD\xFA\xAC\x79\xAE\xF7"
  },
  {
    /* 砍人 */
    /* 把 */
    /* 亂刀砍死～～ */
    "kill", "\xAC\xE5\xA4\x48", "\xA7\xE2", "\xB6\xC3\xA4\x4D\xAC\xE5\xA6\xBA\xA1\xE3\xA1\xE3"
  },
  {
    /* 輕吻 */
    /* 輕吻 */
    /* 的臉頰 */
    "kiss", "\xBB\xB4\xA7\x6B", "\xBB\xB4\xA7\x6B", "\xAA\xBA\xC1\x79\xC0\x55"
  },
  {
    /* 嘲笑 */
    /* 大聲嘲笑 */
    "laugh", "\xBC\x4A\xAF\xBA", "\xA4\x6A\xC1\x6E\xBC\x4A\xAF\xBA", ""
  },
  {
    /* 給我 */
    /* 說：給我 */
    /* ！其餘免談！ */
    "levis", "\xB5\xB9\xA7\xDA", "\xBB\xA1\xA1\x47\xB5\xB9\xA7\xDA", "\xA1\x49\xA8\xE4\xBE\x6C\xA7\x4B\xBD\xCD\xA1\x49"
  },
  {
    /* 舔 */
    /* 狂舔 */
    "lick", "\xBB\x51", "\xA8\x67\xBB\x51", ""
  },
  {
    /* 聽 */
    /* 叫 */
    /* 閉嘴仔細聽 */
    "listen", "\xC5\xA5", "\xA5\x73", "\xB3\xAC\xBC\x4C\xA5\x4A\xB2\xD3\xC5\xA5"
  },
  {
    /* 壓制 */
    /* 施展逆蝦形固定，把 */
    /* 壓制在地板上 */
    "lobster", "\xC0\xA3\xA8\xEE", "\xAC\x49\xAE\x69\xB0\x66\xBD\xBC\xA7\xCE\xA9\x54\xA9\x77\xA1\x41\xA7\xE2", "\xC0\xA3\xA8\xEE\xA6\x62\xA6\x61\xAA\x4F\xA4\x57"
  },
  {
    /* 表白 */
    /* 對 */
    /* 深情的表白 */
    "love", "\xAA\xED\xA5\xD5", "\xB9\xEF", "\xB2\x60\xB1\xA1\xAA\xBA\xAA\xED\xA5\xD5"
  },
  {
    /* 打包 */
    /* 把 */
    /* 打包遞送到大陸 */
    "mail", "\xA5\xB4\xA5\x5D", "\xA7\xE2", "\xA5\xB4\xA5\x5D\xBB\xBC\xB0\x65\xA8\xEC\xA4\x6A\xB3\xB0"
  },
  {
    /* 求婚 */
    /* 捧著九百九十九朵玫瑰向 */
    /* 求婚 */
    "marry", "\xA8\x44\xB1\x42", "\xB1\xB7\xB5\xDB\xA4\x45\xA6\xCA\xA4\x45\xA4\x51\xA4\x45\xA6\xB7\xAA\xB4\xBA\xC0\xA6\x56", "\xA8\x44\xB1\x42"
  },
  {
    /* 早安 */
    /* 對 */
    /* 說『早安』 */
    "morning", "\xA6\xAD\xA6\x77", "\xB9\xEF", "\xBB\xA1\xA1\x79\xA6\xAD\xA6\x77\xA1\x7A"
  },
  {
    /* 午安 */
    /* 對 */
    /* 說『午安』 */
    "noon", "\xA4\xC8\xA6\x77", "\xB9\xEF", "\xBB\xA1\xA1\x79\xA4\xC8\xA6\x77\xA1\x7A"
  },
  {
    /* 點頭 */
    /* 向 */
    /* 點頭稱是 */
    "nod", "\xC2\x49\xC0\x59", "\xA6\x56", "\xC2\x49\xC0\x59\xBA\xD9\xAC\x4F"
  },
  {
    /* 頂肚子 */
    /* 用手肘頂 */
    /* 的肥肚子 */
    "nudge", "\xB3\xBB\xA8\x7B\xA4\x6C", "\xA5\xCE\xA4\xE2\xA8\x79\xB3\xBB", "\xAA\xBA\xAA\xCE\xA8\x7B\xA4\x6C"
  },
  {
    /* 拍肩膀 */
    /* 輕拍 */
    /* 的肩膀 */
    "pad", "\xA9\xE7\xAA\xD3\xBB\x48", "\xBB\xB4\xA9\xE7", "\xAA\xBA\xAA\xD3\xBB\x48"
  },
  {
    /* 平底鍋 */
    /* 從背後拿出了平底鍋，把 */
    /* 敲昏了 */
    "pan", "\xA5\xAD\xA9\xB3\xC1\xE7", "\xB1\x71\xAD\x49\xAB\xE1\xAE\xB3\xA5\x58\xA4\x46\xA5\xAD\xA9\xB3\xC1\xE7\xA1\x41\xA7\xE2", "\xBA\x56\xA9\xFC\xA4\x46"
  },
  {
    /* 拍頭 */
    /* 拍拍 */
    /* 的頭 */
    "pat", "\xA9\xE7\xC0\x59", "\xA9\xE7\xA9\xE7", "\xAA\xBA\xC0\x59"
  },
  {
    /* 撒嬌 */
    /* 跟 */
    /* 嗲聲嗲氣地撒嬌 */
    "pettish", "\xBC\xBB\xBC\x62", "\xB8\xF2", "\xDC\xDD\xC1\x6E\xDC\xDD\xAE\xF0\xA6\x61\xBC\xBB\xBC\x62"
  },
  {
    /* 霹靂 */
    /* 使出 君子風 天地根	般若懺 三式合一打向 */
    /* ～～ */
    "pili", "\xC5\x52\xC6\x45", "\xA8\xCF\xA5\x58 \xA7\x67\xA4\x6C\xAD\xB7 \xA4\xD1\xA6\x61\xAE\xDA	\xAF\xEB\xAD\x59\xC4\x62 \xA4\x54\xA6\xA1\xA6\x58\xA4\x40\xA5\xB4\xA6\x56", "\xA1\xE3\xA1\xE3"
  },
  {
    /* 擰人 */
    /* 用力的把 */
    /* 擰得黑青 */
    "pinch", "\xC0\xBE\xA4\x48", "\xA5\xCE\xA4\x4F\xAA\xBA\xA7\xE2", "\xC0\xBE\xB1\x6F\xB6\xC2\xAB\x43"
  },
  {
    /* 戳弄 */
    /* 戳了戳 */
    /* 想要引起他的注意 */
    "poke", "\xC2\x57\xA7\xCB", "\xC2\x57\xA4\x46\xC2\x57", "\xB7\x51\xAD\x6E\xA4\xDE\xB0\x5F\xA5\x4C\xAA\xBA\xAA\x60\xB7\x4E"
  },
  {
    /* 灌布丁 */
    /* 對 */
    /* 灌了一卡車布丁 */
    "puding", "\xC4\xE9\xA5\xAC\xA4\x42",	"\xB9\xEF", "\xC4\xE9\xA4\x46\xA4\x40\xA5\x64\xA8\xAE\xA5\xAC\xA4\x42"
  },
  {
    /* 打滾 */
    /* 放出多爾袞的音樂， */
    /* 在地上滾來滾去 */
    "roll", "\xA5\xB4\xBA\x75", "\xA9\xF1\xA5\x58\xA6\x68\xBA\xB8\xB3\x4F\xAA\xBA\xAD\xB5\xBC\xD6\xA1\x41", "\xA6\x62\xA6\x61\xA4\x57\xBA\x75\xA8\xD3\xBA\x75\xA5\x68"
  },
  {
    /* 保護 */
    /* 誓死保護著 */
    "protect", "\xAB\x4F\xC5\x40", "\xBB\x7D\xA6\xBA\xAB\x4F\xC5\x40\xB5\xDB", ""
  },
  {
    /* 拉 */
    /* 死命地拉住 */
    /* 不放 */
    "pull", "\xA9\xD4", "\xA6\xBA\xA9\x52\xA6\x61\xA9\xD4\xA6\xED",	"\xA4\xA3\xA9\xF1"
  },
  {
    /* 揍人 */
    /* 狠狠揍了 */
    /* 一頓 */
    "punch", "\xB4\x7E\xA4\x48", "\xAC\xBD\xAC\xBD\xB4\x7E\xA4\x46", "\xA4\x40\xB9\x79"
  },
  {
    /* 耍賴 */
    /* 跟 */
    /* 耍賴 */
    "rascal", "\xAD\x41\xBF\xE0", "\xB8\xF2", "\xAD\x41\xBF\xE0"
  },
  {
    /* 入懷 */
    /* 鑽到 */
    /* 的懷裡睡著了…… */
    "recline", "\xA4\x4A\xC3\x68", "\xC6\x70\xA8\xEC", "\xAA\xBA\xC3\x68\xB8\xCC\xBA\xCE\xB5\xDB\xA4\x46\xA1\x4B\xA1\x4B"
  },
  {
    /* 回收桶 */
    /* 把 */
    /* 丟到資源回收桶 */
    "recycle", "\xA6\x5E\xA6\xAC\xB1\xED", "\xA7\xE2", "\xA5\xE1\xA8\xEC\xB8\xEA\xB7\xBD\xA6\x5E\xA6\xAC\xB1\xED"
  },
  {
    /* 負責 */
    /* 安慰 */
    /* 說：『不要哭，我會負責的』 */
    "respond", "\xAD\x74\xB3\x64", "\xA6\x77\xBC\xA2", "\xBB\xA1\xA1\x47\xA1\x79\xA4\xA3\xAD\x6E\xAD\xFA\xA1\x41\xA7\xDA\xB7\x7C\xAD\x74\xB3\x64\xAA\xBA\xA1\x7A"
  },
  {
    /* 磨爪 */
    /* 撿起 */
    /* 身邊的石子磨磨自己的利爪 */
    "scratch", "\xBF\x69\xA4\xF6", "\xBE\xDF\xB0\x5F", "\xA8\xAD\xC3\xE4\xAA\xBA\xA5\xDB\xA4\x6C\xBF\x69\xBF\x69\xA6\xDB\xA4\x76\xAA\xBA\xA7\x51\xA4\xF6"
  },
  {
    /* 性騷擾 */
    /* 對 */
    /* 性騷擾 */
    "sex", "\xA9\xCA\xC4\xCC\xC2\x5A", "\xB9\xEF", "\xA9\xCA\xC4\xCC\xC2\x5A"
  },
  {
    /* 雪特 */
    /* 對 */
    /* 罵了一聲『雪特』 */
    "shit", "\xB3\xB7\xAF\x53", "\xB9\xEF", "\xBD\x7C\xA4\x46\xA4\x40\xC1\x6E\xA1\x79\xB3\xB7\xAF\x53\xA1\x7A"
  },
  {
    /* 聳肩 */
    /* 無奈地向 */
    /* 聳了聳肩膀 */
    "shrug", "\xC1\x71\xAA\xD3", "\xB5\x4C\xA9\x60\xA6\x61\xA6\x56", "\xC1\x71\xA4\x46\xC1\x71\xAA\xD3\xBB\x48"
  },
  {
    /* 歎氣 */
    /* 對 */
    /* 歎了一口氣 */
    "sigh", "\xBC\xDB\xAE\xF0", "\xB9\xEF", "\xBC\xDB\xA4\x46\xA4\x40\xA4\x66\xAE\xF0"
  },
  {
    /* 打耳光 */
    /* 啪啪的巴了 */
    /* 一頓耳光 */
    "slap", "\xA5\xB4\xA6\xD5\xA5\xFA", "\xB0\xD4\xB0\xD4\xAA\xBA\xA4\xDA\xA4\x46", "\xA4\x40\xB9\x79\xA6\xD5\xA5\xFA"
  },
  {
    /* 擁吻 */
    /* 擁吻著 */
    "smooch", "\xBE\xD6\xA7\x6B", "\xBE\xD6\xA7\x6B\xB5\xDB",	""
  },
  {
    /* 竊笑 */
    /* 嘿嘿嘿地對 */
    /* 竊笑 */
    "snicker", "\xC5\xD1\xAF\xBA", "\xBC\x4B\xBC\x4B\xBC\x4B\xA6\x61\xB9\xEF", "\xC5\xD1\xAF\xBA"
  },
  {
    /* 不屑 */
    /* 對 */
    /* 嗤之以鼻 */
    "sniff", "\xA4\xA3\xAE\x68", "\xB9\xEF", "\xB6\xE1\xA4\xA7\xA5\x48\xBB\xF3"
  },
  {
    /* 對不起 */
    /* 向 */
    /* 說對不起！我對不起大家，我對不起國家社會 */
    "sorry", "\xB9\xEF\xA4\xA3\xB0\x5F", "\xA6\x56", "\xBB\xA1\xB9\xEF\xA4\xA3\xB0\x5F\xA1\x49\xA7\xDA\xB9\xEF\xA4\xA3\xB0\x5F\xA4\x6A\xAE\x61\xA1\x41\xA7\xDA\xB9\xEF\xA4\xA3\xB0\x5F\xB0\xEA\xAE\x61\xAA\xC0\xB7\x7C"
  },
  {
    /* 打屁屁 */
    /* 用巴掌打 */
    /* 的臀部 */
    "spank", "\xA5\xB4\xA7\xBE\xA7\xBE", "\xA5\xCE\xA4\xDA\xB4\x78\xA5\xB4", "\xAA\xBA\xC1\x76\xB3\xA1"
  },
  {
    /* 緊擁 */
    /* 緊緊地擁抱著 */
    "squeeze", "\xBA\xF2\xBE\xD6", "\xBA\xF2\xBA\xF2\xA6\x61\xBE\xD6\xA9\xEA\xB5\xDB", ""
  },
  {
    /* 感謝 */
    /* 向 */
    /* 感謝得五體投地 */
    "thank", "\xB7\x50\xC1\xC2", "\xA6\x56", "\xB7\x50\xC1\xC2\xB1\x6F\xA4\xAD\xC5\xE9\xA7\xEB\xA6\x61"
  },
  {
    /* 丟擲 */
    /* 拿了腳下一塊大石頭朝 */
    /* 那丟了過去 */
    "throw", "\xA5\xE1\xC2\x59", "\xAE\xB3\xA4\x46\xB8\x7D\xA4\x55\xA4\x40\xB6\xF4\xA4\x6A\xA5\xDB\xC0\x59\xB4\xC2", "\xA8\xBA\xA5\xE1\xA4\x46\xB9\x4C\xA5\x68"
  },
  {
    /* 搔癢 */
    /* 咕嘰咕嘰，搔 */
    /* 的癢 */
    "tickle", "\xB7\x6B\xC4\x6F", "\xA9\x42\xBC\x54\xA9\x42\xBC\x54\xA1\x41\xB7\x6B", "\xAA\xBA\xC4\x6F"
  },
  {
    /* 等一下 */
    /* 叫 */
    /* 等一下哦！ */
    "wait", "\xB5\xA5\xA4\x40\xA4\x55", "\xA5\x73", "\xB5\xA5\xA4\x40\xA4\x55\xAE\x40\xA1\x49"
  },
  {
    /* 搖醒 */
    /* 輕輕地把 */
    /* 搖醒 */
    "wake", "\xB7\x6E\xBF\xF4", "\xBB\xB4\xBB\xB4\xA6\x61\xA7\xE2",	"\xB7\x6E\xBF\xF4"
  },
  {
    /* 揮手 */
    /* 對著 */
    /* 揮揮手，表示告別之意 */
    "wave", "\xB4\xA7\xA4\xE2", "\xB9\xEF\xB5\xDB", "\xB4\xA7\xB4\xA7\xA4\xE2\xA1\x41\xAA\xED\xA5\xDC\xA7\x69\xA7\x4F\xA4\xA7\xB7\x4E"
  },
  {
    /* 歡迎 */
    /* 歡迎 */
    /* 進來八卦一下 */
    "welcome", "\xC5\x77\xAA\xEF", "\xC5\x77\xAA\xEF", "\xB6\x69\xA8\xD3\xA4\x4B\xA8\xF6\xA4\x40\xA4\x55"
  },
  {
    /* 什麼 */
    /* 說：『 */
    /* 哩公瞎密哇隴聽某?？?﹖?』 */
    "what", "\xA4\xB0\xBB\xF2", "\xBB\xA1\xA1\x47\xA1\x79", "\xAD\xF9\xA4\xBD\xBD\x4D\xB1\x4B\xAB\x7A\xC3\xF7\xC5\xA5\xAC\x59?\xA1\x48?\xA1\x53?\xA1\x7A"
  },
  {
    /* 鞭笞 */
    /* 手上拿著蠟燭，用鞭子痛打 */
    "whip", "\xC3\x40\xB2\xC7", "\xA4\xE2\xA4\x57\xAE\xB3\xB5\xDB\xC4\xFA\xC0\xEB\xA1\x41\xA5\xCE\xC3\x40\xA4\x6C\xB5\x68\xA5\xB4",	""
  },
  {
    /* 扭屁股 */
    /* 對著 */
    /* 扭屁股 */
    "wiggle", "\xA7\xE1\xA7\xBE\xAA\xD1",	"\xB9\xEF\xB5\xDB",	"\xA7\xE1\xA7\xBE\xAA\xD1"
  },
  {
    /* 眨眼 */
    /* 對 */
    /* 神秘的眨眨眼睛 */
    "wink", "\xAF\x77\xB2\xB4", "\xB9\xEF", "\xAF\xAB\xAF\xB5\xAA\xBA\xAF\x77\xAF\x77\xB2\xB4\xB7\xFA"
  },
  {
    /* 猛攻 */
    /* 對 */
    /* 瘋狂的攻擊 */
    "zap", "\xB2\x72\xA7\xF0", "\xB9\xEF", "\xBA\xC6\xA8\x67\xAA\xBA\xA7\xF0\xC0\xBB"
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
party_action(cu, cmd, party)
  ChatUser *cu;
  char *cmd;
  char *party;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(party_data, ACTNUM_PARTY, cmd)))
  {
    if (*party == '\0')
    {
      /* 大家 */
      party = "\xA4\x6A\xAE\x61";
    }
    else
    {
      ChatUser *xuser;

      xuser = fuzzy_cuser_by_chatid(party);
      if (xuser == NULL)
      {			/* Thor.980724: 用 userid也嘛通 */
	xuser = cuser_by_userid(party);
      }

      if (xuser == NULL)
      {
	sprintf(buf, msg_no_such_id, party);
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	return 0;
      }
      else if (xuser == FUZZY_USER)
      {
	/* ※ 請指明聊天代號 */
	send_to_user(cu, "\xA1\xB0 \xBD\xD0\xAB\xFC\xA9\xFA\xB2\xE1\xA4\xD1\xA5\x4E\xB8\xB9", 0, MSG_MESSAGE);
	return 0;
      }
      else if (cu->room != xuser->room || CLOAK(xuser))
      {
	sprintf(buf, msg_not_here, party);
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	return 0;
      }
      else
      {
	party = xuser->chatid;
      }
    }
    sprintf(buf, "\033[1;32m%s \033[31m%s\033[33m %s \033[31m%s\033[m",
      cu->chatid, cap->part1_msg, party, cap->part2_msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 0;			/* Thor: cu->room 是否為 NULL? */
  }
  return 1;
}


/* --------------------------------------------- */
/* MUD-like social commands : speak              */
/* --------------------------------------------- */


/* itoc.010805.註解:  //ask 大家今天過得好嗎？	 itoc 問大家今天過得好嗎？*/

#define ACTNUM_SPEAK	29

static ChatAction speak_data[ACTNUM_SPEAK] =
{
  {
    /* 詢問 */
    /* 問 */
    "ask", "\xB8\xDF\xB0\xDD", "\xB0\xDD", NULL
  },
  {
    /* 廣播 */
    /* 廣播 */
    "broadcast", "\xBC\x73\xBC\xBD", "\xBC\x73\xBC\xBD", NULL
  },
  {
    /* 歌頌 */
    /* 高聲歌頌 */
    "chant", "\xBA\x71\xB9\x7C", "\xB0\xAA\xC1\x6E\xBA\x71\xB9\x7C", NULL
  },
  {
    /* 喝采 */
    /* 喝采 */
    "cheer", "\xB3\xDC\xAA\xF6", "\xB3\xDC\xAA\xF6", NULL
  },
  {
    /* 輕笑 */
    /* 輕笑 */
    "chuckle", "\xBB\xB4\xAF\xBA", "\xBB\xB4\xAF\xBA", NULL
  },
  {
    /* 詛咒 */
    /* 暗幹 */
    "curse", "\xB6\x41\xA9\x47", "\xB7\x74\xB7\x46", NULL
  },
  {
    /* 要求 */
    /* 要求 */
    "demand", "\xAD\x6E\xA8\x44", "\xAD\x6E\xA8\x44", NULL
  },
  {
    /* 公幹 */
    /* 公幹 */
    "fuck", "\xA4\xBD\xB7\x46", "\xA4\xBD\xB7\x46", NULL
  },
  {
    /* 呻吟 */
    /* 呻吟 */
    "groan", "\xA9\x44\xA7\x75", "\xA9\x44\xA7\x75", NULL
  },
  {
    /* 發牢騷 */
    /* 發牢騷 */
    "grumble", "\xB5\x6F\xA8\x63\xC4\xCC", "\xB5\x6F\xA8\x63\xC4\xCC", NULL
  },
  {
    /* 彈唱 */
    /* 邊彈著吉他，邊唱著 */
    "guitar", "\xBC\x75\xB0\xDB", "\xC3\xE4\xBC\x75\xB5\xDB\xA6\x4E\xA5\x4C\xA1\x41\xC3\xE4\xB0\xDB\xB5\xDB", NULL
  },
  {
    /* 喃喃 */
    /* 喃喃自語 */
    "hum", "\xB3\xE4\xB3\xE4", "\xB3\xE4\xB3\xE4\xA6\xDB\xBB\x79", NULL
  },
  {
    /* 怨嘆 */
    /* 怨嘆 */
    "moan", "\xAB\xE8\xB9\xC4", "\xAB\xE8\xB9\xC4", NULL
  },
  {
    /* 強調 */
    /* 強調 */
    "notice", "\xB1\x6A\xBD\xD5", "\xB1\x6A\xBD\xD5", NULL
  },
  {
    /* 命令 */
    /* 命令 */
    "order", "\xA9\x52\xA5\x4F", "\xA9\x52\xA5\x4F", NULL
  },
  {
    /* 沈思 */
    /* 沈思 */
    "ponder", "\xA8\x48\xAB\xE4", "\xA8\x48\xAB\xE4", NULL
  },
  {
    /* 噘嘴 */
    /* 噘著嘴說 */
    "pout", "\xE4\xFE\xBC\x4C", "\xE4\xFE\xB5\xDB\xBC\x4C\xBB\xA1",	NULL
  },
  {
    /* 祈禱 */
    /* 祈禱 */
    "pray", "\xAC\xE8\xC3\xAB", "\xAC\xE8\xC3\xAB", NULL
  },
  {
    /* 懇求 */
    /* 懇求 */
    "request", "\xC0\xB5\xA8\x44", "\xC0\xB5\xA8\x44", NULL
  },
  {
    /* 大罵 */
    /* 大罵 */
    "shout", "\xA4\x6A\xBD\x7C", "\xA4\x6A\xBD\x7C", NULL
  },
  {
    /* 唱歌 */
    /* 唱歌 */
    "sing", "\xB0\xDB\xBA\x71", "\xB0\xDB\xBA\x71", NULL
  },
  {
    /* 微笑 */
    /* 微笑 */
    "smile", "\xB7\x4C\xAF\xBA", "\xB7\x4C\xAF\xBA", NULL
  },
  {
    /* 假笑 */
    /* 假笑 */
    "smirk", "\xB0\xB2\xAF\xBA", "\xB0\xB2\xAF\xBA", NULL
  },
  {
    /* 發誓 */
    /* 發誓 */
    "swear", "\xB5\x6F\xBB\x7D", "\xB5\x6F\xBB\x7D", NULL
  },
  {
    /* 嘲笑 */
    /* 嘲笑 */
    "tease", "\xBC\x4A\xAF\xBA", "\xBC\x4A\xAF\xBA", NULL
  },
  {
    /* 嗚咽 */
    /* 嗚咽的說 */
    "whimper", "\xB6\xE3\xAB\x7C", "\xB6\xE3\xAB\x7C\xAA\xBA\xBB\xA1", NULL
  },
  {
    /* 哈欠 */
    /* 邊打哈欠邊說 */
    "yawn", "\xAB\xA2\xA4\xED", "\xC3\xE4\xA5\xB4\xAB\xA2\xA4\xED\xC3\xE4\xBB\xA1", NULL
  },
  {
    /* 大喊 */
    /* 大喊 */
    "yell", "\xA4\x6A\xB3\xDB", "\xA4\x6A\xB3\xDB", NULL
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
speak_action(cu, cmd, msg)
  ChatUser *cu;
  char *cmd;
  char *msg;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(speak_data, ACTNUM_SPEAK, cmd)))
  {
    /* \033[1;32m%s \033[31m%s：\033[33m %s\033[m */
    sprintf(buf, "\033[1;32m%s \033[31m%s\xA1\x47\033[33m %s\033[m",
      cu->chatid, cap->part1_msg, msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 0;
  }
  return 1;
}


/* ----------------------------------------------------- */
/* MUD-like social commands : condition			 */
/* ----------------------------------------------------- */


/* itoc.010805.註解:  //agree	itoc 深表同意 */

#define ACTNUM_CONDITION	73

static ChatAction condition_data[ACTNUM_CONDITION] =
{
  {
    /* 同意 */
    /* 深表同意 */
    "agree", "\xA6\x50\xB7\x4E", "\xB2\x60\xAA\xED\xA6\x50\xB7\x4E", NULL
  },
  {
    /* 靈光 */
    /* 苦思良久，忽然靈光一現，不禁呀哈的一聲 */
    "aha", "\xC6\x46\xA5\xFA", "\xAD\x57\xAB\xE4\xA8\x7D\xA4\x5B\xA1\x41\xA9\xBF\xB5\x4D\xC6\x46\xA5\xFA\xA4\x40\xB2\x7B\xA1\x41\xA4\xA3\xB8\x54\xA7\x72\xAB\xA2\xAA\xBA\xA4\x40\xC1\x6E", NULL
  },
  {
    /* 插腰 */
    /* 又氣又無奈的兩手插腰 */
    "akimbo", "\xB4\xA1\xB8\x79", "\xA4\x53\xAE\xF0\xA4\x53\xB5\x4C\xA9\x60\xAA\xBA\xA8\xE2\xA4\xE2\xB4\xA1\xB8\x79", NULL
  },
  {
    /* 哎呀 */
    /* 哎呀呀～ */
    "alas", "\xAB\x75\xA7\x72", "\xAB\x75\xA7\x72\xA7\x72\xA1\xE3", NULL
  },
  {
    /* 拍手 */
    /* 啪啪啪啪啪……啪啪 */
    "applaud", "\xA9\xE7\xA4\xE2", "\xB0\xD4\xB0\xD4\xB0\xD4\xB0\xD4\xB0\xD4\xA1\x4B\xA1\x4B\xB0\xD4\xB0\xD4", NULL
  },
  {
    /* 害羞 */
    /* 害羞地轉開視線 */
    "avert", "\xAE\x60\xB2\xDB", "\xAE\x60\xB2\xDB\xA6\x61\xC2\xE0\xB6\x7D\xB5\xF8\xBD\x75", NULL
  },
  {
    /* 唉呦喂 */
    /* 唉呦喂～ */
    "ayo", "\xAD\xFC\xCB\xE7\xB3\xDE", "\xAD\xFC\xCB\xE7\xB3\xDE\xA1\xE3", NULL
  },
  {
    /* 坐回來 */
    /* 回來坐正繼續奮戰 */
    "back", "\xA7\xA4\xA6\x5E\xA8\xD3", "\xA6\x5E\xA8\xD3\xA7\xA4\xA5\xBF\xC4\x7E\xC4\xF2\xBE\xC4\xBE\xD4", NULL
  },
  {
    /* 在血中 */
    /* 倒在血泊之中 */
    "blood", "\xA6\x62\xA6\xE5\xA4\xA4", "\xAD\xCB\xA6\x62\xA6\xE5\xAA\x79\xA4\xA7\xA4\xA4", NULL
  },
  {
    /* 臉紅 */
    /* 臉都紅了 */
    "blush", "\xC1\x79\xAC\xF5", "\xC1\x79\xB3\xA3\xAC\xF5\xA4\x46", NULL
  },
  {
    /* 心碎 */
    /* 的心破碎成一片一片的 */
    "broke", "\xA4\xDF\xB8\x48", "\xAA\xBA\xA4\xDF\xAF\x7D\xB8\x48\xA6\xA8\xA4\x40\xA4\xF9\xA4\x40\xA4\xF9\xAA\xBA", NULL
  },
  {
    /* 臭蟲 */
    /* 發現這系統有Ｂｕｇ～ */
    "bug", "\xAF\xE4\xC2\xCE", "\xB5\x6F\xB2\x7B\xB3\x6F\xA8\x74\xB2\xCE\xA6\xB3\xA2\xD0\xA2\xFD\xA2\xEF\xA1\xE3", NULL
  },
  {
    /* 沒人理 */
    /* 嗚～～都沒有人理我 ：～ */
    "careles", "\xA8\x53\xA4\x48\xB2\x7A", "\xB6\xE3\xA1\xE3\xA1\xE3\xB3\xA3\xA8\x53\xA6\xB3\xA4\x48\xB2\x7A\xA7\xDA \xA1\x47\xA1\xE3", NULL
  },
  {
    /* 嗑瓜子 */
    /* 很悠閒的嗑起瓜子來了 */
    "chew", "\xB6\xDF\xA5\xCA\xA4\x6C", "\xAB\xDC\xB1\x79\xB6\xA2\xAA\xBA\xB6\xDF\xB0\x5F\xA5\xCA\xA4\x6C\xA8\xD3\xA4\x46", NULL
  },
  {
    /* 爬山 */
    /* 自己慢慢爬上山來…… */
    "climb", "\xAA\xA6\xA4\x73", "\xA6\xDB\xA4\x76\xBA\x43\xBA\x43\xAA\xA6\xA4\x57\xA4\x73\xA8\xD3\xA1\x4B\xA1\x4B", NULL
  },
  {
    /* 感冒 */
    /* 感冒了，媽媽不讓我出去玩 ：（ */
    "cold", "\xB7\x50\xAB\x5F", "\xB7\x50\xAB\x5F\xA4\x46\xA1\x41\xB6\xFD\xB6\xFD\xA4\xA3\xC5\xFD\xA7\xDA\xA5\x58\xA5\x68\xAA\xB1 \xA1\x47\xA1\x5D", NULL
  },
  {
    /* 咳嗽 */
    /* 咳了幾聲 */
    "cough", "\xAB\x79\xB9\xC2", "\xAB\x79\xA4\x46\xB4\x58\xC1\x6E", NULL
  },
  {
    /* 當機 */
    /* 嗚… */
    /* 當機了 */
    "crash", "\xB7\xED\xBE\xF7", "\xB6\xE3\xA1\x4B" BBSNAME "\xB7\xED\xBE\xF7\xA4\x46", NULL
  },
  {
    /* 暴斃 */
    /* 當場暴斃 */
    "die", "\xBC\xC9\xC0\xC5", "\xB7\xED\xB3\xF5\xBC\xC9\xC0\xC5", NULL
  },
  {
    /* 潛水 */
    /* 跳到水裡躲起來 */
    "dive", "\xBC\xE7\xA4\xF4", "\xB8\xF5\xA8\xEC\xA4\xF4\xB8\xCC\xB8\xFA\xB0\x5F\xA8\xD3", NULL
  },
  {
    /* 昏倒 */
    /* 當場昏倒 */
    "faint", "\xA9\xFC\xAD\xCB", "\xB7\xED\xB3\xF5\xA9\xFC\xAD\xCB", NULL
  },
  {
    /* 放屁 */
    /* 全是在放屁，胡扯一通！ */
    "fart", "\xA9\xF1\xA7\xBE", "\xA5\xFE\xAC\x4F\xA6\x62\xA9\xF1\xA7\xBE\xA1\x41\xAD\x4A\xA7\xE8\xA4\x40\xB3\x71\xA1\x49", NULL
  },
  {
    /* 香蕉皮 */
    /* 踩到香蕉皮…滑倒！ */
    "flop", "\xAD\xBB\xBF\xBC\xA5\xD6", "\xBD\xF2\xA8\xEC\xAD\xBB\xBF\xBC\xA5\xD6\xA1\x4B\xB7\xC6\xAD\xCB\xA1\x49", NULL
  },
  {
    /* 飄飄然 */
    /* 飄飄然地，好似飛了起來 */
    "fly", "\xC4\xC6\xC4\xC6\xB5\x4D", "\xC4\xC6\xC4\xC6\xB5\x4D\xA6\x61\xA1\x41\xA6\x6E\xA6\xFC\xAD\xB8\xA4\x46\xB0\x5F\xA8\xD3", NULL
  },
  {
    /* 蹙眉 */
    /* 蹙眉，不知為了什麼 */
    "frown", "\xC2\xD9\xAC\xDC", "\xC2\xD9\xAC\xDC\xA1\x41\xA4\xA3\xAA\xBE\xAC\xB0\xA4\x46\xA4\xB0\xBB\xF2", NULL
  },
  {
    /* 拿金牌 */
    /* 唱著：『金ㄍㄠˊ金ㄍㄠˊ出國比賽，得冠軍，拿金牌，光榮倒鄧來！』 */
    "gold", "\xAE\xB3\xAA\xF7\xB5\x50", "\xB0\xDB\xB5\xDB\xA1\x47\xA1\x79\xAA\xF7\xA3\x7C\xA3\xB1\xA3\xBD\xAA\xF7\xA3\x7C\xA3\xB1\xA3\xBD\xA5\x58\xB0\xEA\xA4\xF1\xC1\xC9\xA1\x41\xB1\x6F\xAB\x61\xAD\x78\xA1\x41\xAE\xB3\xAA\xF7\xB5\x50\xA1\x41\xA5\xFA\xBA\x61\xAD\xCB\xBE\x48\xA8\xD3\xA1\x49\xA1\x7A", NULL
  },
  {
    /* 肚子餓 */
    /* 的肚子發出咕嚕咕嚕～的聲音 */
    "gulu", "\xA8\x7B\xA4\x6C\xBE\x6A", "\xAA\xBA\xA8\x7B\xA4\x6C\xB5\x6F\xA5\x58\xA9\x42\xC2\x50\xA9\x42\xC2\x50\xA1\xE3\xAA\xBA\xC1\x6E\xAD\xB5", NULL
  },
  {
    /* 哈哈 */
    /* 哇哈哈哈…大笑了起來 */
    "haha", "\xAB\xA2\xAB\xA2", "\xAB\x7A\xAB\xA2\xAB\xA2\xAB\xA2\xA1\x4B\xA4\x6A\xAF\xBA\xA4\x46\xB0\x5F\xA8\xD3", NULL
  },
  {
    /* 高興 */
    /* 高興得在地上打滾 */
    "happy", "\xB0\xAA\xBF\xB3", "\xB0\xAA\xBF\xB3\xB1\x6F\xA6\x62\xA6\x61\xA4\x57\xA5\xB4\xBA\x75", NULL
  },
  {
    /* 打嗝 */
    /* 打嗝個不停 */
    "hiccup", "\xA5\xB4\xDC\xD0", "\xA5\xB4\xDC\xD0\xAD\xD3\xA4\xA3\xB0\xB1", NULL
  },
  {
    /* 呵呵 */
    /* 呵呵呵笑個不停 */
    "hoho", "\xA8\xFE\xA8\xFE", "\xA8\xFE\xA8\xFE\xA8\xFE\xAF\xBA\xAD\xD3\xA4\xA3\xB0\xB1", NULL
  },
  {
    /* 被催眠 */
    /* 眼神呆滯，被催眠了……ｚＺｚzzz */
    "hypnzed", "\xB3\x51\xB6\xCA\xAF\x76", "\xB2\xB4\xAF\xAB\xA7\x62\xBA\xA2\xA1\x41\xB3\x51\xB6\xCA\xAF\x76\xA4\x46\xA1\x4B\xA1\x4B\xA3\x43\xA2\xE8\xA3\x43zzz", NULL
  },
  {
    /* 呆住 */
    /* 瞬間呆住了 */
    "idle", "\xA7\x62\xA6\xED", "\xC0\xFE\xB6\xA1\xA7\x62\xA6\xED\xA4\x46", NULL
  },
  {
    /* 痞子 */
    /* 痞子般的晃來晃去 */
    "jacky", "\xB5\x6C\xA4\x6C", "\xB5\x6C\xA4\x6C\xAF\xEB\xAA\xBA\xAE\xCC\xA8\xD3\xAE\xCC\xA5\x68", NULL
  },
  {
    /* 吃醋 */
    /* 氣鼓鼓地喝了一缸醋 */
    "jealous", "\xA6\x59\xBE\x4C", "\xAE\xF0\xB9\xAA\xB9\xAA\xA6\x61\xB3\xDC\xA4\x46\xA4\x40\xAC\xFB\xBE\x4C", NULL
  },
  {
    /* 跳樓 */
    /* 跳樓自殺 */
    "jump", "\xB8\xF5\xBC\xD3", "\xB8\xF5\xBC\xD3\xA6\xDB\xB1\xFE",	NULL
  },
  {
    /* 幸運 */
    /* 哇！福氣啦！ */
    "luck", "\xA9\xAF\xB9\x42", "\xAB\x7A\xA1\x49\xBA\xD6\xAE\xF0\xB0\xD5\xA1\x49", NULL
  },
  {
    /* 一種舞 */
    /* 開始跳起了ＭａＣａＲｅＮａ～～～～ */
    "macarn", "\xA4\x40\xBA\xD8\xBB\x52",	"\xB6\x7D\xA9\x6C\xB8\xF5\xB0\x5F\xA4\x46\xA2\xDB\xA2\xE9\xA2\xD1\xA2\xE9\xA2\xE0\xA2\xED\xA2\xDC\xA2\xE9\xA1\xE3\xA1\xE3\xA1\xE3\xA1\xE3", NULL
  },
  {
    /* 喵喵 */
    /* 喵喵口苗口苗～～～～～ */
    "miou", "\xD8\x70\xD8\x70", "\xD8\x70\xD8\x70\xA4\x66\xAD\x5D\xA4\x66\xAD\x5D\xA1\xE3\xA1\xE3\xA1\xE3\xA1\xE3\xA1\xE3", NULL
  },
  {
    /* 賺錢 */
    /* 埋首研究怎樣賺大錢 */
    "money", "\xC1\xC8\xBF\xFA", "\xAE\x49\xAD\xBA\xAC\xE3\xA8\x73\xAB\xE7\xBC\xCB\xC1\xC8\xA4\x6A\xBF\xFA", NULL
  },
  {
    /* 扁嘴 */
    /* 扁嘴中！ */
    "mouth", "\xAB\xF3\xBC\x4C", "\xAB\xF3\xBC\x4C\xA4\xA4\xA1\x49", NULL
  },
  {
    /* 低咕 */
    /* 低聲咕噥著某些事。 */
    "mutter", "\xA7\x43\xA9\x42", "\xA7\x43\xC1\x6E\xA9\x42\xBE\xBA\xB5\xDB\xAC\x59\xA8\xC7\xA8\xC6\xA1\x43", NULL
  },
  {
    /* 怎麼會 */
    /* ：奈ㄝ啊捏?? */
    "nani", "\xAB\xE7\xBB\xF2\xB7\x7C", "\xA1\x47\xA9\x60\xA3\xAE\xB0\xDA\xAE\xBA??", NULL
  },
  {
    /* 流鼻血 */
    /* 流鼻血 */
    "nose", "\xAC\x79\xBB\xF3\xA6\xE5", "\xAC\x79\xBB\xF3\xA6\xE5",	NULL
  },
  {
    /* 嘔吐 */
    /* 嘔吐中 */
    "puke", "\xB9\xC3\xA6\x52", "\xB9\xC3\xA6\x52\xA4\xA4", NULL
  },
  {
    /* 休息 */
    /* 休息中，請勿打擾 */
    "rest", "\xA5\xF0\xAE\xA7", "\xA5\xF0\xAE\xA7\xA4\xA4\xA1\x41\xBD\xD0\xA4\xC5\xA5\xB4\xC2\x5A",	NULL
  },
  {
    /* 翻肚 */
    /* 翻肚 */
    "reverse", "\xC2\xBD\xA8\x7B", "\xC2\xBD\xA8\x7B", NULL
  },
  {
    /* 開房間 */
    /* r-o-O-m-r-O-Ｏ-Mmm-rRＲ........ */
    "room", "\xB6\x7D\xA9\xD0\xB6\xA1", "r-o-O-m-r-O-\xA2\xDD-Mmm-rR\xA2\xE0........", NULL
  },
  {
    /* 尖叫 */
    /* 大聲尖叫！ 啊~~~~~~~ */
    "scream", "\xA6\x79\xA5\x73", "\xA4\x6A\xC1\x6E\xA6\x79\xA5\x73\xA1\x49 \xB0\xDA~~~~~~~", NULL
  },
  {
    /* 搖頭 */
    /* 搖了搖頭 */
    "shake", "\xB7\x6E\xC0\x59", "\xB7\x6E\xA4\x46\xB7\x6E\xC0\x59", NULL
  },
  {
    /* 睡著 */
    /* 趴在鍵盤上睡著了，口水流進鍵盤，造成當機！ */
    "sleep", "\xBA\xCE\xB5\xDB", "\xAD\x77\xA6\x62\xC1\xE4\xBD\x4C\xA4\x57\xBA\xCE\xB5\xDB\xA4\x46\xA1\x41\xA4\x66\xA4\xF4\xAC\x79\xB6\x69\xC1\xE4\xBD\x4C\xA1\x41\xB3\x79\xA6\xA8\xB7\xED\xBE\xF7\xA1\x49", NULL
  },
  {
    /* 打鼾中 */
    /* 打鼾中… */
    "snore", "\xA5\xB4\xC2\x4D\xA4\xA4", "\xA5\xB4\xC2\x4D\xA4\xA4\xA1\x4B", NULL
  },
  {
    /* 賤胚 */
    /* Ｓｏｎ Ｏｆ Ｂｉｔｃｈ！！ */
    "sob", "\xBD\xE2\xAD\x46", "\xA2\xE1\xA2\xF7\xA2\xF6 \xA2\xDD\xA2\xEE \xA2\xD0\xA2\xF1\xA2\xFC\xA2\xEB\xA2\xF0\xA1\x49\xA1\x49", NULL
  },
  {
    /* 凝視 */
    /* 靜靜地凝視著天空 */
    "stare", "\xBE\xAE\xB5\xF8", "\xC0\x52\xC0\x52\xA6\x61\xBE\xAE\xB5\xF8\xB5\xDB\xA4\xD1\xAA\xC5", NULL
  },
  {
    /* 疲倦 */
    /* 伸伸懶腰又打了個呵欠很疲倦似的。 */
    "stretch", "\xAF\x68\xAD\xC2", "\xA6\xF9\xA6\xF9\xC3\x69\xB8\x79\xA4\x53\xA5\xB4\xA4\x46\xAD\xD3\xA8\xFE\xA4\xED\xAB\xDC\xAF\x68\xAD\xC2\xA6\xFC\xAA\xBA\xA1\x43", NULL
  },
  {
    /* 講古 */
    /* 開始講古了 */
    "story", "\xC1\xBF\xA5\x6A", "\xB6\x7D\xA9\x6C\xC1\xBF\xA5\x6A\xA4\x46", NULL
  },
  {
    /* 搖擺走 */
    /* 大搖大擺地走 */
    "strut", "\xB7\x6E\xC2\x5C\xA8\xAB",	"\xA4\x6A\xB7\x6E\xA4\x6A\xC2\x5C\xA6\x61\xA8\xAB", NULL
  },
  {
    /* 自殺 */
    /* 自殺 */
    "suicide", "\xA6\xDB\xB1\xFE", "\xA6\xDB\xB1\xFE", NULL
  },
  {
    /* 流汗 */
    /* 揮汗如雨！ */
    "sweat", "\xAC\x79\xA6\xBD", "\xB4\xA7\xA6\xBD\xA6\x70\xAB\x42\xA1\x49", NULL
  },
  {
    /* 流淚 */
    /* 痛哭流涕中..... */
    "tear", "\xAC\x79\xB2\x5C", "\xB5\x68\xAD\xFA\xAC\x79\xAE\xF7\xA4\xA4.....",	NULL
  },
  {
    /* 思考 */
    /* 歪著頭想了一下 */
    "think", "\xAB\xE4\xA6\xD2", "\xAC\x6E\xB5\xDB\xC0\x59\xB7\x51\xA4\x46\xA4\x40\xA4\x55", NULL
  },
  {
    /* 吐舌 */
    /* 吐了吐舌頭 */
    "tongue", "\xA6\x52\xA6\xDE", "\xA6\x52\xA4\x46\xA6\x52\xA6\xDE\xC0\x59", NULL
  },
  {
    /* 撞牆 */
    /* 跑去撞牆 */
    "wall", "\xBC\xB2\xC0\xF0", "\xB6\x5D\xA5\x68\xBC\xB2\xC0\xF0",	NULL
  },
  {
    /* 哇哇 */
    /* 哇哇哇~~~~~!!!!!  ~~~>_<~~~ */
    "wawa", "\xAB\x7A\xAB\x7A", "\xAB\x7A\xAB\x7A\xAB\x7A~~~~~!!!!!  ~~~>_<~~~", NULL
  },
  {
    /* 洗手間 */
    /* 企洗手間一下 :> */
    "wc", "\xAC\x7E\xA4\xE2\xB6\xA1", "\xA5\xF8\xAC\x7E\xA4\xE2\xB6\xA1\xA4\x40\xA4\x55 :>", NULL
  },
  {
    /* 肚子餓 */
    /* 肚子餓!	:( */
    "whine", "\xA8\x7B\xA4\x6C\xBE\x6A", "\xA8\x7B\xA4\x6C\xBE\x6A!	:(", NULL
  },
  {
    /* 吹口哨 */
    /* 吹口哨 */
    "whistle", "\xA7\x6A\xA4\x66\xAD\xEF", "\xA7\x6A\xA4\x66\xAD\xEF", NULL
  },
  {
    /* 狼嚎 */
    /* ㄠㄨㄠㄨ…ㄠㄨㄠㄨ… */
    "wolf", "\xAF\x54\xC0\x7A", "\xA3\xB1\xA3\xB9\xA3\xB1\xA3\xB9\xA1\x4B\xA3\xB1\xA3\xB9\xA3\xB1\xA3\xB9\xA1\x4B", NULL
  },
  {
    /* 汪汪 */
    /* 汪汪汪！ */
    "www", "\xA8\x4C\xA8\x4C", "\xA8\x4C\xA8\x4C\xA8\x4C\xA1\x49", NULL
  },
  {
    /* ㄛ耶 */
    /* 噢～ＹＡ！ *^_^* */
    "ya", "\xA3\xAC\xAD\x43", "\xBE\xBE\xA1\xE3\xA2\xE7\xA2\xCF\xA1\x49 *^_^*", NULL
  },
  {
    /* 打呼 */
    /* 呼嚕～ZZzZzｚＺZZzzZzzzZZ */
    "zzz", "\xA5\xB4\xA9\x49", "\xA9\x49\xC2\x50\xA1\xE3ZZzZz\xA3\x43\xA2\xE8ZZzzZzzzZZ", NULL
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
condition_action(cu, cmd)
  ChatUser *cu;
  char *cmd;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(condition_data, ACTNUM_CONDITION, cmd)))
  {
    sprintf(buf, "\033[1;32m%s \033[31m%s\033[m",
      cu->chatid, cap->part1_msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 1;
  }
  return 0;
}


/* --------------------------------------------- */
/* MUD-like social commands : help               */
/* --------------------------------------------- */


static char *dscrb[] =
{
  /* \033[1;37m【 Verb + Nick：   動詞 + 對方名字 】\033[36m  例：//kick piggy\033[m */
  "\033[1;37m\xA1\x69 Verb + Nick\xA1\x47   \xB0\xCA\xB5\xFC + \xB9\xEF\xA4\xE8\xA6\x57\xA6\x72 \xA1\x6A\033[36m  \xA8\xD2\xA1\x47//kick piggy\033[m",
  /* \033[1;37m【 Verb + Message：動詞 + 要說的話 】\033[36m  例：//sing 天天天藍\033[m */
  "\033[1;37m\xA1\x69 Verb + Message\xA1\x47\xB0\xCA\xB5\xFC + \xAD\x6E\xBB\xA1\xAA\xBA\xB8\xDC \xA1\x6A\033[36m  \xA8\xD2\xA1\x47//sing \xA4\xD1\xA4\xD1\xA4\xD1\xC2\xC5\033[m",
  /* \033[1;37m【 Verb：動詞 】   ↑↓：舊話重提\033[m */
  "\033[1;37m\xA1\x69 Verb\xA1\x47\xB0\xCA\xB5\xFC \xA1\x6A   \xA1\xF4\xA1\xF5\xA1\x47\xC2\xC2\xB8\xDC\xAD\xAB\xB4\xA3\033[m", NULL
};



static ChatAction *catbl[] =
{
  party_data, speak_data, condition_data, NULL
};


static void
chat_partyinfo(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
  {
    /* 3 動作  交談  狀態 */
    send_to_user(cu, "3 \xB0\xCA\xA7\x40  \xA5\xE6\xBD\xCD  \xAA\xAC\xBA\x41", 0, MSG_PARTYINFO);
  }
}


static void
chat_party(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int kind, i;
  ChatAction *cap;
  char buf[80];

  if (!common_client_command)
    return;

  kind = atoi(nextword(&msg));
  if (kind < 0 || kind > 2)
    return;

  sprintf(buf, "%d\t%s", kind, kind == 2 ? "I" : "");

  /* Xshadow: 只有 condition 才是 immediate mode */
  send_to_user(cu, buf, 0, MSG_PARTYLISTSTART);

  cap = catbl[kind];
  for (i = 0; cap[i].verb; i++)
  {
    sprintf(buf, "%-10s %-20s", cap[i].verb, cap[i].chinese);
    send_to_user(cu, buf, 0, MSG_PARTYLIST);
  }

  sprintf(buf, "%d", kind);
  send_to_user(cu, buf, 0, MSG_PARTYLISTEND);
}


#define	MAX_VERB_LEN	8
#define VERB_NO		10


static void
view_action_verb(cu, cmd)	/* Thor.980726: 新加動詞分類顯示 */
  ChatUser *cu;
  int cmd;
{
  int i;
  char *p, *q, *data, *expn, buf[256];
  ChatAction *cap;

  send_to_user(cu, "/c", 0, MSG_CLRSCR);

  data = buf;

  if (cmd < '1' || cmd > '3')
  {				/* Thor.980726: 寫得不好, 想辦法改進... */
    for (i = 0; p = dscrb[i]; i++)
    {
      /*   [//]help %d          - MUD-like 社交動詞   第 %d 類 */
      sprintf(data, "  [//]help %d          - MUD-like \xAA\xC0\xA5\xE6\xB0\xCA\xB5\xFC   \xB2\xC4 %d \xC3\xFE", i + 1, i + 1);
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, p, 0, MSG_MESSAGE);
      send_to_user(cu, " ", 0, MSG_MESSAGE);	/* Thor.980726: 換行 */
    }
  }
  else
  {
    i = cmd - '1';

    send_to_user(cu, dscrb[i], 0, MSG_MESSAGE);

    expn = buf + 100;		/* Thor.980726: 應該不會overlap吧? */

    *data = '\0';
    *expn = '\0';

    cap = catbl[i];

    for (i = 0; p = cap[i].verb; i++)
    {
      q = cap[i].chinese;

      strcat(data, p);
      strcat(expn, q);

      if (((i + 1) % VERB_NO) == 0)
      {
	send_to_user(cu, data, 0, MSG_MESSAGE);
	send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.980726: 顯示中文註解 */
	*data = '\0';
	*expn = '\0';
      }
      else
      {
	strncat(data, "        ", MAX_VERB_LEN - strlen(p));
	strncat(expn, "        ", MAX_VERB_LEN - strlen(q));
      }
    }

    if (i % VERB_NO)
    {
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.980726: 顯示中文註解 */
    }
  }
  /* send_to_user(cu, " ", 0); *//* Thor.980726: 換行, 需要 " " 嗎? */
}


/* ----------------------------------------------------- */
/* chat user service routines                            */
/* ----------------------------------------------------- */


static ChatCmd chatcmdlist[] =
{
  "act", chat_act, 0,
  "bye", chat_bye, 0,
  "chatroom", chat_chatroom, 1,		/* Xshadow: for common client */
  "clear", chat_clear, 0,
  "cloak", chat_cloak, 2,
  "date", chat_date, 0,
  "flags", chat_setroom, 0,
  "help", chat_help, 0,
  "ignore", chat_ignore, 1,
  "invite", chat_invite, 0,
  "join", chat_join, 0,
  "kick", chat_kick, 1,
  "msg", chat_private, 0,
  "nick", chat_nick, 0,
  "operator", chat_makeop, 0,
  "party", chat_party, 1,		/* Xshadow: party data for common client */
  "partyinfo", chat_partyinfo, 1,	/* Xshadow: party info for common client */

#ifndef STAND_ALONE
  "query", chat_query, 0,
#endif

  "quit", chat_bye, 0,

  "room", chat_list_rooms, 0,
  "unignore", chat_unignore, 1,
  "whoin", chat_list_by_room, 1,
  "wall", chat_broadcast, 2,

  "who", chat_map_chatids_thisroom, 0,
  "list", chat_list_users, 0,
  "topic", chat_topic, 1,
  "version", chat_version, 1,

  NULL, NULL, 0
};


/* Thor: 0 不用 exact, 1 要 exactly equal, 2 秘密指令 */


static int
command_execute(cu)
  ChatUser *cu;
{
  char *cmd, *msg, buf[128];
  /* Thor.981108: lkchu patch: chatid + msg 只用 80 bytes 不夠, 改為 128 */
  ChatCmd *cmdrec;
  int match, ch;

  msg = cu->ibuf;
  match = *msg;

  /* Validation routine */

  if (cu->room == NULL)
  {
    /* MUST give special /! or /-! command if not in the room yet */

    if (match == '/' && ((ch = msg[1]) == '!' || (ch == '-' && msg[2] == '!')))
    {
      if (ch == '-')
	fprintf(flog, "cli\t[%d] S%d\n", cu->sno, cu->sock);

      cu->clitype = (ch == '-') ? 1 : 0;
      return (login_user(cu, msg + 2 + cu->clitype));
    }
    else
    {
      return -1;
    }
  }

  /* If not a /-command, it goes to the room. */

  if (match != '/')
  {
    if (match)
    {
      if (cu->room && !CLOAK(cu))	/* 隱身的人也不能說話哦 */
      {
	char chatid[16];

	sprintf(chatid, "%s:", cu->chatid);
	sprintf(buf, "%-10s%s", chatid, msg);
	send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      }
    }
    return 0;
  }

  msg++;
  cmd = nextword(&msg);
  match = 0;

  if (*cmd == '/')
  {
    cmd++;
    /* if (!*cmd || !str_cmp("help", cmd)) */
    if (!*cmd || str_match(cmd, "help") >= 0)	/* itoc.010321: 部分 match 就算 */
    {
      cmd = nextword(&msg);	/* Thor.980726: 動詞分類 */
      view_action_verb(cu, *cmd);
      match = 1;
    }
    else if (!party_action(cu, cmd, msg))
      match = 1;
    else if (!speak_action(cu, cmd, msg))
      match = 1;
    else
      match = condition_action(cu, cmd);
  }
  else
  {
    char *str;

    common_client_command = 0;
    if (*cmd == '-')
    {
      if (cu->clitype)
      {
	cmd++;			/* Xshadow: 指令從下一個字元才開始 */
	common_client_command = 1;
      }
      else
      {
	/* 不是 common client 但送出 common client 指令 -> 假裝沒看到 */
      }
    }

    str_lower(buf, cmd);

    for (cmdrec = chatcmdlist; str = cmdrec->cmdstr; cmdrec++)
    {
      switch (cmdrec->exact)
      {
      case 1:			/* exactly equal */
	match = !str_cmp(str, buf);
	break;

      case 2:			/* Thor: secret command */
	if (CHATSYSOP(cu))
	  match = !str_cmp(str, buf);
	break;

      default:			/* not necessary equal */
	match = str_match(buf, str) >= 0;
	break;
      }

      if (match)
      {
	cmdrec->cmdfunc(cu, msg);
	break;
      }
    }
  }

  if (!match)
  {
    /* ◆ 指令錯誤：/%s */
    sprintf(buf, "\xA1\xBB \xAB\xFC\xA5\x4F\xBF\xF9\xBB\x7E\xA1\x47/%s", cmd);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* serve chat_user's connection                          */
/* ----------------------------------------------------- */


static int
cuser_serve(cu)
  ChatUser *cu;
{
  int ch, len, isize;
  char *str, *cmd, buf[256];

  str = buf;
  len = recv(cu->sock, str, sizeof(buf) - 1, 0);
  if (len < 0)
  {
    ch = errno;

    exit_room(cu, EXIT_LOSTCONN, NULL);
    logit("recv", strerror(ch));
    return -1;
  }

  if (len == 0)
  {
    if (++cu->retry > 100)
      return -1;
    return 0;
  }

#if 0
  /* Xshadow: 將送達的資料忠實紀錄下來 */
  memcpy(logbuf, buf, sizeof(buf));
  for (ch = 0; ch < sizeof(buf); ch++)
  {
    if (!logbuf[ch])
      logbuf[ch] = '$';
  }

  logbuf[len + 1] = '\0';
  logit("recv: ", logbuf);
#endif

#if 0
  logit(cu->userid, str);
#endif

  cu->xdata += len;

  isize = cu->isize;
  cmd = cu->ibuf + isize;
  while (len--)
  {
    ch = *str++;

    if (ch == '\r' || !ch)
      continue;

    if (ch == '\n')
    {
      *cmd = '\0';

      if (command_execute(cu) < 0)
	return -1;

      isize = 0;
      cmd = cu->ibuf;

      continue;
    }

    if (isize < SCR_WIDTH)
    {
      *cmd++ = ch;
      isize++;
    }
  }
  cu->isize = isize;
  return 1;
}


/* ----------------------------------------------------- */
/* chatroom server core routines                         */
/* ----------------------------------------------------- */


static int
/* start_daemon(mode)
  int mode; */
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct sockaddr_in sin;
  struct linger ld;
#ifdef HAVE_RLIMIT
  struct rlimit limit;
#endif

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  /* time((time_t *) &value); */
  time_t now;
  time(&now);
  value = (int)now;

  /* gmtime((time_t *) &value); */
  gmtime(&now);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime(&now));

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve                           */
  /* --------------------------------------------------- */

  dns_init();
#if 0
  gethostname(buf, sizeof(buf));
  gethostbyname(buf);
#endif

#ifdef HAVE_RLIMIT
  /* --------------------------------------------------- */
  /* adjust the resource limit                           */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

  limit.rlim_cur = limit.rlim_max = 4 * 1024 * 1024;
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS	/* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

#if 0
  limit.rlim_cur = limit.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &limit);
#endif
#endif

  /* --------------------------------------------------- */
  /* detach daemon process                               */
  /* --------------------------------------------------- */

  close(2);
  close(1);

  /* if (mode > 1) */
  if (inetd)
    return 0;

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* bind the service port				 */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  /*
   * timeout 方面, 將 socket 改成 O_NDELAY (no delay, non-blocking),
   * 如果能順利送出資料就送出, 不能送出就算了, 不再等待 TCP_TIMEOUT 時間。
   * (default 是 120 秒, 並且有 3-way handshaking 機制, 有可能一等再等)。
   */

#if 1
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
#endif

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

  value = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

  if ((bind(fd, (struct sockaddr *) & sin, sizeof(sin)) < 0) ||
    (listen(fd, SOCK_QLEN) < 0))
    exit(1);

  return fd;
}


#ifdef	SERVER_USAGE
static void
server_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    "user time: %.6f\n"
    "system time: %.6f\n"
    "maximum resident set size: %lu P\n"
    "integral resident set size: %lu\n"
    "page faults not requiring physical I/O: %d\n"
    "page faults requiring physical I/O: %d\n"
    "swaps: %d\n"
    "block input operations: %d\n"
    "block output operations: %d\n"
    "messages sent: %d\n"
    "messages received: %d\n"
    "signals received: %d\n"
    "voluntary context switches: %d\n"
    "involuntary context switches: %d\n"
    "gline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    ru.ru_minflt,
    ru.ru_majflt,
    ru.ru_nswap,
    ru.ru_inblock,
    ru.ru_oublock,
    ru.ru_msgsnd,
    ru.ru_msgrcv,
    ru.ru_nsignals,
    ru.ru_nvcsw,
    ru.ru_nivcsw,
    gline);

  fflush(flog);
}
#endif


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0)
    ;
}


static void
sig_trap(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "signal [%d] at line %d (errno: %d)", sig, gline, errno);
  logit("EXIT", buf);
  fclose(flog);
  exit(1);
}


static void
sig_over()
{
  int fd;

  server_usage();
  logit("OVER", "");
  fclose(flog);
  for (fd = 0; fd < 64; fd++)
    close(fd);
  execl("bin/xchatd", NULL);
}


static void
main_signals()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); */
  /* Thor.981206: 統一 POSIX 標準用法  */

  /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = sig_trap;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  act.sa_handler = sig_over;
  sigaction(SIGXCPU, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = server_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: 統一 POSIX 標準用法  */
  /* 在此借用 sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int sock, nfds, maxfds, servo_sno;
  ChatUser *cu,/* *userpool,*/ **FBI;
  time_t uptime, tcheck;
  fd_set rset, xset;
  static struct timeval tv = {CHAT_INTERVAL, 0};
  struct timeval tv_tmp; /* Thor.981206: for future reservation bug */

  sock = 0;

  while ((nfds = getopt(argc, argv, "hid")) != -1)
  {
    switch (nfds)
    {
    case 'i':
      sock = 1;
      break;

    case 'd':
      break;

    case 'h':
    default:

      fprintf(stderr, "Usage: %s [options]\n"
        "\t-i  start from inetd with wait option\n"
        "\t-d  debug mode\n"
        "\t-h  help\n",
        argv[0]);
      exit(0);
    }
  }

  servo_daemon(sock);
  /* start_daemon(argc); */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  umask(077);

  log_init();

  main_signals();

  /* --------------------------------------------------- */
  /* init variable : rooms & users			 */
  /* --------------------------------------------------- */

  userpool = NULL;
  strcpy(mainroom.name, MAIN_NAME);
  strcpy(mainroom.topic, MAIN_TOPIC);

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  tcheck = 0;
  servo_sno = 0;

  for (;;)
  {
    uptime = time(0);
    if (tcheck < uptime)
    {
      nfds = maxfds = 0;
      FD_ZERO(&mainfset);
      FD_SET(0, &mainfset);

      tcheck = uptime - CHAT_INTERVAL;

      for (FBI = &mainuser; cu = *FBI;)
      {
	if (cu->uptime < tcheck)
	{
	  cuser_free(cu);

	  *FBI = cu->unext;

	  cu->unext = userpool;
	  userpool = cu;
	}
	else
	{
	  nfds++;
	  sock = cu->sock;
	  FD_SET(sock, &mainfset);
	  if (maxfds < sock)
	    maxfds = sock;

	  FBI = &(cu->unext);
	}
      }

      totaluser = nfds;
      fprintf(flog, "MAINTAIN %d user (%d)\n", nfds, maxfds++);
      fflush(flog);

      tcheck = uptime + CHAT_INTERVAL;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    rset = mainfset;
    xset = mainfset;

    /* Thor.981206: for future reservation bug */
    tv_tmp = tv;
    nfds = select(maxfds, &rset, NULL, &xset, &tv_tmp);

#if 0
    {
      char buf[32];
      static int xxx;

      if ((++xxx & 8191) == 0)
      {
	sprintf(buf, "%d/%d", nfds, maxfds);
	logit("MAIN", buf);
      }
    }
#endif

    if (nfds == 0)
    {
      continue;
    }

    if (nfds < 0)
    {
      sock = errno;
      if (sock != EINTR)
      {
	logit("select", strerror(sock));
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &mainuser; cu = *FBI;)
    {
      sock = cu->sock;

      if (FD_ISSET(sock, &rset))
      {
	static int xxx, xno;

	nfds = cuser_serve(cu);

	if ((++xxx & 511) == 0)
	{
	  int sno;

	  sno = cu->sno;
	  fprintf(flog, "rset\t[%d] S%d R%d %d\n", sno, sock, nfds, xxx);
	  if (sno == xno)
	    nfds = -1;
	  else
	    xno = sno;
	}
      }
      else if (FD_ISSET(sock, &xset))
      {
	nfds = -1;
      }
      else
      {
	nfds = 0;
      }

      if (nfds < 0 || cu->uptime <= 0)	/* free this client */
      {
	cuser_free(cu);

	*FBI = cu->unext;

	cu->unext = userpool;
	userpool = cu;

	continue;
      }

      if (nfds > 0)
      {
	cu->uptime = uptime;
      }

      FBI = &(cu->unext);
    }

    /* ------------------------------------------------- */
    /* accept new connection				 */
    /* ------------------------------------------------- */

    if (FD_ISSET(0, &rset))
    {

      {
	static int yyy;

	if ((++yyy & 2047) == 0)
	  fprintf(flog, "conn\t%d\n", yyy);
      }

      for (;;)
      {
	int value;
	struct sockaddr_in sin;

	value = sizeof(sin);
	sock = accept(0, (struct sockaddr *) &sin, &value);
	if (sock > 0)
	{
	  if (cu = userpool)
	  {
	    userpool = cu->unext;
	  }
	  else
	  {
	    cu = (ChatUser *) malloc(sizeof(ChatUser));
	  }

	  *FBI = cu;

	  /* variable initialization */

	  memset(cu, 0, sizeof(ChatUser));
	  cu->sock = sock;
	  cu->tbegin = uptime;
	  cu->uptime = uptime;
	  cu->sno = ++servo_sno;
	  cu->xdata = 0;
	  cu->retry = 0;
	  memcpy(cu->rhost, &sin.sin_addr, sizeof(struct in_addr));

	  totaluser++;

	  FD_SET(sock, &mainfset);
	  if (sock >= maxfds)
	    maxfds = sock + 1;

	  {
	    int value;

	    value = 1;
	    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
	      (char *) &value, sizeof(value));
	  }

#if 1
	  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NDELAY);
#endif

	  fprintf(flog, "CONN\t[%d] %d %s\n",
	    servo_sno, sock, Btime(cu->tbegin));
	  break;
	}

	nfds = errno;
	if (nfds != EINTR)
	{
	  logit("accept", strerror(nfds));
	  break;
	}

#if 0
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
#endif
      }
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */

  }
}
