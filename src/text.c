/*
 *  text.c
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

char* ots_get_line_text(const OtsSentence* aLine,
                        gboolean only_if_selected,
                        size_t* out_size) {
  if (!aLine) {
    return NULL;
  }
  GString* text = g_string_new(NULL);
  GList* li;
  char* utf8_data;
  
  if (!only_if_selected || aLine->selected) {
    /* for each word in the sentence Do: */
    for (li = (GList*)aLine->words; li != NULL; li = li->next) {
      /*if word exists*/
      if (li->data && strlen(li->data)) {
        g_string_append(text, (char*)li->data);
      }
    }
  }
  if (out_size) {
    *out_size = text->len;
  }
  utf8_data = text->str;
  g_string_free(text, FALSE);
  return utf8_data;
}

static void ots_print_line(FILE* stream, const OtsSentence* aLine) {
  char* utf8_txt;
  size_t len = 0;
  utf8_txt = ots_get_line_text(aLine, TRUE, &len);
  fwrite(utf8_txt, 1, len, stream);
  g_free(utf8_txt);
}

char* ots_get_doc_text(const OtsArticle* Doc, size_t* out_len) {
  GList* li;
  GString* text = g_string_new(NULL);
  char* utf8_data;
  size_t line_len = 0;
  
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    utf8_data = ots_get_line_text((OtsSentence*)li->data, TRUE, &line_len);
    g_string_append_len(text, utf8_data, line_len);
    g_free(utf8_data);
  }
  if (out_len) {
    *out_len = text->len;
  }
  utf8_data = text->str;
  g_string_free(text, FALSE);
  return utf8_data;
}

void ots_print_doc(FILE* stream, const OtsArticle* Doc) {
  GList* li;
  /* for each line in Article Do: */
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    ots_print_line(stream, (OtsSentence*)li->data);
  }
  fputc('\n', stream);
}
