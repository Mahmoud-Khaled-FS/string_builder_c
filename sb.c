#include "sb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

StringBuilder *sb_init_cap(size_t cap)
{
  StringBuilder *sb = malloc(sizeof(StringBuilder));

  sb->data = malloc(sizeof(char) * cap);
  sb->cap = cap;
  sb->len = 0;
  return sb;
}

StringBuilder *sb_init()
{
  return sb_init_cap(INITIAL_CAPACITY);
}

StringBuilder *sb_from_string(char *string)
{
  StringBuilder *sb = sb_init(MAX(strlen(string), INITIAL_CAPACITY));
  sb_push_str(sb, string);
  return sb;
}

void sb_valid_index(StringBuilder *sb, size_t index, char *fn_name)
{
  if (index < 0 || index >= sb->len)
  {
    fprintf(stderr, "String Builder Error: index out of range %ld (%s)\n", index, fn_name);
    exit(1);
  }
}

char sb_check_capacity(StringBuilder *sb, size_t textSize)
{
  if (sb->cap > sb->len + textSize)
  {
    return 1;
  }
  if (sb->cap == 0)
  {
    sb->cap = INITIAL_CAPACITY;
  }
  else
  {
    sb->cap <<= 1;
  }
  if (sb->cap == 0)
  {
    return 0;
  }
  if (sb->cap < textSize)
  {
    sb->cap += textSize;
  }
  printf("INC CAP: %zu\n", sb->cap);
  sb->data = realloc(sb->data, sb->cap * sizeof(char));
  if (!sb->data)
  {
    return 0;
  }
  return 1;
}

char *sb_to_string(StringBuilder *sb)
{
  sb->data[sb->len] = '\0';
  return sb->data;
}

void sb_free(StringBuilder *sb)
{
  if (sb == NULL)
    return;
  free(sb->data);
  free(sb);
}

int sb_push_str(StringBuilder *sb, char *text)
{
  printf("PUSH: %s\n", text);
  size_t textSize = strlen(text);
  if (!sb_check_capacity(sb, textSize + 1))
    return 0;
  memcpy(sb->data + sb->len, text, textSize);
  sb->len += textSize;
  sb->data[sb->len] = '\0';
  return 1;
}

int sb_push_int(StringBuilder *sb, int number)
{
  return sb_push_format(sb, "%d", number);
}

int sb_push_format(StringBuilder *sb, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  char buf[4095];
  vsnprintf(&buf[0], 4095, format, args);
  va_end(args);
  return sb_push_str(sb, buf);
}

int sb_replace_char(StringBuilder *sb, char ch, size_t index)
{
  if (index < 0 || index >= sb->len)
    return 0;
  sb->data[index] = ch;
  return 1;
}

char sb_char_at(StringBuilder *sb, size_t index)
{
  sb_valid_index(sb, index, "sb_char_at");

  return sb->data[index];
}

void sb_slice(StringBuilder *sb, size_t start, size_t end)
{
  sb_valid_index(sb, start, "sb_delete");
  if (end > sb->len)
    end = sb->len;
  if (start > end)
  {
    return;
  }

  size_t removedItemsCount = end - start;
  for (size_t i = end; i < sb->len; i++)
  {
    sb->data[start + (i - end)] = sb->data[i];
  }
  sb->len -= removedItemsCount;
}

void sb_clear(StringBuilder *sb)
{
  if (sb->cap > THRESHOLD_SIZE)
  {
    free(sb->data);
    sb->data = malloc(INITIAL_CAPACITY * sizeof(char));
    sb->cap = INITIAL_CAPACITY;
  }
  else
  {
    memset(sb->data, 0, sb->len);
  }
  sb->len = 0;
}

void sb_set(StringBuilder *sb, char ch)
{
  memset(sb->data, ch, sb->len);
}

void sb_trim_right(StringBuilder *sb)
{
  size_t i = sb->len;
  if (!is_whitespace(sb_char_at(sb, i - 1)))
  {
    return;
  }
  while (1)
  {
    if (i < 0 || !is_whitespace(sb_char_at(sb, i - 1)))
    {
      break;
    }
    i--;
  }
  sb_slice(sb, i, sb->len);
}

