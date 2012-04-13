/*
 *  ots.c
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
#include <glib/glib.h>
#include "libots.h"

void print_about(FILE* stream, OtsArticle* Doc);

int main(int argc, char** argv) {
  OtsArticle* Art;
  int sumPercent = 20;    /* if not told otherwise highlight 20% of the  document */
  
#ifndef PPAPI
  char* dictionary_file = "en"; /* if not told otherwise, assume we're using english */
  const char* input_file = NULL;
  char* output_file = NULL;
  FILE* input_stream  = stdin;  /*by default read from stdin */
  FILE* output_stream = stdout; /*by default read from stdout */
  GError* error = NULL;
  GOptionContext* context = NULL;
  const char* const* args;
  int n_args     = 0;
  int keywords   = FALSE;
  int about      = FALSE;
  int version    = FALSE;
  
  const GOptionEntry options[] = {
    {"ratio"   , 'r', 0, G_OPTION_ARG_INT   , &sumPercent, "summarization % [default = 20%]", "<int>"}, 
    {"dic"     , 'd', 0, G_OPTION_ARG_STRING, &dictionary_file, "dictionary to use", "<string>"},
    {"out"     , 'o', 0, G_OPTION_ARG_STRING, &output_file, "output file [default = stdout]", "<string>"},
    {"keywords", 'k', 0, G_OPTION_ARG_NONE  , &keywords, "only output keywords", NULL},
    {"about"   , 'a', 0, G_OPTION_ARG_NONE  , &about, "only output the summary", NULL},
    {"version" , 'v', 0, G_OPTION_ARG_NONE  , &version, "show version information", NULL},
    { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &args, NULL, "[file.txt | stdin]" },
    {NULL}
  };
  
  context = g_option_context_new(" - Open Text Summarizer");
  g_option_context_add_main_entries(context, options, NULL);
  
  /* Parse command line */
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_option_context_free(context);
    g_print("%s\n", error->message);
    g_error_free(error);
    return 1;
  }
  
  /* print version number */
  if (version) {
    printf("%s\n", PACKAGE_STRING);
    g_option_context_free(context);
    return 0;
  }
  
  if (args) {
    while (args[n_args] != NULL) {
      n_args++;
    }
  }
  if (n_args > 1) {
    g_option_context_free(context);
    return 1;
  }
  
  if (n_args == 1 && g_file_test(args[0], G_FILE_TEST_EXISTS) == TRUE) {
    input_file = args[0];
  }
  if (input_file) {
    input_stream = fopen(input_file, "r");
    if (!input_stream) {
      g_option_context_free(context);
      perror("Couldn't load input file");
      return 1;
    }
  }
  
  if (output_file) {
    output_stream = fopen(output_file, "w");
    if (!output_stream) {
      if (input_file) {
        fclose(input_stream);
      }
      g_option_context_free(context);
      perror("Couldn't load output file");
      return 1;
    }
  }
#endif /* PPAPI */
  
  Art = ots_new_article();
  
#ifndef PPAPI 
  if (!ots_load_xml_dictionary(Art, dictionary_file)) {
    ots_free_article(Art);
    if (input_file) {
      fclose(input_stream);
    }
    if (output_file) {
      fclose(output_stream);
    }
    g_option_context_free(context);
    perror("Couldn't load dictionary");
    return 1;
  }
