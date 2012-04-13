/*
 *  wordlist.c
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
#include "grader-tc.h"

/*word lists manipulations , mainly for grader-tc */
OtsWordEntery* ots_new_wordEntery_strip(const char* wordString,
                                        const OtsStemRule* rule) {
  /*for real text use*/
  OtsWordEntery* aWord = g_new0(OtsWordEntery, 1);
  aWord->occ  = 1;
  aWord->word = ots_stem_format(wordString, rule);
  aWord->stem = ots_stem_strip(wordString, rule);
  return aWord;
}

OtsWordEntery* ots_new_wordEntery(unsigned const char* wordString) {
  /*for dictionary use only, no formating here*/
  OtsWordEntery* aWord = g_new0(OtsWordEntery, 1);
  aWord->occ  = 1;
  aWord->word = g_strdup((char*)wordString);
  aWord->stem = g_strdup((char*)wordString);
  return aWord;
}

void ots_free_wordEntery(OtsWordEntery* we) {
  if (NULL != we) {
    if (NULL != we->word) {
      g_free(we->word);
    }
    if (NULL != we->stem) {
      g_free(we->stem);
    }
    g_free(we);
  }
}

void ots_free_wordlist(GList* aList) {
  if (NULL != aList) {
    g_list_foreach(aList, (GFunc)ots_free_wordEntery, NULL);
    g_list_free(aList);
  } 
}

OtsWordEntery* ots_copy_wordEntery(OtsWordEntery* obj) {
  if (NULL == obj) {
    return NULL;
  }
  OtsWordEntery* aWord = g_new(OtsWordEntery, 1);
  aWord->occ  = obj->occ;
  aWord->word = g_strdup(obj->word);
  
  if (NULL != obj->stem) {
    aWord->stem = g_strdup(obj->stem);
  } else {
    aWord->stem = NULL;
  }
  return aWord;
}

static int ots_sort_handler(OtsWordEntery* node1, OtsWordEntery* node2) {
  if (node1->occ > node2->occ) {
    return -1;
  }
  if (node1->occ < node2->occ) {
    return 1;
  }
  return 0;
}

/* sort article */
GList* ots_sort_list(GList* aList) {
  GList* newList = g_list_sort(aList, (GCompareFunc)ots_sort_handler);
  return newList;
}

GList* ots_union_list(const GList* aLst, const GList* bLst) {
  int insert;
  GList* li;
  GList* di;
  GList* newLst = NULL;
  
  for (li = (GList*)aLst; li != NULL; li = li->next) {
    insert = 1;
    for (di = (GList*)bLst; di != NULL; di = di->next) {
      if(li->data && di->data &&
         ((OtsWordEntery*)li->data)->word &&
         ((OtsWordEntery*)di->data)->word && /* all defined? */
         (0 == g_strncasecmp((((OtsWordEntery*)li->data)->word), /*fix me: unicode issue?*/
                             (((OtsWordEntery*)di->data)->word), 10))) {
        insert = 0; /* if word in B */
      }
    }
    if (1 == insert && li->data) {
      newLst =
        g_list_append(newLst, ots_copy_wordEntery((OtsWordEntery*)li->data));
    }
  }
  return newLst;
}

/* return the String value of the n'th word */
char* ots_word_in_list(const GList* aList, const int index) {
  OtsWordEntery* obj = NULL;
  GList* item = (GList*)g_list_nth((GList*)aList, index);
  if (NULL != item) {
    obj = item->data;
  }
  if (NULL != obj) {
    return obj->word;
  } else {
    return NULL;
  }
}

/* return the String value of stem of the n'th word */
char* ots_stem_in_list(const GList* aList, const int index) {
  OtsWordEntery* obj = NULL;
  GList* item = (GList*)g_list_nth((GList*)aList, index);
  if (NULL != item) {
    obj = item->data;
  }
  if (NULL != obj) {
    return obj->stem;
  } else {
    return NULL;
  }
}

/*Adds a word to the word count of the article*/
void ots_add_wordstat(OtsArticle* Doc, const char* wordString) {
  if ((NULL == Doc) || (NULL == wordString) ||
      (0 == strlen(wordString))       ||
      (0 == strcmp(wordString, " "))  ||
      (0 == strcmp(wordString, "\n")) ||
      (0 == strcmp(wordString, "\t"))) {
    return;
  }
  GList* li;
  OtsWordEntery* stat;
  OtsStemRule* rule = Doc->stem;
  char* tmp = NULL;
  
  if (wordString) {
    tmp = ots_stem_strip(wordString, rule);
  }
  
  for (li = (GList*)Doc->wordStat; li != NULL; li = li->next) {
    /* search the word in current wordlist */
    if (li->data && (0 == strcmp(tmp, ((OtsWordEntery*)li->data)->stem))) {
      /* occurred in another place in the text now; */
      ((OtsWordEntery*)li->data)->occ++;
      g_free(tmp);
      
      /*printf for debug*/
      /* 
      if (0 != strcmp((OtsWordEntery*)li->data)->word,wordString))
      printf("[%s]==[%s]\n",((OtsWordEntery*)li->data)->word,wordString);
      */
      return;
    }
  }
  /* if not in list , Add  stem  it to the list */
  stat = ots_new_wordEntery_strip(wordString, rule);
  if (stat) {
    Doc->wordStat = g_list_prepend(Doc->wordStat, stat);
  }
  g_free(tmp);
  return;
}

void ots_print_wordlist(FILE* stream, const GList* aList) {
  GList* li;
  for (li = (GList*)aList; li != NULL; li = li->next) {
    fprintf(stream, "Word[%d][%s]\n",
            ((OtsWordEntery*)li->data)->occ,
            ((OtsWordEntery*)li->data)->word);
  }
}
