#include "sb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

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

StringBuilder *sb_from_file(char *path)
{
  FILE *f = fopen(path, "r");
  if (f == NULL)
  {
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);

  StringBuilder *sb = sb_init_cap((size_t)size + 1);
  rewind(f);

  size_t buffer_read_size = fread(sb->data, sizeof(char), sb->cap, f);

  assert(buffer_read_size == size);
  sb->len = size;
  sb->data[sb->len] = '\0';

  fclose(f);
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

bool sb_check_capacity(StringBuilder *sb, size_t text_size)
{
  SB_VALIDATE(sb);
  if (sb->cap > sb->len + text_size)
  {
    return true;
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
    return false;
  }
  if (sb->cap < text_size)
  {
    sb->cap += text_size;
  }
  sb->data = realloc(sb->data, sb->cap * sizeof(char));
  if (!sb->data)
  {
    return false;
  }
  return true;
}

char *sb_to_string(StringBuilder *sb)
{
  if (sb == NULL)
  {
    return NULL;
  }
  sb->data[sb->len] = '\0';
  return sb->data;
}

void sb_free(StringBuilder **sb)
{
  free((*sb)->data);
  free((*sb));
  *sb = NULL;
  assert(*sb == NULL);
}

bool sb_push_str(StringBuilder *sb, char *text)
{
  SB_VALIDATE(sb);
  size_t textSize = strlen(text);
  if (!sb_check_capacity(sb, textSize + 1))
    return false;
  memcpy(sb->data + sb->len, text, textSize);
  sb->len += textSize;
  sb->data[sb->len] = '\0';
  return true;
}

bool sb_push_sb(StringBuilder *dist, StringBuilder *src)
{
  SB_VALIDATE(dist);
  SB_VALIDATE(src);
  return sb_push_str(dist, sb_to_string(src));
}

bool sb_push_sb_free(StringBuilder *dist, StringBuilder **src)
{
  SB_VALIDATE(dist);
  SB_VALIDATE(*src);
  char result = sb_push_sb(dist, *src);
  sb_free(src);
  return result;
}

bool sb_push_int(StringBuilder *sb, int number)
{
  SB_VALIDATE(sb);
  return sb_push_format(sb, "%d", number);
}

bool sb_push_format(StringBuilder *sb, const char *format, ...)
{
  SB_VALIDATE(sb);
  va_list args;
  va_start(args, format);
  char buf[4095];
  vsnprintf(&buf[0], 4095, format, args);
  va_end(args);
  return sb_push_str(sb, buf);
}

bool sb_replace_char(StringBuilder *sb, char ch, size_t index)
{
  SB_VALIDATE(sb);
  if (index < 0 || index >= sb->len)
    return false;
  sb->data[index] = ch;
  return true;
}

char sb_char_at(StringBuilder *sb, size_t index)
{
  if (sb == NULL)
    return '\0';
  sb_valid_index(sb, index, "sb_char_at");

  return sb->data[index];
}

void sb_slice(StringBuilder *sb, size_t start, size_t end)
{
  sb_valid_index(sb, start, "sb_slice");
  if (end > sb->len)
    end = sb->len;
  if (start > end)
  {
    return;
  }

  size_t removedItemsCount = end - start;
  memmove(
      sb->data + start,
      sb->data + end,
      (sb->len - end) * sizeof(char));
  sb->len -= removedItemsCount;
  sb->data[sb->len] = '\0';
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
long *_kmp_Search(char *pat, char *txt, int *count)
{
  int M = strlen(pat);
  int N = strlen(txt);

  int *lps = (int *)malloc(M * sizeof(int));
  int j = 0;

  _compute_lsa_array(pat, M, lps);

  int i = 0;
  long *index = (long *)malloc(sizeof(long) * N);
  *count = 0;
  while (i < N)
  {
    if (pat[j] == txt[i])
    {
      j++;
      i++;
    }
    if (j == M)
    {
      index[*count] = i - j;
      (*count)++;
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

long sb_index_of(StringBuilder *sb, char *search_text)
{
  int count;
  long *indexs = _kmp_Search(search_text, sb_to_string(sb), &count);
  if (count == 0)
  {
    return -1;
  }
  long index = indexs[0];
  free(indexs);
  return index;
}

long sb_last_index_of(StringBuilder *sb, char *search_text)
{
  int count;
  long *indexs = _kmp_Search(search_text, sb_to_string(sb), &count);
  if (count == 0)
  {
    return -1;
  }
  long index = indexs[count - 1];
  free(indexs);
  return index;
}

long *sb_all_index_of(StringBuilder *sb, char *search_text, int *count)
{
  return _kmp_Search(search_text, sb_to_string(sb), count);
}

bool sb_start_with(StringBuilder *sb, char *text)
{
  SB_VALIDATE(sb);
  size_t textSize = strlen(text);
  if (textSize > sb->len)
    return false;
  for (size_t i = 0; i < textSize; i++)
  {
    if (sb_char_at(sb, i) != text[i])
    {
      return false;
    }
  }
  return true;
}

bool sb_end_with(StringBuilder *sb, char *text)
{
  SB_VALIDATE(sb);
  size_t textSize = strlen(text);
  if (textSize > sb->len)
    return false;
  for (long i = textSize - 1; i >= 0; i--)
  {
    size_t offset = textSize - i;
    if (sb_char_at(sb, sb->len - offset) != text[i])
    {
      return true;
    }
  }
  return false;
}

bool sb_equals(StringBuilder *sb, char *comp_string)
{
  SB_VALIDATE(sb);
  return sb_to_string(sb) == comp_string;
}

bool sb_insert_at(StringBuilder *sb, char *text, size_t index)
{
  SB_VALIDATE(sb);
  sb_valid_index(sb, index, "sb_insert_at");
  size_t textSize = strlen(text);
  if (!sb_check_capacity(sb, textSize + 1))
    return false;
  // Shift the existing chars to right
  memmove(sb->data + index + textSize, sb->data + index, index + sb->len);
  // insert new text
  memcpy(sb->data + index, text, textSize);
  sb->len += textSize;
  sb->data[sb->len] = '\0';
  return true;
}

bool sb_reverse(StringBuilder *sb)
{
  SB_VALIDATE(sb);
  size_t left = 0;
  size_t right = sb->len - 1;

  while (left < right)
  {
    char temp = sb->data[right];
    sb->data[right] = sb->data[left];
    sb->data[left] = temp;
    left++;
    right--;
  }
  return true;
}

void sb_upper(StringBuilder *sb)
{
  SB_VALIDATE(sb);
  for (size_t i = 0; i < sb->len; ++i)
  {
    sb->data[i] = toupper((unsigned char)sb->data[i]);
  }
}

void sb_lower(StringBuilder *sb)
{
  SB_VALIDATE(sb);
  for (size_t i = 0; i < sb->len; ++i)
  {
    sb->data[i] = tolower((unsigned char)sb->data[i]);
  }
}

void sb_print(StringBuilder *sb)
{
  if (!sb || !sb->data)
  {
    printf("(null)");
    return;
  }
  printf("%s", sb_to_string(sb));
}

void sb_println(StringBuilder *sb)
{
  if (!sb || !sb->data)
  {
    printf("(null)\n");
    return;
  }
  printf("%s\n", sb_to_string(sb));
}

void sb_debug(StringBuilder *sb)
{
  if (sb == NULL)
  {
    printf("(null)\n");
    return;
  }
  printf("[DEBUG]>%s\n[DEBUG]>len: %zu, cap: %zu\n", sb_to_string(sb), sb->len, sb->cap);
}