#else
  const char* dict_buffer = "<?xml version=\"1.0\"?>\n\
<dictionary lang=\"English\" maintainer=\"nadav256@hotmail.com\">\n\
  <stemmer>\n\
\n\
\n\
    <step1_pre> \n\
	  <rule>\"|</rule> \n\
	  <rule>(|</rule> \n\
	  <rule>'|</rule> \n\
	  <rule>[|</rule> \n\
	  <rule>{|</rule> \n\
	 </step1_pre>\n\
	 \n\
	 \n\
    <step1_post>\n\
      <rule>.\"|</rule> \n\
	  <rule>,\"|</rule> \n\
	  <rule>,'|</rule> \n\
	  <rule>.|</rule>\n\
	  <rule>..|</rule>\n\
	  <rule>...|</rule> \n\
	  <rule>,|</rule> \n\
	  <rule>\"|</rule>\n\
	  <rule>\")|</rule>\n\
	  <rule>\"?|</rule>\n\
	  <rule>)|</rule> \n\
	  <rule>?|</rule> \n\
	  <rule>:|</rule> \n\
	  <rule>;|</rule> \n\
	  <rule>!|</rule> \n\
	  <rule>-|</rule>\n\
	  <rule>--|</rule>\n\
	  <rule>'s|</rule>\n\
	  <rule>'d|</rule> \n\
	  <rule>n't|</rule> \n\
	  <rule>'t|</rule> \n\
	  <rule>'ve|</rule>\n\
	  <rule>'re|</rule>\n\
	  <rule>'m|</rule>\n\
	  <rule>]|</rule>\n\
	  <rule>}|</rule>\n\
	 </step1_post>\n\
  \n\
  \n\
    <manual>\n\
	  <rule>wrote|write</rule>\n\
	  <rule>came|come</rule> \n\
	  <rule>went|go</rule>\n\
	  <rule>choosing|choice</rule>\n\
	  <rule>was|be</rule>\n\
	  <rule>were|be</rule>\n\
     	  <rule>ate|eat</rule>	  \n\
     	  <rule>eaten|eat</rule>\n\
     	  <rule>beaten|beat</rule>\n\
	  <rule>became|become</rule>\n\
	  <rule>began|begin</rule>\n\
	  <rule>beheld|behold</rule>\n\
	  <rule>bent|bend</rule>\n\
	  <rule>bound|bind</rule>\n\
	  <rule>bleed|bled</rule>\n\
	  <rule>blown|blow</rule>\n\
	  <rule>blew|blow</rule>\n\
	  <rule>broken|break</rule>\n\
	  <rule>broke|break</rule>\n\
	  <rule>brought|bring</rule>\n\
	  <rule>built|build</rule>\n\
	  <rule>bought|buy</rule>\n\
	  <rule>caught|catch</rule>\n\
	  <rule>dealt|deal</rule>\n\
	  <rule>dug|dig</rule>\n\
	  <rule>dove|dive</rule>\n\
	  <rule>done|do</rule>\n\
	  <rule>did|do</rule>\n\
	  <rule>died|dead</rule>\n\
	  <rule>drawn|draw</rule>\n\
	  <rule>dreamt|dream</rule>\n\
	  <rule>dreamed|dream</rule>\n\
	  <rule>drunk|drink</rule>\n\
	  <rule>drank|drink</rule>\n\
	  <rule>dwelt|dwell</rule>\n\
	  <rule>fell|fall</rule>\n\
	  <rule>fallen|fall</rule>\n\
	  <rule>fed|feed</rule>\n\
	  <rule>felt|feel</rule>\n\
	  <rule>flown|fly</rule>\n\
	  <rule>flew|fly</rule>\n\
	  <rule>forbidden|forbid</rule>\n\
	  <rule>forgot|forget</rule>\n\
	  <rule>forgotten|forget</rule>\n\
	  <rule>forsaken|forsake</rule>\n\
	  <rule>froze|freez</rule>\n\
	  <rule>get|got</rule>\n\
	  <rule>gave|give</rule>\n\
	  <rule>gone|go</rule>\n\
	  <rule>grew|grow</rule>\n\
	  <rule>grown|grow</rule>\n\
	  <rule>hidden|hide</rule>\n\
	  <rule>hung|hang</rule>\n\
	  <rule>held|hold</rule>\n\
	  <rule>kept|keep</rule>\n\
	  <rule>knew|know</rule>\n\
	  <rule>known|know</rule>\n\
	  <rule>laid|lay</rule>\n\
	  <rule>lead|led</rule>\n\
	  <rule>leave|left</rule>\n\
	  <rule>lie|lay</rule>\n\
	  <rule>lit|light</rule>\n\
	  <rule>lose|lost</rule>\n\
	  <rule>meet|met</rule>\n\
	  <rule>made|make</rule>\n\
	  <rule>misled|mislead</rule>\n\
	  <rule>mistook|mistake</rule>\n\
	  <rule>mistaken|mistake</rule>\n\
	  <rule>overdid|overdo</rule>\n\
	  <rule>overdone|overdo</rule>\n\
	  <rule>paid|pay</rule>\n\
	  <rule>rode|ride</rule>\n\
	  <rule>rang|ring</rule>\n\
	  <rule>rung|ring</rule>\n\
	  <rule>rose|rise</rule>\n\
	  <rule>ran|run</rule>\n\
	  <rule>said|say</rule>\n\
	  <rule>shot|shoot</rule>\n\
	  <rule>sang|sing</rule>\n\
	  <rule>sung|sing</rule>\n\
	  <rule>sleep|slept</rule>\n\
	  <rule>speak|spoke</rule>\n\
	  <rule>spend|spent</rule>\n\
	  <rule>stood|stand</rule>\n\
	  <rule>stuck|stick</rule>\n\
	  <rule>satrove|strive</rule>\n\
	  <rule>strung|string</rule>\n\
	  <rule>swept|sweep</rule>\n\
	  <rule>swam|swim</rule>\n\
	  <rule>took|take</rule>\n\
	  <rule>taken|take</rule>\n\
	  <rule>teach|taught</rule>\n\
	  <rule>torn|tear</rule>\n\
	  <rule>told|tell</rule>\n\
	  <rule>thought|think</rule>\n\
	  <rule>threw|throw</rule>\n\
	  <rule>woke|wake</rule>\n\
	  <rule>wept|weep</rule>\n\
	  <rule>won|win</rule>\n\
	  <rule>withdrawn|withdraw</rule>\n\
	  <rule>withdrew|withdraw</rule>\n\
	  <rule>wrote|write</rule>\n\
	  <rule>written|write</rule> \n\
    </manual>\n\
   \n\
    \n\
    <pre>\n\
     <rule>1before1|2after2</rule> \n\
    </pre>\n\
   \n\
    <post>\n\
	<rule>tions|t</rule>\n\
	<rule>sions|s</rule>\n\
	<rule>icians|</rule>\n\
	<rule>ician|</rule>\n\
	<rule>ics|</rule>\n\
	<rule>ical|</rule>\n\
	<rule>sses|ss</rule> \n\
	<rule>ss|ss</rule> \n\
	<rule>---een|</rule> \n\
	<rule>iest|y</rule>\n\
	<rule>ily|y</rule>\n\
	<rule>sure|s</rule>\n\
	<rule>ans|</rule>\n\
	<rule>ian|</rule>\n\
	<rule>ials|</rule>\n\
	<rule>ial|</rule>\n\
	<rule>able|</rule>\n\
	<rule>ibility|</rule>\n\
	<rule>ity|</rule>\n\
	<rule>ble|</rule>\n\
	<rule>ist|</rule>\n\
	<rule>ence|</rule>\n\
	<rule>ement|</rule>\n\
	<rule>ment|</rule>\n\
	<rule>ize|y</rule>\n\
	<rule>ies|y</rule>\n\
	<rule>eed|</rule>\n\
	<rule>iful|</rule>\n\
	<rule>nning|n</rule>\n\
	<rule>nnable|n</rule>\n\
	<rule>nner|n</rule>\n\
	<rule>nned|n</rule>\n\
	<rule>nnen|n</rule>\n\
	<rule>gger|g</rule>\n\
	<rule>gged|g</rule>\n\
	<rule>ggen|g</rule>\n\
	<rule>gging|g</rule>\n\
	<rule>ggable|g</rule>\n\
	<rule>pper|p</rule>\n\
	<rule>pped|p</rule>\n\
	<rule>ppen|p</rule>\n\
	<rule>pping|p</rule>\n\
	<rule>ppable|p</rule>\n\
	<rule>tting|t</rule>\n\
	<rule>ting|t</rule>\n\
	<rule>tten|t</rule>	\n\
	<rule>ttable|t</rule>\n\
	<rule>tter|t</rule>\n\
	<rule>tted|t</rule>\n\
	<rule>ller|ll</rule>\n\
	<rule>lled|ll</rule>\n\
	<rule>llen|ll</rule>\n\
	<rule>lling|ll</rule>\n\
	<rule>llable|ll</rule>\n\
	<rule>sser|ss</rule>\n\
	<rule>ssed|ss</rule>\n\
	<rule>ssen|ss</rule>\n\
	<rule>ssing|ss</rule>\n\
	<rule>ssable|s</rule>\n\
	<rule>dding|dd</rule>\n\
	<rule>eing|e</rule>\n\
	<rule>ing|</rule>\n\
	<rule>mies|my</rule>\n\
	<rule>ly|</rule>\n\
	<rule>en|</rule>\n\
	<rule>bl|</rule>\n\
	<rule>izer|y</rule>\n\
	<rule>eli|</rule>\n\
	<rule>ousli|ous</rule>\n\
	<rule>ization|y</rule>\n\
	<rule>ation|</rule>\n\
	<rule>ator|</rule>	\n\
	<rule>ers|</rule>	\n\
	<rule>ies|</rule>\n\
	<rule>es|</rule>\n\
	<rule>ied|</rule>\n\
	<rule>ed|</rule>\n\
	<rule>cy|t</rule>\n\
	<rule>es|</rule>\n\
	<rule>is|is</rule>\n\
	<rule>s|</rule>\n\
   	<rule>ee|ee</rule>\n\
   	<rule>e|</rule>\n\
    </post>\n\
    \n\
    \n\
    <synonyms>\n\
	    <rule>colour|color</rule>\n\
	    <rule>honour|honor</rule>\n\
	    <rule>murder|kill</rule>\n\
		<rule>assist|help</rule>\n\
		<rule>simple|basic</rule>\n\
		<rule>winsome|charming</rule>\n\
		<rule>incisive|perceptive</rule>\n\
		<rule>bay|bark</rule>\n\
		<rule>verbose|wordy</rule>\n\
		<rule>angry|mad</rule>\n\
   		<rule>unhappy|sad</rule>\n\
			<rule>depressed|sad</rule>\n\
			<rule>dismal|sad</rule>\n\
			<rule>mournful|sad</rule>\n\
			<rule>dreadful|sad</rule>\n\
			<rule>dreary|sad</rule>\n\
			<rule>discouraged|sad</rule>\n\
			<rule>fled|run</rule>\n\
			<rule>fearful|afraid</rule>\n\
			<rule>terrified|afraid</rule>\n\
			<rule>hysterical|afraid</rule>\n\
			<rule>worried|afraid</rule>\n\
			<rule>scared|afraid</rule>\n\
			<rule>petrified|afraid</rule>\n\
			<rule>worse|bad</rule>\n\
			<rule>terrible|bad</rule>\n\
			<rule>horrible|bad</rule>\n\
			<rule>wicked|evil</rule>\n\
			<rule>huge|big</rule>\n\
			<rule>massive|bug</rule>\n\
			<rule>giant|big</rule>\n\
			<rule>gigantic|big</rule>\n\
			<rule>monstrous|big</rule>\n\
			<rule>tremendous|big</rule>\n\
			<rule>bulky|big</rule>\n\
			<rule>anxious|eager</rule>\n\
			<rule>intent|eager</rule>\n\
			<rule>ardent|eager</rule>\n\
			<rule>avid|eager</rule>\n\
			<rule>brave|bold</rule>\n\
			<rule>excellent|good</rule>\n\
			<rule>worthy|good</rule>\n\
			<rule>proper|good</rule>\n\
			<rule>favored|good</rule>\n\
			<rule>fine|good</rule>\n\
			<rule>brisk|happy</rule>\n\
			<rule>glad|happy</rule>\n\
			<rule>cheerful|happy</rule>\n\
			<rule>jolly|happy</rule>\n\
			<rule>pleased|happy</rule>\n\
			<rule>satisfied|happy</rule>\n\
			<rule>vivacious|happy</rule>\n\
			<rule>cheery|happy</rule>\n\
			<rule>merry|happy</rule>\n\
			<rule>injured|hurt</rule>\n\
			<rule>offended|hurt</rule>\n\
			<rule>distressed|hurt</rule>\n\
			<rule>suffering|hurt</rule>\n\
			<rule>afflicted|hurt</rule>\n\
			<rule>little|small</rule>\n\
			<rule>tiny|small</rule>\n\
			<rule>microscopic|small</rule>\n\
			<rule>miniscule|small</rule>\n\
			<rule>slender|small</rule>   \n\
			<rule>insignificant|small</rule>\n\
			<rule>gaze|look</rule>\n\
			<rule>stare|look</rule>\n\
			<rule>view|look</rule>\n\
			<rule>inspect|look</rule>\n\
			<rule>glance|look</rule>\n\
			<rule>announce|say</rule>			\n\
    </synonyms>\n\
    \n\
    \n\
  </stemmer>\n\
\n\
   <parser>\n\
    \n\
 	 <linebreak>\n\
	  <rule>.\"</rule>\n\
	  <rule>?\"</rule>\n\
	  <rule>!\"</rule>\n\
	  <rule>.</rule>\n\
	  <rule>?</rule>  \n\
	  <rule>;</rule>\n\
	  <rule>|</rule>\n\
	  <rule>!</rule>  \n\
	 </linebreak>\n\
	 \n\
 	 <linedontbreak>\n\
	  <rule>Dr.</rule>\n\
	  <rule>Mr.</rule>  \n\
	  <rule>Mrs.</rule>\n\
	  <rule>U.S.</rule>  \n\
	  <rule>Rep.</rule>  \n\
	  <rule>Sen.</rule>  \n\
	  <rule>St.</rule>  \n\
	  <rule>Jan.</rule>  \n\
	  <rule>Feb.</rule>  \n\
	  <rule>Mar.</rule>  \n\
	  <rule>Apr.</rule>  \n\
	  <rule>May.</rule>  \n\
	  <rule>Jun.</rule>  \n\
	  <rule>Jul.</rule>  \n\
	  <rule>Aug.</rule>  \n\
	  <rule>Sep.</rule>  \n\
	  <rule>Oct.</rule>  \n\
	  <rule>Nov.</rule>  \n\
	  <rule>Dec.</rule>  \n\
	  <rule>Lt.</rule>  \n\
	  <rule>Gov.</rule>  \n\
	  <rule>a.m.</rule>  \n\
	  <rule>p.m.</rule>  \n\
	 </linedontbreak>\n\
   </parser>\n\
\n\
  <grader-syn>\n\
	<depreciate>\n\
	 	<rule>for example</rule>\n\
	 	<rule>such as</rule>\n\
	</depreciate>\n\
  </grader-syn>\n\
\n\
\n\
  <grader-tf>\n\
  	<word idf=\"0.002\">dog</word>\n\
  	<word idf=\"0.0004\">house</word>\n\
  </grader-tf>\n\
\n\
\n\
  <grader-tc>\n\
	<word>--</word>\n\
	<word>-</word>\n\
	<word>a</word>\n\
	<word>about</word>\n\
	<word>again</word>\n\
	<word>all</word>\n\
	<word>along</word>\n\
	<word>almost</word>\n\
	<word>also</word>\n\
	<word>always</word>\n\
	<word>am</word>\n\
	<word>among</word>\n\
	<word>an</word>\n\
	<word>and</word>\n\
	<word>another</word>\n\
	<word>any</word>\n\
	<word>anybody</word>\n\
	<word>anything</word>\n\
	<word>anywhere</word>\n\
	<word>apart</word>\n\
	<word>are</word>\n\
	<word>around</word>\n\
	<word>as</word>\n\
	<word>at</word>\n\
	<word>be</word>\n\
	<word>because</word>\n\
	<word>been</word>\n\
	<word>before</word>\n\
	<word>being</word>\n\
	<word>between</word>\n\
	<word>both</word>\n\
	<word>but</word>\n\
	<word>by</word>\n\
	<word>can</word>\n\
	<word>cannot</word>\n\
	<word>comes</word>\n\
	<word>could</word>\n\
	<word>couldn</word>\n\
	<word>did</word>\n\
	<word>didn</word>\n\
	<word>different</word>\n\
	<word>do</word>\n\
	<word>does</word>\n\
	<word>doesn</word>\n\
	<word>done</word>\n\
	<word>don</word>\n\
	<word>down</word>\n\
	<word>during</word>\n\
	<word>dr</word>\n\
	<word>each</word>\n\
	<word>either</word>\n\
	<word>enough</word>\n\
	<word>etc</word>\n\
	<word>even</word>\n\
	<word>every</word>\n\
	<word>everybody</word>\n\
	<word>everything</word>\n\
	<word>everywhere</word>\n\
	<word>except</word>\n\
	<word>exactly</word>\n\
	<word>few</word>\n\
	<word>final</word>\n\
	<word>first</word>\n\
	<word>for</word>\n\
	<word>from</word>\n\
	<word>get</word>\n\
	<word>go</word>\n\
	<word>goes</word>\n\
	<word>gone</word>\n\
	<word>good</word>\n\
	<word>got</word>\n\
	<word>had</word>\n\
	<word>has</word>\n\
	<word>have</word>\n\
	<word>having</word>\n\
	<word>he</word>\n\
	<word>hence</word>\n\
	<word>her</word>\n\
	<word>him</word>\n\
	<word>his</word>\n\
	<word>how</word>\n\
	<word>however</word>\n\
	<word>I</word>\n\
	<word>i.e</word>\n\
	<word>if</word>\n\
	<word>in</word>\n\
	<word>initial</word>\n\
	<word>into</word>\n\
	<word>is</word>\n\
	<word>isn</word>\n\
	<word>it</word>\n\
	<word>its</word>\n\
	<word>it</word>\n\
	<word>itself</word>\n\
	<word>just</word>\n\
	<word>last</word>\n\
	<word>least</word>\n\
	<word>less</word>\n\
	<word>let</word>\n\
	<word>lets</word>\n\
	<word>let's</word>\n\
	<word>like</word>\n\
	<word>lot</word>\n\
	<word>made</word>\n\
	<word>make</word>\n\
	<word>many</word>\n\
	<word>may</word>\n\
	<word>maybe</word>\n\
	<word>me</word>\n\
	<word>might</word>\n\
	<word>mine</word>\n\
	<word>more</word>\n\
	<word>most</word>\n\
	<word>Mr</word>\n\
	<word>much</word>\n\
	<word>must</word>\n\
	<word>my</word>\n\
	<word>near</word>\n\
	<word>need</word>\n\
	<word>next</word>\n\
	<word>niether</word>\n\
	<word>no</word>\n\
	<word>nobody</word>\n\
	<word>nor</word>\n\
	<word>not</word>\n\
	<word>nothing</word>\n\
	<word>now</word>\n\
	<word>nowhere</word>\n\
	<word>of</word>\n\
	<word>off</word>\n\
	<word>often</word>\n\
	<word>oh</word>\n\
	<word>ok</word>\n\
	<word>okay</word>\n\
	<word>on</word>\n\
	<word>once</word>\n\
	<word>one</word>\n\
	<word>only</word>\n\
	<word>onto</word>\n\
	<word>or</word>\n\
	<word>other</word>\n\
	<word>our</word>\n\
	<word>ours</word>\n\
	<word>out</word>\n\
	<word>over</word>\n\
	<word>own</word>\n\
	<word>perhaps</word>\n\
	<word>please</word>\n\
	<word>previous</word>\n\
	<word>quite</word>\n\
	<word>rather</word>\n\
	<word>re</word>\n\
	<word>really</word>\n\
	<word>s</word>\n\
	<word>said</word>\n\
	<word>same</word>\n\
	<word>say</word>\n\
	<word>see</word>\n\
	<word>seems</word>\n\
	<word>several</word>\n\
	<word>shall</word>\n\
	<word>she</word>\n\
	<word>should</word>\n\
	<word>shouldn't</word>\n\
	<word>since</word>\n\
	<word>so</word>\n\
	<word>some</word>\n\
	<word>somebody</word>\n\
	<word>something</word>\n\
	<word>somewhere</word>\n\
	<word>still</word>\n\
	<word>stuff</word>\n\
	<word>such</word>\n\
	<word>than</word>\n\
	<word>t</word>\n\
	<word>that</word>\n\
	<word>the</word>\n\
	<word>their</word>\n\
	<word>theirs</word>\n\
	<word>them</word>\n\
	<word>then</word>\n\
	<word>there</word>\n\
	<word>these</word>\n\
	<word>they</word>\n\
	<word>thing</word>\n\
	<word>things</word>\n\
	<word>this</word>\n\
	<word>those</word>\n\
	<word>through</word>\n\
	<word>thus</word>\n\
	<word>to</word>\n\
	<word>too</word>\n\
	<word>top</word>\n\
	<word>two</word>\n\
	<word>under</word>\n\
	<word>unless</word>\n\
	<word>until</word>\n\
	<word>up</word>\n\
	<word>upon</word>\n\
	<word>us</word>\n\
	<word>use</word>\n\
	<word>v</word>\n\
	<word>ve</word>\n\
	<word>very</word>\n\
	<word>want</word>\n\
	<word>was</word>\n\
	<word>we</word>\n\
	<word>well</word>\n\
	<word>went</word>\n\
	<word>were</word>\n\
	<word>what</word>\n\
	<word>when</word>\n\
	<word>where</word>\n\
	<word>which</word>\n\
	<word>while</word>\n\
	<word>who</word>\n\
	<word>whom</word>\n\
	<word>why</word>\n\
	<word>will</word>\n\
	<word>with</word>\n\
	<word>without</word>\n\
	<word>won</word>\n\
	<word>would</word>\n\
	<word>x</word>\n\
	<word>yes</word>\n\
	<word>yet</word>\n\
	<word>you</word>\n\
	<word>you</word>\n\
	<word>your</word>\n\
	<word>yours</word>\n\
  </grader-tc>\n\
</dictionary>\n";
#if 0
  FILE * pFileDict = fopen("dict.txt", "w");
  if (pFileDict != NULL) {
    fputs(dict_buffer, pFileDict);
    fclose(pFileDict);
  }
