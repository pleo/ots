/*
 *  grader-tf.c
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
#include <math.h>
#include "libots.h"

/* Grader - using the Term frequency algorithm. Will give each line a score */
OtsWordTF* ots_new_OtsWordTF(unsigned const char* word, const double tf) {
  OtsWordTF* obj = g_new0(OtsWordTF, 1);
  if (NULL != word) {
    obj->word = g_strdup((char*)word);
  }
  obj->tf = tf;
  return obj;
}

void ots_free_OtsWordTF(OtsWordTF* obj) {
  if (NULL != obj) {
    if (NULL != obj->word) {
      g_free(obj->word);
    }
    g_free(obj);
  }
}

void ots_free_TF_wordlist(GList* aList) {
  if (NULL != aList) {
    g_list_foreach(aList,(GFunc)ots_free_OtsWordTF, NULL);
    g_list_free(aList);
  }
}

void ots_grade_line_tf(OtsSentence* aLine) {
  return;
}

void ots_grade_doc_tf(OtsArticle* Doc) {
  if (0 == Doc->lineCount) {
    return;
  }
  /*Load tf list*/
  /*Load idf list*/
  GList* li;
  
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    ots_grade_line_tf((OtsSentence*)li->data /* , tf list , idf list*/);
  }
  return; 
}


double ots_tf_word_score(const double tf, const double idf) {
  /*IDF: how rare is word across the collection
  TF: how often is word in doc */
  return tf * idf;
}

/*
Determine frequency of query words
n = (num-of-sentences words appears in) 
N = (total-number-of-sentences)
f = n/N
*/
double ots_calc_idf(const int term_count, const int doc_word_count) {
  return -log(doc_word_count / term_count);
}

double ots_calc_tf(const int term_count, const int doc_word_count) {
  if (0 == term_count) {
    return 0;
  } else {
    return 0.5 + 0.5 * (doc_word_count / term_count);
  }
}
