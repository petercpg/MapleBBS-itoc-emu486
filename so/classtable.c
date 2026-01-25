/*-------------------------------------------------------*/
/* classtable.c	( YZU WindTopBBS Ver 3.02 )		 */
/*-------------------------------------------------------*/
/* target : 功課表					 */
/* create :   /  /                                       */
/* update : 02/07/12                                     */
/* author :						 */
/* modify : itoc.bbs@bbs.ee.nctu.edu.tw			 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_CLASSTABLE

/* ----------------------------------------------------- */
/* classtable.c 中運用的資料結構			 */
/* ----------------------------------------------------- */


#define MAX_WEEKDAY	6	/* 一星期有 6 天 */
#define	MAX_DAYCLASS	16	/* 一天有 16 節 */


typedef struct
{
  char name[9];		/* 課名 */
  char teacher[9];	/* 教師 */
  char class[5];	/* 教室 */
  char objid[7];	/* 課號 */
}   CLASS;


typedef struct
{
  CLASS table[MAX_WEEKDAY][MAX_DAYCLASS];	/* 一星期 MAX_WEEKDAY * MAX_DAYCLASS 堂課 */
}   CLASS_TABLE;


typedef struct
{
  char c_class[5];	/* 第幾節 */
  char c_start[6];	/* 上課時間 */
  char c_break[6];	/* 下課時間 */
}  CLOCK;


static CLOCK class_time[MAX_DAYCLASS] = 	/* 課堂時間 */
{
  /*  一  */
  {" \xA4\x40 ", "06:00", "06:50"}, 
  /*  二  */
  {" \xA4\x47 ", "07:00", "07:50"}, 
  /*  三  */
  {" \xA4\x54 ", "08:00", "08:50"}, 
  /*  四  */
  {" \xA5\x7C ", "09:00", "09:50"}, 
  /*  五  */
  {" \xA4\xAD ", "10:10", "11:00"}, 
  /*  六  */
  {" \xA4\xBB ", "11:10", "12:00"}, 
  /*  七  */
  {" \xA4\x43 ", "12:30", "13:20"}, 
  /*  八  */
  {" \xA4\x4B ", "13:30", "14:20"}, 
  /*  九  */
  {" \xA4\x45 ", "14:30", "15:20"}, 
  /*  十  */
  {" \xA4\x51 ", "15:40", "16:30"}, 
  /* 十一 */
  {"\xA4\x51\xA4\x40", "16:40", "17:30"}, 
  /* 十二 */
  {"\xA4\x51\xA4\x47", "17:40", "18:30"}, 
  /* 十三 */
  {"\xA4\x51\xA4\x54", "18:30", "19:20"}, 
  /* 十四 */
  {"\xA4\x51\xA5\x7C", "19:30", "20:20"}, 
  /* 十五 */
  {"\xA4\x51\xA4\xAD", "20:30", "21:20"}, 
  /* 十六 */
  {"\xA4\x51\xA4\xBB", "21:30", "22:20"}
};


/* ----------------------------------------------------- */
/* CLASS 處理函數					 */
/* ----------------------------------------------------- */


static void
class_show(x, y, class)
  int x, y;
  CLASS *class;
{
  move(x, y);
  /* 課名：%s */
  prints("\xBD\xD2\xA6\x57\xA1\x47%s", class->name);
  move(x + 1, y);
  /* 教師：%s */
  prints("\xB1\xD0\xAE\x76\xA1\x47%s", class->teacher);
  move(x + 2, y);
  /* 教室：%s */
  prints("\xB1\xD0\xAB\xC7\xA1\x47%s", class->class);
  move(x + 3, y);
  /* 課號：%s */
  prints("\xBD\xD2\xB8\xB9\xA1\x47%s", class->objid);
}