#endif
  
  if (!ots_load_xml_dictionary_buf(Art, dict_buffer)) {
    ots_free_article(Art);
    perror("Couldn't load dictionary");
    return 1;
  }
#endif /* PPAPI */
  
#ifdef PPAPI
  const char* article = "Ok, \n\
 there's no way to do this gracefully, so I won't even try. I'm going to \n\
just hunker down for some really impressive extended flaming, and my \n\
asbestos underwear is firmly in place, and extremely uncomfortable.\n\
\n\
  I want to make it clear that DRM is perfectly ok with Linux!\n\
\n\
There, I've said it. I'm out of the closet. So bring it on...\n\
\n\
I've had some private discussions with various people about this already,\n\
and I do realize that a lot of people want to use the kernel in some way\n\
to just make DRM go away, at least as far as Linux is concerned. Either by\n\
some policy decision or by extending the GPL to just not allow it.\n\
\n\
In some ways the discussion was very similar to some of the software\n\
patent related GPL-NG discussions from a year or so ago: \"we don't like\n\
it, and we should change the license to make it not work somehow\". \n\
\n\
And like the software patent issue, I also don't necessarily like DRM\n\
myself, but I still ended up feeling the same: I'm an \"Oppenheimer\", and I\n\
refuse to play politics with Linux, and I think you can use Linux for\n\
whatever you want to - which very much includes things I don't necessarily\n\
personally approve of.\n\
\n\
The GPL requires you to give out sources to the kernel, but it doesn't\n\
limit what you can _do_ with the kernel. On the whole, this is just\n\
another example of why rms calls me \"just an engineer\" and thinks I have\n\
no ideals.\n\
\n\
[ Personally, I see it as a virtue - trying to make the world a slightly\n\
  better place _without_ trying to impose your moral values on other \n\
  people. You do whatever the h*ll rings your bell, I'm just an engineer \n\
  who wants to make the best OS possible. ]\n\
\n\
In short, it's perfectly ok to sign a kernel image - I do it myself\n\
indirectly every day through the kernel.org, as kernel.org will sign the\n\
tar-balls I upload to make sure people can at least verify that they came\n\
that way. Doing the same thing on the binary is no different: signing a\n\
binary is a perfectly fine way to show the world that you're the one\n\
behind it, and that _you_ trust it.\n\
\n\
And since I can imaging signing binaries myself, I don't feel that I can\n\
disallow anybody else doing so.\n\
\n\
Another part of the DRM discussion is the fact that signing is only the \n\
first step: _acting_ on the fact whether a binary is signed or not (by \n\
refusing to load it, for example, or by refusing to give it a secret key) \n\
is required too.\n\
\n\
But since the signature is pointless unless you _use_ it for something,\n\
and since the decision how to use the signature is clearly outside of the\n\
scope of the kernel itself (and thus not a \"derived work\" or anything like\n\
that), I have to convince myself that not only is it clearly ok to act on\n\
the knowledge of whather the kernel is signed or not, it's also outside of\n\
the scope of what the GPL talks about, and thus irrelevant to the license.\n\
\n\
That's the short and sweet of it. I wanted to bring this out in the open, \n\
because I know there are people who think that signed binaries are an act \n\
of \"subversion\" (or \"perversion\") of the GPL, and I wanted to make sure \n\
that people don't live under mis-apprehension that it can't be done.\n\
\n\
I think there are many quite valid reasons to sign (and verify) your\n\
kernel images, and while some of the uses of signing are odious, I don't\n\
see any sane way to distinguish between \"good\" signers and \"bad\" signers.\n\
\n\
Comments? I'd love to get some real discussion about this, but in the end \n\
I'm personally convinced that we have to allow it.\n\
\n\
Btw, one thing that is clearly _not_ allowed by the GPL is hiding private\n\
keys in the binary. You can sign the binary that is a result of the build\n\
process, but you can _not_ make a binary that is aware of certain keys\n\
without making those keys public - because those keys will obviously have\n\
been part of the kernel build itself.\n\
\n\
So don't get these two things confused - one is an external key that is\n\
applied _to_ the kernel (ok, and outside the license), and the other one\n\
is embedding a key _into_ the kernel (still ok, but the GPL requires that\n\
such a key has to be made available as \"source\" to the kernel).\n\
\n\
			Linus";
  ots_parse_file(article, Art); /* read article from stdin , put it in struct Article */
#else
  ots_parse_file(input_stream, Art);  /* read article from stdin , put it in struct Article */
#endif /* PPAPI */
  
  /* grade each sentence (how relevent is it to the text) */
  ots_grade_doc(Art);
  
  /* highlight what we are going to print 0% - 100% of the words */
  ots_highlight_doc(Art, sumPercent);
#ifndef PPAPI
  if (keywords) {
    printf("deprecated\n"); /* print_keywords(output_stream,Art); */
  } else if (about) {
    print_about(output_stream, Art);
  } else {
    ots_print_doc(output_stream, Art);  /* print article in text */
  }
#else
  
  ots_print_doc(stdout, Art); /* print article in text */
  //print_about(stdout, Art);
#endif /* PPAPI */
  
  ots_free_article(Art);
  
#ifndef PPAPI
  if (input_file) {
    fclose(input_stream);
  }
  if (output_file) {
    fclose(output_stream);
  }
#endif /* PPAPI */
  return 0;
}

void print_about(FILE* stream, OtsArticle* Doc) {
  fprintf(stream, "Article talks about \"%s\"\n", Doc->title);
}
