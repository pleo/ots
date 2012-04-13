/*
 *  dictionary.c
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

#ifdef PPAPI
#include <libroxml/roxml-internal.h>
#else
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#endif /* PPAPI */

#ifndef PPAPI
/* loads the xml dictionary file to memory*/
gboolean ots_load_xml_dictionary(OtsArticle* Doc, const char* name) {
  xmlDocPtr  doc        = NULL;
  xmlNodePtr head       = NULL;
  xmlNodePtr stem       = NULL;
  xmlNodePtr pre        = NULL;
  xmlNodePtr post       = NULL;
  xmlNodePtr syno       = NULL;     /* synonyms     */
  xmlNodePtr manual     = NULL;     /* manual  */
  xmlNodePtr step1_pre  = NULL;     /* step1  */
  xmlNodePtr step1_post = NULL;     /* step1  */
  
  xmlNodePtr parse   = NULL;        /* parser rules */
  xmlNodePtr pbreak  = NULL;
  xmlNodePtr pdbreak = NULL;
  
  xmlNodePtr tc_words = NULL;       /* term count dictionary   */
  xmlNodePtr tf_words = NULL;       /* term frequency dictionary   */
  
  OtsStemRule* rule = Doc->stem;
  
  char* dict_name;
  char* local_dict_name;
  
  dict_name = g_strdup_printf("%s%s.xml", DICTIONARY_DIR, name);
  local_dict_name = g_strdup_printf("%s.xml", name);
  
  if (g_file_test(local_dict_name,G_FILE_TEST_EXISTS)) {
    doc = xmlParseFile(local_dict_name);
    /* it warns to the screen so we cant use it; enable for web services only */
  }
  if (doc == NULL) {
    doc = xmlParseFile(dict_name);
    if (doc == NULL) {
      return (FALSE);
    }
  }
  
  head = xmlDocGetRootElement(doc);
  if (head == NULL) {
    fprintf(stderr, "empty document\n");
    xmlFreeDoc(doc);
    return (FALSE);
  }
  
  if (xmlStrcmp(head->name, (const xmlChar*)"dictionary")) {
    fprintf(stderr, "%s", head->name);
    xmlFreeDoc(doc);
    return (FALSE);
  }
  
  if (head != NULL) {
    stem = head->xmlChildrenNode;
  }
  while ((stem != NULL) &&
         xmlStrcmp(stem->name, (const xmlChar*)"stemmer")) {
    stem = stem->next;
  }
  
  if (head != NULL) {
    parse = head->xmlChildrenNode;
  }
  while ((parse != NULL) &&
         xmlStrcmp(parse->name, (const xmlChar*)"parser")) {
    parse = parse->next;
  }
  
  if (head != NULL) {
    tc_words = head->xmlChildrenNode;
  }
  while ((tc_words != NULL) &&
         xmlStrcmp(tc_words->name, (const xmlChar*)"grader-tc")) {
      tc_words = tc_words->next;
  }
  
  if (head != NULL) {
    tf_words = head->xmlChildrenNode;
  }
  while ((tf_words != NULL) &&
         xmlStrcmp(tf_words->name, (const xmlChar*)"grader-tf")) {
      tf_words = tf_words->next;
  }
  
  if (stem != NULL) {
    pre = stem->xmlChildrenNode;
  }
  while ((pre != NULL) && xmlStrcmp(pre->name, (const xmlChar*)"pre")) {
    pre = pre->next;
  }
  
  if (stem != NULL) {
    post = stem->xmlChildrenNode;
  }
  while ((post != NULL) && xmlStrcmp(post->name, (const xmlChar*)"post")) {
    post = post->next;
  }
  
  if (stem != NULL) {
    syno = stem->xmlChildrenNode;
  }
  while ((syno != NULL) &&
         xmlStrcmp(syno->name, (const xmlChar*)"synonyms")) {
    syno = syno->next;
  }
  
  if (stem != NULL) {
    manual = stem->xmlChildrenNode;
  }
  while ((manual != NULL) &&
         xmlStrcmp(manual->name, (const xmlChar*)"manual")) {
    manual = manual->next;
  }
  
  if (stem != NULL) {
    step1_pre = stem->xmlChildrenNode;
  }
  while ((step1_pre != NULL) &&
         xmlStrcmp(step1_pre->name, (const xmlChar*)"step1_pre")) {
    step1_pre = step1_pre->next;
  }
  
  if (stem != NULL) {
    step1_post = stem->xmlChildrenNode;
  }
  while ((step1_post != NULL) &&
         xmlStrcmp(step1_post->name, (const xmlChar*)"step1_post")) {
    step1_post = step1_post->next;
  }
  
  if (pre != NULL) {
    pre = pre->xmlChildrenNode; /*point to first word */
  }
  while (pre != NULL) {
    if (0 == xmlStrcmp(pre->name, (const xmlChar*)"rule")) {
    rule->RemovePre =
        g_list_append(rule->RemovePre,
                      xmlNodeListGetString(doc, pre->xmlChildrenNode, 1));
    }
    pre = pre->next;
  }
  
  if (post != NULL) {
    post = post->xmlChildrenNode;
  }
  while (post != NULL) {
    if (0 == xmlStrcmp(post->name, (const xmlChar*)"rule")) {
      rule->RemovePost =
        g_list_append(rule->RemovePost,
                      xmlNodeListGetString(doc, post->xmlChildrenNode, 1));
    }
    post = post->next;
  }
  
  if (syno != NULL) {
    syno = syno->xmlChildrenNode;
  }
  while (syno != NULL) {
    if (0 == xmlStrcmp(syno->name, (const xmlChar*)"rule")) {
      rule->synonyms =
        g_list_append(rule->synonyms,
                      xmlNodeListGetString(doc, syno->xmlChildrenNode, 1));
    }
    syno = syno->next;
  }
  if (manual != NULL) {
    manual = manual->xmlChildrenNode;
  }
  while (manual != NULL) {
    if (0 == xmlStrcmp(manual->name, (const xmlChar*)"rule")) {
      rule->manual =
        g_list_append(rule->manual,
                      xmlNodeListGetString(doc, manual->xmlChildrenNode, 1));
    }
    manual = manual->next;
  }
  
  if (step1_pre != NULL) {
    step1_pre = step1_pre->xmlChildrenNode;
  }
  while (step1_pre != NULL) {
    if (0 == xmlStrcmp(step1_pre->name, (const xmlChar*)"rule")) {
      rule->step1_pre =
        g_list_append(rule->step1_pre,
          xmlNodeListGetString(doc, step1_pre->xmlChildrenNode, 1));
    }
    step1_pre = step1_pre->next;
  }
  
  if (step1_post != NULL) {
    step1_post = step1_post->xmlChildrenNode;
  }
  while (step1_post != NULL) {
    if (0 == xmlStrcmp(step1_post->name, (const xmlChar*)"rule")) {
      rule->step1_post =
        g_list_append(rule->step1_post,
          xmlNodeListGetString(doc, step1_post->xmlChildrenNode, 1));
    }
    step1_post = step1_post->next;
  }
  
  if (parse != NULL) {
    pbreak = parse->xmlChildrenNode;
  }
  while ((pbreak != NULL) &&
         xmlStrcmp(pbreak->name, (const xmlChar*)"linebreak")) {
    pbreak = pbreak->next;
  }
  
  if (parse != NULL) {
    pdbreak = parse->xmlChildrenNode;
  }
  while ((pdbreak != NULL) &&
         xmlStrcmp(pdbreak->name, (const xmlChar*)"linedontbreak")) {
    pdbreak = pdbreak->next;
  }
  
  /*Parser break*/
  if (pbreak != NULL) {
    pbreak = pbreak->xmlChildrenNode;
  }
  while (pbreak != NULL) {
    if (0 == xmlStrcmp(pbreak->name, (const xmlChar*)"rule")) {
      rule->ParserBreak =
        g_list_append(rule->ParserBreak,
                      xmlNodeListGetString(doc, pbreak->xmlChildrenNode, 1));
    }
    pbreak = pbreak->next;
  }
  
  /*Parser Don't break*/
  if (pdbreak != NULL) {
    pdbreak = pdbreak->xmlChildrenNode;
  }
  while (pdbreak != NULL) {
    if (0 == xmlStrcmp(pdbreak->name, (const xmlChar*)"rule")) {
      rule->ParserDontBreak =
        g_list_append(rule->ParserDontBreak,
                      xmlNodeListGetString(doc, pdbreak->xmlChildrenNode, 1));
    }
    pdbreak = pdbreak->next;
  }
  
  /* Term Count load dict */
  if (tc_words != NULL) {
    tc_words = tc_words->xmlChildrenNode;
  }
  while (tc_words != NULL) {
    if (0 == xmlStrcmp(tc_words->name, (const xmlChar*)"word")) {
      const xmlChar* key =
        xmlNodeListGetString(doc, tc_words->xmlChildrenNode, 1);
      Doc->dict = g_list_append(Doc->dict, (gpointer)ots_new_wordEntery(key));
      xmlFree((xmlChar*)key);
    }
    tc_words = tc_words->next;
  }
  
  /*Term Frequency load dict*/
  if (tf_words != NULL) {
    tf_words = tf_words->xmlChildrenNode;
  }
  while (tf_words != NULL) {
    if (0 == xmlStrcmp(tf_words->name, (const xmlChar*)"word")) {
      const xmlChar* key =
        xmlNodeListGetString(doc, tf_words->xmlChildrenNode, 1);
      const xmlChar* idf_key = xmlGetProp(tf_words, (const xmlChar*)"idf");
      Doc->tf_terms = g_list_append(Doc->tf_terms,
        ots_new_OtsWordTF(key, atof((const char*)idf_key)));
      xmlFree((xmlChar*)key);
      xmlFree((xmlChar*)idf_key);
    }
    tf_words = tf_words->next;
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();
  g_free(dict_name);
  g_free(local_dict_name);
  return (TRUE);
}
#endif /* PPAPI */

#ifdef PPAPI

static inline void ots_get_xml_node(node_t** node, const char* name) {
#if DEBUG > 1
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(*node, NULL, 0));
#endif /* DEBUG > 1 */
  while ((*node != NULL) &&
         g_strcmp0(roxml_get_name(*node, NULL, 0), (const char*)name)) {
    *node = roxml_get_next_sibling(*node);
#if DEBUG > 2
    fprintf(stdout, "%s\n", roxml_get_name(*node, NULL, 0));
#endif /* DEBUG > 2 */
  }
#if DEBUG > 0
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(*node, NULL, 0));
#endif /* DEBUG > 0 */
}