static void
class_edit(class)
  CLASS *class;
{
  int echo;

  echo = *(class->name) ? GCARRY : DOECHO;
  /* 課名： */
  vget(4, 0, "\xBD\xD2\xA6\x57\xA1\x47", class->name, sizeof(class->name), echo);
  /* 教師： */
  vget(5, 0, "\xB1\xD0\xAE\x76\xA1\x47", class->teacher, sizeof(class->teacher), echo);
  /* 教室： */
  vget(6, 0, "\xB1\xD0\xAB\xC7\xA1\x47", class->class, sizeof(class->class), echo);
  /* 課號： */
  vget(7, 0, "\xBD\xD2\xB8\xB9\xA1\x47", class->objid, sizeof(class->objid), echo);
}


static int			/* 1:正確 0:錯誤 */
class_number(day, class)	/* 傳回星期幾第幾節 */
  int *day;
  int *class;
{
  char ans[5];

  move(2, 0);
  /* 503 表示星期五第三節 */
  outs("503 \xAA\xED\xA5\xDC\xAC\x50\xB4\xC1\xA4\xAD\xB2\xC4\xA4\x54\xB8\x60");
  /* 上課時間： */
  *day = vget(3, 0, "\xA4\x57\xBD\xD2\xAE\xC9\xB6\xA1\xA1\x47", ans, 4, DOECHO) - '1';	/* 503 表示星期五第三節 */
  *class = atoi(ans + 1) - 1;
  if (*day > MAX_WEEKDAY - 1 || *day < 0 || *class > MAX_DAYCLASS - 1 || *class < 0)
    return 0;

  return 1;
}


/* ----------------------------------------------------- */
/* CLASS_TABLE 處理函數					 */
/* ----------------------------------------------------- */


static void
table_file(fpath, table)	/* 把 table 寫入 FN_CLASSTBL_LOG */
  char *fpath;
  CLASS_TABLE *table;
{
  int i, j;
  FILE *fp;

  fp = fopen(fpath, "w");

  /*            星期一    星期二    星期三    星期四    星期五    星期六\n */
  fprintf(fp, "           \xAC\x50\xB4\xC1\xA4\x40    \xAC\x50\xB4\xC1\xA4\x47    \xAC\x50\xB4\xC1\xA4\x54    \xAC\x50\xB4\xC1\xA5\x7C    \xAC\x50\xB4\xC1\xA4\xAD    \xAC\x50\xB4\xC1\xA4\xBB\n");
  for (i = 0; i < MAX_DAYCLASS; i++)
  {
    /* 第%s節   */
    fprintf(fp, "\xB2\xC4%s\xB8\x60  ", class_time[i].c_class);
    for (j = 0; j < MAX_WEEKDAY; j++)
      fprintf(fp, "%-8.8s  ", table->table[j][i].name);

    fprintf(fp, "\n  %s   ", class_time[i].c_start);
    for (j = 0; j < MAX_WEEKDAY; j++)
      fprintf(fp, "%-8.8s  ", table->table[j][i].teacher);

    /* \n   ↓      */
    fprintf(fp, "\n   \xA1\xF5     ");
    for (j = 0; j < MAX_WEEKDAY; j++)
      fprintf(fp, "%-8.8s  ", table->table[j][i].class);

    fprintf(fp, "\n  %s   ", class_time[i].c_break);
    for (j = 0; j < MAX_WEEKDAY; j++)
      fprintf(fp, "%-8.8s  ", table->table[j][i].objid);

    fprintf(fp, "\n\n");
  }
  fclose(fp);
}


static void
table_show(table)
  CLASS_TABLE *table;
{
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_CLASSTBL_LOG);
  table_file(fpath, table);
  more(fpath, NULL);
}


static void
table_mail(table)
  CLASS_TABLE *table;
{
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_CLASSTBL_LOG);
  table_file(fpath, table);
  /* 個人功課表 */
  mail_self(fpath, cuser.userid, "\xAD\xD3\xA4\x48\xA5\x5C\xBD\xD2\xAA\xED", MAIL_READ);
}


static void
table_edit(table)
  CLASS_TABLE *table;
{
  int i, j;

  /* 編輯個人功課表 */
  vs_bar("\xBD\x73\xBF\xE8\xAD\xD3\xA4\x48\xA5\x5C\xBD\xD2\xAA\xED");

  if (class_number(&i, &j))
  {
    class_edit(&(table->table[i][j]));
    class_show(10, 0, &(table->table[i][j]));
  }
}


