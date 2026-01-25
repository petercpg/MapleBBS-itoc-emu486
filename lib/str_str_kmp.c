/* a alternative str_str() using KMP algorithm. */
/* English case independent and BIG5 Chinese supported */

#include <stdlib.h>


void
str_expand(dst, src)	/* 將 char 轉為 short，並將英文變小寫 */
  char *dst, *src;
{
  int ch;
  int in_chi = 0;	/* 1: 前一碼是中文字 */

  do
  {
    ch = *src++;

    if (in_chi || ch & 0x80)
    {
      in_chi ^= 1;
    }
    else
    {    
      if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;
      *dst++ = 0;
    }
    *dst++ = ch;
  } while (ch);
}


void
str_str_kmp_tbl(pat, tbl)
  const short *pat;
  int *tbl;
{
  register short c;
  register int i, j;

  tbl[0] = -1;
  for (j = 1; c = pat[j]; j++)
  {
    i = tbl[j - 1];
    while (i >= 0 && c != pat[i + 1])
      i = tbl[i];
    tbl[j] = (c == pat[i + 1]) ? i + 1 : -1;
  }
}


const int
str_str_kmp(str, pat, tbl)
  const short *str;
  const short *pat;
  const int *tbl;
{
  register const short *i;
  register int j;

  for (i = str, j = 0; *i && pat[j];)
  {
    if (*i == pat[j])
    {
      j++;
    }
    else if (j)
    {
      j = tbl[j - 1] + 1;
      continue;		/* 不需要 i++ */
    }
    i++;
  }

  /* match */
  if (!pat[j])
    return 1;

  return 0;
}


#undef	TEST

#ifdef TEST
static void
try_match(str, key)
  char *str, *key;
{
  short a[256], b[256];		/* 假設 256 已足夠 */
  int tbl[256];

  str_expand(a, str);
  str_expand(b, key);

  str_str_kmp_tbl(key, tbl);
  /* 「%s」 %s包括 「%s」\n */
  printf("\xA1\x75%s\xA1\x76 %s\xA5\x5D\xAC\x41 \xA1\x75%s\xA1\x76\n", 
    /* 不 */
    str, str_str_kmp(a, b, tbl) ? "" : "\xA4\xA3", key);
}


int
main()
{
  /* 好的電影 */
  /* 犒 */
  try_match("\xA6\x6E\xAA\xBA\xB9\x71\xBC\x76", "\xBA\xB9");
  /* 好的電影 */
  try_match("\xA6\x6E\xAA\xBA\xB9\x71\xBC\x76", "N");
  /* 好的電影 */
  try_match("\xA6\x6E\xAA\xBA\xB9\x71\xBC\x76", "n");
  /* 好的電影 */
  /* 好的 */
  try_match("\xA6\x6E\xAA\xBA\xB9\x71\xBC\x76", "\xA6\x6E\xAA\xBA");

  /* x好的x電影 */
  /* 漩電 */
  try_match("x\xA6\x6E\xAA\xBAx\xB9\x71\xBC\x76", "\xBA\x78\xB9\x71");
  /* x好的x電影 */
  /* 的x */
  try_match("x\xA6\x6E\xAA\xBAx\xB9\x71\xBC\x76", "\xAA\xBAx");
  /* x好的x電影 */
  /* 的X */
  try_match("x\xA6\x6E\xAA\xBAx\xB9\x71\xBC\x76", "\xAA\xBAX");
  /* x好的X電影 */
  /* 的x */
  try_match("x\xA6\x6E\xAA\xBAX\xB9\x71\xBC\x76", "\xAA\xBAx");

  /* abx好的x電影 */
  /* x電 */
  try_match("abx\xA6\x6E\xAA\xBAx\xB9\x71\xBC\x76", "x\xB9\x71");

  return 0;
}
#endif