static inline void ots_append_xml_node_contents(GList** list, node_t** node) {
#if DEBUG > 1
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(*node, NULL, 0));
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_content(*node, NULL, 0, NULL));
#endif /* DEBUG > 1 */
  while (*node != NULL) {
    if (!g_strcmp0(roxml_get_name(*node, NULL, 0), (const char*)"rule")) {
      *list = g_list_append(*list,
        g_strdup(roxml_get_content(*node, NULL, 0, NULL)));
#if DEBUG > 2
  fprintf(stdout, "%s\n", roxml_get_content(*node, NULL, 0, NULL));
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(*node, NULL, 0));
#endif /* DEBUG > 2 */
    }
    *node = roxml_get_next_sibling((*node));
  }
}

gboolean ots_load_xml_dictionary_buf(OtsArticle* Doc, const char* buffer) {
  char* head_name    = NULL;
  node_t* stem       = NULL;
  node_t* pre        = NULL;
  node_t* post       = NULL;
  node_t* syno       = NULL;        /* synonyms */
  node_t* manual     = NULL;        /* manual   */
  node_t* step1_pre  = NULL;        /* step1    */
  node_t* step1_post = NULL;        /* step1    */
  
  node_t* parse   = NULL;           /* parser rules */
  node_t* pbreak  = NULL;
  node_t* pdbreak = NULL;
  
  node_t* tc_words = NULL;          /* term count dictionary   */
  node_t* tf_words = NULL;          /* term frequency dictionary   */
  
  OtsStemRule* rule = Doc->stem;
  node_t* head = roxml_load_buf((char*)buffer);
  
  if (head == NULL) {
    fprintf(stderr, "empty document\n");
    roxml_release(RELEASE_ALL);
    return (FALSE);
  }
#if DEBUG > 1
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(head, NULL, 0));
#endif /* DEBUG > 1 */
  if (g_strcmp0(roxml_get_name(head, head_name, 0), (const char*)"dictionary")) {
    fprintf(stderr, "%s\n", head_name);
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  
  stem = roxml_get_chld(head, NULL, 0);
  if(stem == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&stem, (const char*)"stemmer");
  
  parse = roxml_get_chld(head, NULL, 0);
  if(parse == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&parse, (const char*)"parser");
  
  tc_words = roxml_get_chld(head, NULL, 0);
  if(tc_words == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&tc_words, (const char*)"grader-tc");
  
  tf_words = roxml_get_chld(head, NULL,  0);
  if(tf_words == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&tf_words, (const char*)"grader-tf");
  
  pre = roxml_get_chld(stem, NULL,  0);
  if(pre == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&pre, (const char*)"pre");
  
  post = roxml_get_chld(stem, NULL,  0);
  if(post == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&post, (const char*)"post");
  
  syno = roxml_get_chld(stem, NULL, 0);
  if(syno == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&syno, (const char*)"synonyms");
  
  manual = roxml_get_chld(stem, NULL, 0);
  if(manual == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&manual, (const char*)"manual");
  
  step1_pre = roxml_get_chld(stem, NULL, 0);
  if(step1_pre == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&step1_pre, (const char*)"step1_pre");
  
  step1_post = roxml_get_chld(stem, NULL, 0);
  if(step1_post == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&step1_post, (const char*)"step1_post");
  
  pre = roxml_get_chld(pre, NULL, 0); /* point to first word */
  if(pre == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->RemovePre, &pre);
  
  post = roxml_get_chld(post, NULL, 0);
  if(post == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->RemovePost, &post);
  
  syno = roxml_get_chld(syno, NULL, 0);
  if(syno == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->synonyms, &syno);
  
  manual = roxml_get_chld(manual, NULL, 0);
  if(manual == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->manual, &manual);
  
  step1_pre = roxml_get_chld(step1_pre, NULL, 0);
  if(step1_pre == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->step1_pre, &step1_pre);
  
  step1_post = roxml_get_chld(step1_post, NULL, 0);
  if(step1_post == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->step1_post, &step1_post);
  
  pbreak = roxml_get_chld(parse, NULL, 0);
  if(pbreak == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&pbreak, (const char*)"linebreak");
  
  pdbreak = roxml_get_chld(parse, NULL, 0);
  if(pdbreak == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_get_xml_node(&pdbreak, (const char*)"linedontbreak");
  
  /*Parser break*/
  pbreak = roxml_get_chld(pbreak, NULL, 0);
  if(pbreak == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->ParserBreak, &pbreak);
  
  /*Parser Don't break*/
  pdbreak = roxml_get_chld(pdbreak, NULL, 0);
  if(pdbreak == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
  ots_append_xml_node_contents(&rule->ParserDontBreak, &pdbreak);
  
  /* Term Count load dict */
  tc_words = roxml_get_chld(tc_words, NULL, 0);
  if(tc_words == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
#if DEBUG > 1
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(tc_words, NULL, 0));
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_content(tc_words, NULL, 0, NULL));
#endif /* DEBUG > 1 */
  while (tc_words != NULL) {
    if (!g_strcmp0(roxml_get_name(tc_words, NULL, 0), (const char*)"word")) {
      Doc->dict = g_list_prepend(Doc->dict,
        (gpointer)ots_new_wordEntery(
          (unsigned const char*)roxml_get_content(tc_words, NULL, 0, NULL)));
    }
    tc_words = roxml_get_next_sibling(tc_words);
#if DEBUG > 2
    fprintf(stdout, "%s\n", roxml_get_content(tc_words, NULL, 0, NULL));
#endif /* DEBUG > 2 */
  }
  
  /*Term Frequency load dict*/
  tf_words = roxml_get_chld(tf_words, NULL, 0);
  if(tf_words == NULL) {
    roxml_release(RELEASE_ALL);
    roxml_close(head);
    return (FALSE);
  }
#if DEBUG > 1
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_name(tf_words, NULL, 0));
  fprintf(stdout, "%d: %s\n", __LINE__, roxml_get_content(tf_words, NULL, 0, NULL));
#endif /* DEBUG > 1 */
  while (tf_words != NULL) {
    if (!g_strcmp0(roxml_get_name(tf_words, NULL, 0), (const char*)"word")) {
      const gchar* key = g_strdup(roxml_get_content(tf_words, NULL, 0, NULL));
      node_t* idf_attr = roxml_get_attr(tf_words, "idf", 0);
      if (idf_attr != NULL) {
        const char* idf_val = roxml_get_content(idf_attr, NULL, 0, NULL);
        if (idf_val != NULL) {
#if DEBUG > 3
          fprintf(stdout, "%s\n", idf_val);
#endif /* DEBUG > 3 */
          Doc->tf_terms = g_list_append(Doc->tf_terms,
          ots_new_OtsWordTF((const unsigned char*)key, atof(idf_val)));
        }
      }
      g_free((gpointer)key);
    }
    tf_words = roxml_get_next_sibling(tf_words);
#if DEBUG > 2
    fprintf(stdout, "%s\n", roxml_get_content(tf_words, NULL, 0, NULL));
#endif /* DEBUG > 2 */
  }
  roxml_release(RELEASE_ALL);
  roxml_close(head);
  return (TRUE);
}
#endif /* PPAPI */