static void
table_del(table)
  CLASS_TABLE *table;
{
  int i, j;

  /* 刪除個人功課表 */
  vs_bar("\xA7\x52\xB0\xA3\xAD\xD3\xA4\x48\xA5\x5C\xBD\xD2\xAA\xED");

  if (!class_number(&i, &j))
    return;

  class_show(10, 0, &(table->table[i][j]));

  if (vans(msg_sure_ny) == 'y')
    memset(&(table->table[i][j]), 0, sizeof(CLASS));
}


static void
table_copy(table)
  CLASS_TABLE *table;
{
  int i, j, x, y;

  /* 個人功課表 */
  vs_bar("\xAD\xD3\xA4\x48\xA5\x5C\xBD\xD2\xAA\xED");

  move(9, 0);
  /* 來源： */
  outs("\xA8\xD3\xB7\xBD\xA1\x47");
  if (!class_number(&i, &j))
    return;

  class_show(10, 0, &(table->table[i][j]));

  move(9, 39);
  /* 目的： */
  outs("\xA5\xD8\xAA\xBA\xA1\x47");
  if (!class_number(&x, &y))
    return;

  class_show(10, 39, &(table->table[x][y]));

  if (vans(msg_sure_ny) == 'y')
    memcpy(&(table->table[x][y]), &(table->table[i][j]), sizeof(CLASS));
}


int
main_classtable()
{
  char fpath[64];
  CLASS_TABLE newtable, oldtable, *ptr;

  usr_fpath(fpath, cuser.userid, FN_CLASSTBL);
  ptr = &newtable;

  if (rec_get(fpath, ptr, sizeof(CLASS_TABLE), 0))
    memset(ptr, 0, sizeof(CLASS_TABLE));
  memcpy(&oldtable, ptr, sizeof(CLASS_TABLE));

  for (;;)
  {
    /* 課表系統 (E/C/D)編輯/複製/刪除 P)印出 K)全砍 S)存檔 M)信箱 Q)離開 [Q]  */
    switch (vans("\xBD\xD2\xAA\xED\xA8\x74\xB2\xCE (E/C/D)\xBD\x73\xBF\xE8/\xBD\xC6\xBB\x73/\xA7\x52\xB0\xA3 P)\xA6\x4C\xA5\x58 K)\xA5\xFE\xAC\xE5 S)\xA6\x73\xC0\xC9 M)\xAB\x48\xBD\x63 Q)\xC2\xF7\xB6\x7D [Q] "))
    {
    case 'e':
      table_edit(ptr);
      break;
    case 'd':
      table_del(ptr);
      break;
    case 'c':
      table_copy(ptr);
      break;

    case 'p':
      table_show(ptr);
      break;
    case 'm':
      table_mail(ptr);
      break;

    case 's':
      rec_put(fpath, ptr, sizeof(CLASS_TABLE), 0, NULL);
      memcpy(&oldtable, ptr, sizeof(CLASS_TABLE));
      /* 儲存完成 */
      vmsg("\xC0\x78\xA6\x73\xA7\xB9\xA6\xA8");
      break;
    case 'k':
      if (vans(msg_sure_ny) == 'y')
      {
	unlink(fpath);
	memset(ptr, 0, sizeof(CLASS_TABLE));
	memset(&oldtable, 0, sizeof(CLASS_TABLE));
      }
      break;

    default:
      goto end_loop;
    }
  }

end_loop:

  /* 檢查新舊是否一樣，若不一樣要問是否存檔 */
  if (memcmp(&oldtable, ptr, sizeof(CLASS_TABLE)))
  {
    /* 是否儲存(Y/N)？[Y]  */
    if (vans("\xAC\x4F\xA7\x5F\xC0\x78\xA6\x73(Y/N)\xA1\x48[Y] ") != 'n')
      rec_put(fpath, ptr, sizeof(CLASS_TABLE), 0, NULL);
  }

  return 0;  
}
#endif	/* HAVE_CLASSTABLE */