void sb_trim_left(StringBuilder *sb)
{
  size_t i = 0;
  if (!is_whitespace(sb_char_at(sb, i)))
  {
    return;
  }
  i++;
  while (i < sb->len)
  {
    if (!is_whitespace(sb_char_at(sb, i)))
    {
      break;
    }
    i++;
  }
  sb_slice(sb, 0, i);
}

void sb_trim(StringBuilder *sb)
{
  sb_trim_left(sb);
  sb_trim_right(sb);
}

void _compute_lsa_array(char *pat, int M, int *lps)
{
  int len = 0; // lenght of previous longest prefix suffix
  int i;

  lps[0] = 0; // lps[0] is always 0
  i = 1;

  while (i < M) // loop calculates lps[i] for i = 1 to M-1
  {
    if (pat[i] == pat[len])
    {
      len++;
      lps[i] = len;
      i++;
    }
    else // pat[i] != pat[len]
    {
      if (len != 0)
      {
        // this is tricky
        // consider the example AAACAAAA and i =7
        len = lps[len - 1];
        // we donot increment i here
      }
      else // if len == 0
      {
        lps[i] = 0;
        i++;
      }
    }
  }
}
long _kmp_Search(char *pat, char *txt)
{
  printf("%s, %s\n", pat, txt);
  int M = strlen(pat);
  int N = strlen(txt);

  int *lps = (int *)malloc(M * sizeof(int));
  int j = 0;

  _compute_lsa_array(pat, M, lps);

  int i = 0;
  long index = -1;
  while (i < N)
  {
    if (pat[j] == txt[i])
    {
      j++;
      i++;
    }
    if (j == M)
    {
      index = i - j;
      break;
      j = lps[j - 1];
    }
    else if (pat[j] != txt[i])
    {
      if (j != 0)
        j = lps[j - 1];
      else
        i++;
    }
  }
  free(lps);
  return index;
}

long sb_index_of(StringBuilder *sb, char *searchText)
{
  return _kmp_Search(searchText, sb_to_string(sb));
}

char sb_start_with(StringBuilder *sb, char *text)
{
  size_t textSize = strlen(text);
  if (textSize > sb->len)
    return 0;
  for (size_t i = 0; i < textSize; i++)
  {
    if (sb_char_at(sb, i) != text[i])
    {
      return 0;
    }
  }
  return 1;
}

char sb_end_with(StringBuilder *sb, char *text)
{
  size_t textSize = strlen(text);
  if (textSize > sb->len)
    return 0;
  for (long i = textSize - 1; i >= 0; i--)
  {
    size_t offset = textSize - i;
    if (sb_char_at(sb, sb->len - offset) != text[i])
    {
      return 0;
    }
  }
  return 1;
}

char sb_equals(StringBuilder *sb, char *compString)
{
  return sb_to_string(sb) == compString;
}

int main()
{
  StringBuilder *sb = sb_from_string("hello  ");
  // StringBuilder *sb = sb_init(64);
  // sb_push_str(sb, "    hello\n    ");
  // sb_trim(sb);
  // sb_trim_right(sb);
  // sb_trim_left(sb);
  // sb_clear(sb);
  for (int i = 0; i < 100; i++)
  {
    sb_push_str(sb, "number: ");
    sb_push_int(sb, i);
    sb_push_format(sb, " format %d", i);
    sb_push_str(sb, "\n");
  }
  // sb_delete(sb, 0, 100);
  // sb_delete(sb, 1, 3);
  sb_push_str(sb, "abc");
  // sb_delete(sb, 2, sb->len);
  // push_sb(sb, "hello world\n");
  char *searchText = "abc";
  long index = sb_index_of(sb, searchText);
  printf("index of (%s): %ld\n", searchText, index);
  // sb_slice(sb, index + 1, index + 3);
  printf("%s\nlen: %zu, cap: %zu\n", sb_to_string(sb), sb->len, sb->cap);

  printf("string start with hello: %d\n", sb_end_with(sb, "abc"));

  return 0;
}