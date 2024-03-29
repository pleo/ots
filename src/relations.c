/*
 *  relations.c
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
#include "grader-tc.h"
#include "libots.h"
/*
The Inner product of two texts is defined as the number of topics they
share. This set of functions implements this relations using the ots
api.

Application: a relation between a slashdot article and a comment made
usage: ots_text_relations(story,"en",comment,"en",n);
where n is the max number of most important topics to consider;
safe to give a high number (ex: 20);

returns: 
  0 - off topic
  n - number of topics they share

*/

#define OTS_MAX_TOPIC_WORD_SIZE 256

/*Returns the number of topics that two blocks of text share*/
int ots_text_relations(const char* text1,
                       const char* lang_code1,
                       const char* text2,
                       const char* lang_code2,
                       const int topic_num) {
  GList* top1 = ots_text_stem_list(text1, lang_code1, topic_num);
  GList* top2 = ots_text_stem_list(text2, lang_code2, topic_num);
  int score   = ots_topic_list_score(top1, top2);
  
  if (top1) {
    g_list_foreach(top1, (GFunc)g_free, NULL);
    g_list_free(top1);
  }
  if (top2) {
    g_list_foreach(top2, (GFunc)g_free, NULL);
    g_list_free(top2);
  }
  return score;
}

/*For a given text, return the list of the topics*/
char* ots_text_topics(const char* text,
                      const char* lang_code,
                      int topic_num) {
  if (NULL==text) {
    return NULL;
  }
  int i;
  char* str;
  char* tmp;
  GString* word   = g_string_new(NULL);
  OtsArticle* Art = ots_new_article();
  
  /*Load the dictionary*/
#ifndef PPAPI
  ots_load_xml_dictionary(Art, lang_code);
#else
  ots_load_xml_dictionary_buf(Art, lang_code);
#endif /* PPAPI */
  
  if (text!=NULL) {
    /* read text , put it in struct Article */
    ots_parse_stream(text,strlen(text), Art);
  }
  ots_grade_doc(Art);
  
  for (i = 0; i <= topic_num; i++) {
    tmp = ots_word_in_list(Art->ImpWords, i);
    if ((NULL != tmp) && (0 < strlen(tmp))) {
      g_string_append(word, tmp);
      g_string_append(word, " ");
    }
  }
  str = word->str;
  g_string_free(word, FALSE);
  ots_free_article(Art);
  return str;
}

/* For a given text, return the list of the stemmed topics */
GList* ots_text_stem_list(const char* text,
                          const char* lang_code,
                          int topic_num) {
  if (NULL == text) {
    return NULL;
  }
  int i;
  char* tmp;
  GList* topics = NULL;
  OtsArticle* Art = ots_new_article();
  
#ifndef PPAPI
  ots_load_xml_dictionary(Art, lang_code);
#else
  ots_load_xml_dictionary_buf(Art, lang_code);
#endif /* PPAPI */
  
  if (NULL != text) {
    ots_parse_stream(text, strlen(text), Art);
  }
  
  ots_grade_doc(Art);
  
  for (i = 0; i <= topic_num; i++) {
    tmp = ots_stem_in_list(Art->ImpWords, i);
    if (tmp && (strlen(tmp) > 0)) {
      topics = g_list_append(topics, g_strdup(tmp));
    }
  }
  ots_free_article(Art);
  return topics;
}

/* Gives a score on the relations between two lists of topics; simmilar to the inner product */
int ots_topic_list_score(const GList* topic_list1,
                         const GList* topic_list2) {
  if (!(topic_list1) || !(topic_list2)) {
    return 0;
  }
  int count = 0;
  GList* tmplist1 = g_list_first((GList*)topic_list1);
  GList* tmplist2;
  
  while(tmplist1) {
    tmplist2 = g_list_first((GList*)topic_list2);
  while(tmplist2) {
      if (tmplist1->data && tmplist2->data && (strlen(tmplist2->data) > 1) &&
          (0 == strncmp(tmplist1->data,
                        tmplist2->data,
                        OTS_MAX_TOPIC_WORD_SIZE))) {
        count++;
      }
      tmplist2 = g_list_next(tmplist2);
    }
  tmplist1 = g_list_next(tmplist1);
  }
  return count;
}
