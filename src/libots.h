/*
 *  libots.h
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

#ifndef LIBOTS_H
#define LIBOTS_H

#include <stdio.h>
#include <glib/glib.h>

G_BEGIN_DECLS

/* Version number of package */
#define VERSION "0.5.0"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "ots 0.4.2"

#ifndef DEBUG
#define DEBUG 0
#endif /* DEBUG */

#ifndef PPAPI
#define PPAPI 0x1337
#endif /* PPAPI */

/* the Term Frequency data structure */
typedef struct
{
  double tf; /* Also used for TF */
  char* word;
} OtsWordTF;

typedef struct
{
  /* a GList of char* */
  GList* RemovePre;  /* (a|b)  replace string a with b */
  GList* RemovePost;
  GList* step1_pre;
  GList* step1_post;
  
  GList* synonyms;
  GList* manual;
  
  GList* ParserBreak;
  GList* ParserDontBreak;
  
  /* to be implemented */
  GList* ReplaceChars;
} OtsStemRule;

typedef struct
{
  glong score;       /* score set by the grader */
  gint wc;           /* word count */
  void* user_data;   /* pointer to the original sentence, or serial number maybe */
  GList* words;      /* a Glist of words (char*) */
  gboolean selected; /* is selected? */
} OtsSentence;

typedef struct
{
  gint lineCount;    /* lines in the text */
  GList* lines;      /* a Glist of sentences (struct Sentence) */
  char* title;       /* title , auto generated */
  
  OtsStemRule* stem; /* stemming & parsing rules */
  
  /* Term Frequency grader */
  GList* tf_terms;
  GList* idf_terms;
  
  /*Term Count grader*/
  GList* dict;       /* dictionary from xml */
  GList* wordStat;   /* a wordlist of all words in the article and their occ */
  GList* ImpWords;   /* important words - for term count grader */
} OtsArticle;

OtsArticle* ots_new_article(void);
void ots_free_article(OtsArticle* art);

/* parser */
/* file input */
#ifdef PPAPI
void ots_parse_file(const char* stream, OtsArticle* Doc);
#else
void ots_parse_file(FILE* stream, OtsArticle* Doc);
#endif /* PPAPI */

/* parse unicode stream */
void ots_parse_stream(const char* utf8 , size_t len, OtsArticle* Doc);

OtsSentence* ots_append_line(OtsArticle* Doc);
void ots_append_word(OtsSentence* aLine, const char* aWord);
void ots_add_wordstat(OtsArticle* Doc, const char* wordString);

/* dictionary */
gboolean ots_load_xml_dictionary(OtsArticle* Doc, const char* name);
gboolean ots_load_xml_dictionary_buf(OtsArticle* Doc, const char* buffer);

int ots_get_article_word_count(const OtsArticle* Doc);

/* grader */
void ots_highlight_doc(OtsArticle* Doc, int percent);     /* example: 20% */
void ots_highlight_doc_lines(OtsArticle* Doc, int lines); /* example: 10 lines */
void ots_highlight_doc_words(OtsArticle* Doc, int words); /* example: 50 words */

void ots_grade_doc(OtsArticle* Doc);

void  ots_free_OtsWordTF(OtsWordTF* obj); /* todo: put in .h file */
OtsWordTF* ots_new_OtsWordTF(unsigned const char* word, const double idf);

/* TEXT output */
void ots_print_doc(FILE* stream, const OtsArticle* Doc);

char* ots_get_doc_text(const OtsArticle* Doc, size_t* out_len);

/* Plugin writing */
char* ots_get_line_text(const OtsSentence* aLine,
                        gboolean only_if_selected,
                        size_t* out_size);
gboolean ots_is_line_selected(const OtsSentence* aLine);

/* Stemm support */
OtsStemRule* new_stem_rule(void);
void free_stem_rule(OtsStemRule* rule);

/* returns newly allocated string with the root of the word */
char* ots_stem_strip(const char* aWord, const OtsStemRule* rule);

/* Remove leading spaces, comas, colons, etc. */
char* ots_stem_format(const char* aWord, const OtsStemRule* rule);

/* Relations between texts */

/* Returns the number of topics that two blocks of text share */
int ots_text_relations(const char* text1,
                       const char* lang_code1,
                       const char* text2,
                       const char* lang_code2,
                       const int topic_num);

/* For a given text, return the list of the topics */
char* ots_text_topics(const char* text, const char* lang_code, int topic_num);

/* For a given text, return the list of the stemmed topics */
GList* ots_text_stem_list(const char* text,
                          const char* lang_code,
                          int topic_num);

/* Gives a score on the relations between two lists of topics;
simmilar to the inner product */
int ots_topic_list_score(const GList* topic_list1, const GList* topic_list2);

G_END_DECLS

#endif /* LIBOTS_H */
