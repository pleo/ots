/*
 *  grader.c
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

extern void ots_grade_doc_tc(OtsArticle* Doc);

/*Grader driver - will call one of the grading algorithm*/
/*must be called after the first grader*/

void ots_grade_structure(OtsArticle* Doc) {
  if (NULL == Doc) {
    return;
  }
  GList* li;
  GList* first  = NULL;
  GList* second = NULL;
  OtsSentence* first_line = NULL;
  
  if (NULL != Doc->lines) {
    first_line = (OtsSentence*)(Doc->lines->data);
  }
  if (NULL != first_line) {
    /* first line/title is very important so we increase its score */
    first_line->score *= 2;
  }
  /* This loop will * 1.6 the score of each line that
     starts with \n \n , in other words a new paragraph */
  for (li = (GList*)Doc->lines; li != NULL; li = li->next) {
    OtsSentence* aLine = (li->data);
    if (NULL != aLine) {      /*line is there */
      first = aLine->words;	  /*first word? */
      if (NULL != first) {
        second = first->next; /*second word? */
        /*have content? */
      if ((NULL != second) && strcmp(first->data, "\n") &&
            strcmp(second->data, "\n")) {
          /* new paragraph? */
        aLine->score *= 1.6;
    }
      }
    }
  }
}

/** 
Each grader needs to do:
1.give a ->score to each line 
2.Set the ->title of the document
**/
void ots_grade_doc(OtsArticle* Doc) {
  if (NULL == Doc) {
    return;
  }
  /*Term count*/
  ots_grade_doc_tc(Doc);
  
  /* or ots_grade_doc_fc(Doc); Term Frequency */
  ots_grade_structure(Doc);
}
