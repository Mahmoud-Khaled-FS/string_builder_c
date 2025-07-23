#ifndef SB_H
#define SB_H

#include <stdio.h>

#define ALPHA_CHARS 256
#define THRESHOLD_SIZE (1024 * 1024)
#define INITIAL_CAPACITY 128

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')

typedef struct
{
  char *data;
  size_t len;
  size_t cap;
} StringBuilder;

StringBuilder *sb_init_cap(size_t cap);
StringBuilder *sb_init();
StringBuilder *sb_from_string(char *string);

void sb_valid_index(StringBuilder *sb, size_t index, char *fn_name);

char sb_check_capacity(StringBuilder *sb, size_t textSize);

char *sb_to_string(StringBuilder *sb);

void sb_free(StringBuilder *sb);

int sb_push_str(StringBuilder *sb, char *text);

int sb_push_int(StringBuilder *sb, int number);

int sb_push_format(StringBuilder *sb, const char *format, ...);

int sb_replace_char(StringBuilder *sb, char ch, size_t index);

char sb_char_at(StringBuilder *sb, size_t index);

void sb_slice(StringBuilder *sb, size_t start, size_t end);

void sb_clear(StringBuilder *sb);

void sb_set(StringBuilder *sb, char ch);

void sb_trim_right(StringBuilder *sb);

void sb_trim_left(StringBuilder *sb);

void sb_trim(StringBuilder *sb);

long sb_index_of(StringBuilder *sb, char *searchText);

char sb_start_with(StringBuilder *sb, char *text);

char sb_end_with(StringBuilder *sb, char *text);

char sb_equals(StringBuilder *sb, char *compString);
#endif