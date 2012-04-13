/*
 *  parser.c
 *
 *  Copyright (C) 2003 Nadav Rotem <nadav256@hotmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *  
 *  Modified by Leon Pajk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libots.h"

#ifdef BUFFER_SIZE
#undef BUFFER_SIZE
#endif

#define BUFFER_SIZE (1024 * 8)

char ots_match_post(const char* aWord, const char* post) {
  int i;
  int wlen = strlen(aWord);
  int plen = strlen(post);
  
  if (plen > wlen) {
    return 0;
  }
  
  for (i = 0; i < plen; i++) {
    if (aWord[wlen - plen + i] != post[i]) {
      return 0;	/* no match */
    }
  }
  return 1;     /*word match */
}

#ifdef PPAPI
void ots_parse_file(const char* buffer, OtsArticle* Doc) {
  ots_parse_stream(buffer, strlen(buffer), Doc);
}
#else
void ots_parse_file(FILE* stream, OtsArticle* Doc) {
  char fread_buffer[BUFFER_SIZE];
  char* buffer      = g_new0(char, BUFFER_SIZE);
  size_t nread      = 0;
  size_t total_read = 0;
  size_t avail_size = BUFFER_SIZE;
  
  while ((nread = fread(fread_buffer,
                        sizeof(char),
                        sizeof(fread_buffer),
                        stream)) > 0) {
    if (nread + total_read > avail_size) {
      avail_size <<= 1;
      buffer = g_renew(char, buffer, avail_size);
    }
    
    strncpy(buffer + total_read, fread_buffer, nread);
    total_read += nread;
  }
  ots_parse_stream(buffer, total_read, Doc);
  g_free(buffer);
}
#endif

char ots_parser_should_break(const char* aWord, const OtsStemRule* rule) {
  char toBreak = 0;
  GList* li;
  char* postfix;
  
  for (li = (GList*)rule->ParserBreak; li != NULL; li = li->next) {
    postfix = li->data;
    if (ots_match_post(aWord, postfix)) {
      toBreak = 1;
      break;
    }
  }
  
  for (li = (GList*)rule->ParserDontBreak; li != NULL; li = li->next) {
    postfix = li->data;
    if (ots_match_post(aWord, postfix)) {
      toBreak = 0;
      break;
    }
  }
  return toBreak;
}

void ots_parse_stream(const char* utf8, size_t len, OtsArticle* Doc) {
  /*parse the unicode stream */
  OtsSentence* tmpLine = ots_append_line(Doc);
  OtsStemRule* rule    = Doc->stem;
  gunichar uc;
  int index = 0;
  char* s   = (char*)utf8;
  GString* word_buffer = g_string_new(NULL);
  
  while ((*s) && (index < len)) {
    uc = g_utf8_get_char(s);
    if (!g_unichar_isspace(uc)) {
      /* space is the end of a word */
      g_string_append_unichar(word_buffer, uc);
    } else  {
      if (0 < word_buffer->len) {
        ots_append_word(tmpLine, word_buffer->str);
        if (ots_parser_should_break(word_buffer->str, rule)) {
          tmpLine = ots_append_line(Doc);	/* Add a new line */
        }
        g_string_assign(word_buffer, "");
      }
      
      if (uc == '\n') {
        ots_append_word(tmpLine, "\n");
      } else {
        ots_append_word(tmpLine, " ");
      }
      g_string_assign(word_buffer, "");
    }
    s = g_utf8_next_char(s);
    ++index;
  }
  if (0 < word_buffer->len) {
    /*final flush*/
    ots_append_word(tmpLine, word_buffer->str);
    g_string_assign(word_buffer, "");
  }
  g_string_free(word_buffer, TRUE);
}
