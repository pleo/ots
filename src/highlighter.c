/*
 *  highlighter
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

/* After the grader has graded the article and each
 sentence has a score the highlighter will select
 some of the sentences */
static int ots_highlight_max_line(OtsArticle* Doc) {
  GList* li;
  int max = 0;
  
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    /* if not selected, count me in */
    if (0 == ((OtsSentence*)li->data)->selected) {
      max = MAX(max, ((OtsSentence*)li->data)->score);
    }
  }
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    if ((max == ((OtsSentence*)li->data)->score) &&
        (0 == ((OtsSentence*)li->data)->selected)) {
      /* if score==max && not selected before ,select me; */
      ((OtsSentence*)li->data)->selected = 1;
      return ((OtsSentence*)li->data)->wc;
    }
  }
  return 0;
}

/* todo: impement this
void ots_highlight_doc_wordcount(OtsArticle* Doc, int wordCount)
void ots_highlight_doc_linecount(OtsArticle* Doc, int wordCount)
//blur selection by avrage of near sentences , will mark blocks
void ots_highlight_doc_soft(OtsArticle* Doc, int percent)
*/

void ots_highlight_doc(OtsArticle* Doc, int percent) {
  if (0 == Doc->lineCount) {
    return;
  }
  
  if (100 < percent) {
    percent = 100;
  } else if (0 > percent) {
    percent = 0;
  }
  
  double ratio  = 0.01 * percent;
  int wordCount = ots_get_article_word_count(Doc);
  int i;
  
  for (i = 0; i < (ratio * (double)wordCount);) {
    i += ots_highlight_max_line(Doc);
  }
}

void ots_highlight_doc_lines(OtsArticle* Doc, int lines) {
  if (0 == Doc->lineCount) {
    return;
  }
  int lineCount = Doc->lineCount,
      i = 0,
      tmp;
  
  while ((i < lines) && (i < lineCount)) {
    i++;
    tmp = ots_highlight_max_line(Doc);
  }
}

void ots_highlight_doc_words(OtsArticle* Doc, int words) {
  if (0 == Doc->lineCount) {
    return;
  }
  int docWordCount = ots_get_article_word_count(Doc);
  int i = 0;
  
  while ((i < docWordCount) && (i <= words)) {
    i += ots_highlight_max_line(Doc);
  }
}
