//<one line to give the program's name and a brief idea of what it does.>
//    Copyright (C) 2012  Leon Pajk
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ots.h"

#include <stdio.h>
#include <cmath>
#include <string>
#include "ppapi/c/pp_file_info.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/var.h"

#ifndef DEBUG
#define DEBUG 1
#endif /* DEBUG */

#ifndef dprintf
#define dprintf(fmt, ...) \
        do { if (DEBUG) fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)
#endif /* dprintf */

namespace ots {

// Callbacks that are called asynchronously by the system as a result of various
// pp::FileIO methods.
namespace AsyncCallbacks {
// Callback that is called as a result of pp::FileIO::Flush
void FlushCallback(void*, int32_t) {
}

// Callback that is called as a result of pp::FileIO::Write
void WriteCallback(void* data, int32_t bytes_written) {
  if (bytes_written < 0) {
    return;  // error
  }
  Ots* ots = static_cast<Ots*>(data);
  ots->offset_ += bytes_written;
  if (ots->offset_ == ots->bytes_buffer_.length()) {
    ots->file_io_->Flush(pp::CompletionCallback(FlushCallback, NULL));
  } else {
    // Not all the bytes to be written have been written, so call
    // pp::FileIO::Write again.
    ots->file_io_->Write(ots->offset_,
                         &ots->bytes_buffer_[ots->offset_],
                         ots->bytes_buffer_.length() - ots->offset_,
                         pp::CompletionCallback(WriteCallback, ots));
  }
}

// Callback that is called as a result of pp::FileSystem::Open
void FileSystemOpenCallback(void* data, int32_t result) {
  if (result != PP_OK) {
    return;
  }
  Ots* ots = static_cast<Ots*>(data);
  ots->UpdateDictionaryFromFile();
}

// Callback that is called as a result of pp::FileIO::Read
void ReadCallback(void* data, int32_t bytes_read) {
  if (bytes_read < 0) {
    return;  // error
  }
  Ots* ots = static_cast<Ots*>(data);
  ots->bytes_to_read_ -= bytes_read;
  if (ots->bytes_to_read_ ==  0) {
    // File has been read to completion. Parse the bytes to get the dictionary.
    ots->UpdateDictionaryFromBuffer();
  } else {
    ots->offset_ += bytes_read;
    ots->file_io_->Read(ots->offset_,
                        &ots->bytes_buffer_[ots->offset_],
                        ots->bytes_to_read_,
                        pp::CompletionCallback(ReadCallback, ots));
  }
}

// Callback that is called as a result of pp::FileIO::Query
void QueryCallback(void* data, int32_t result) {
  if (result != PP_OK) {
    return;
  }
  Ots* ots = static_cast<Ots*>(data);
  ots->bytes_to_read_ = ots->file_info_.size;
  ots->offset_ = 0;
  
  dprintf("File size: %llu\n", ots->file_info_.size);
  
  if (ots->file_info_.size > 0) {
    ots->bytes_buffer_.resize(ots->bytes_to_read_);
    ots->file_io_->Read(ots->offset_,
                        &ots->bytes_buffer_[0],
                        ots->bytes_to_read_,
                        pp::CompletionCallback(ReadCallback, ots));
  } else { // Empty file, set default dictionary
    ots->DefaultDictionaryString();
    ots->write_to_file_ = true;
    ots->WriteDictionaryToFile();
  }
}

// Callback that is called as a result of pp::FileIO::Open
void FileOpenCallback(void*data, int32_t result) {
  if (result != PP_OK) {
    return;
  }
  Ots* ots = static_cast<Ots*>(data);
  // Query the file in order to get the file size.
  ots->file_io_->Query(&ots->file_info_, pp::CompletionCallback(QueryCallback,
                                                                ots));
}

// Callback that is called as a result of pp::Core::CallOnMainThread
void UpdateCallback(void* data, int32_t /*result*/) {
  Ots* ots = static_cast<Ots*>(data);
  ots->Update();
}

}  // namespace AsyncCallbacks

class UpdateScheduler {
 public:
  UpdateScheduler(int32_t delay, Ots* ots)
    : delay_(delay), ots_(ots) {}
  ~UpdateScheduler() {
    pp::Core* core = pp::Module::Get()->core();
    core->CallOnMainThread(delay_, pp::CompletionCallback(
      AsyncCallbacks::UpdateCallback, ots_));
  }

 private:
  int32_t delay_;  // milliseconds
  Ots* ots_;       // weak
};

// Constructor (ctor)
Ots::Ots(PP_Instance instance)
    : pp::Instance(instance),
      bytes_to_read_(0),
      offset_(0),
      file_io_(NULL),
      file_ref_(NULL),
      file_system_(NULL),
      write_to_file_(false),
      ots_article(ots_new_article()),
      sumPercent(20) {
}

// Destructor (dtor)
Ots::~Ots() {
  file_io_->Close();
  delete file_io_;
  delete file_ref_;
  delete file_system_;
  ots_free_article(ots_article);
}

bool Ots::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  // Read the dictionary from file.
  file_system_ = new pp::FileSystem(this, PP_FILESYSTEMTYPE_LOCALPERSISTENT);
  file_ref_ = new pp::FileRef(*file_system_, "/en.xml");
  // We kick off a series of callbacks which open a file, query the file for
  // it's length in bytes, read the file contents, and then update the score
  // display based on the file contents.
  int32_t rv = file_system_->Open(4 * 1024 * 1024, // 4MB
    pp::CompletionCallback(AsyncCallbacks::FileSystemOpenCallback, this));
  if (rv != PP_OK_COMPLETIONPENDING) {
    PostMessage(pp::Var("ERROR: Could not open local persistent file system."));
    return true;
  }
  UpdateScheduler(0, this);
  return true;
}

void Ots::DidChangeView(const pp::Rect& position,
                        const pp::Rect& clip) {
}

void Ots::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string()) {
    return;
  }
  article = var_message.AsString();
  SummarizeText();
}

void Ots::Update() {
  WriteDictionaryToFile();
  UpdateDictionaryDisplay();
}

void Ots::UpdateDictionaryFromBuffer() {
  if (bytes_buffer_.length() && bytes_buffer_.compare(dictionary_) != 0) {
    dictionary_.resize(bytes_buffer_.length());
    dictionary_.assign(bytes_buffer_);
  }
  UpdateDictionaryDisplay();
}

void Ots::UpdateDictionaryFromFile() {
  file_io_ = new pp::FileIO(this);
  file_io_->Open(*file_ref_,
                 PP_FILEOPENFLAG_READ | PP_FILEOPENFLAG_WRITE |
                 PP_FILEOPENFLAG_CREATE,
                 pp::CompletionCallback(AsyncCallbacks::FileOpenCallback,
                                        this));
}

void Ots::WriteDictionaryToFile() {
  if (file_io_ == NULL || !write_to_file_) {
    return;
  }
  // Write the dictionary in XML format to file.
  bytes_buffer_.resize(dictionary_.length());
  bytes_buffer_.assign(dictionary_);
  offset_ = 0;  // overwrite dictionary in file.
  file_io_->Write(offset_,
                  bytes_buffer_.c_str(),
                  bytes_buffer_.length(),
                  pp::CompletionCallback(AsyncCallbacks::WriteCallback,
                                         this));
}

void Ots::DefaultDictionaryString() {
  dictionary_ = "<?xml version=\"1.0\"?>\n\
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
}



void Ots::UpdateDictionaryDisplay() {
  if (file_io_ == NULL) {
    // Since we cant't save the dictionary to file, do nothing and return.
    return;
  }
  SummarizeText();
}

void Ots::SummarizeText() {
  if (dictionary_.length() == 0) {
    dprintf("%s\n", "dictionary length is 0!");
    return;
  }
  
  dprintf("isMainThread: %d\n", (int)pp::Module::Get()->core()->IsMainThread());
  ots_parse_file(article.c_str(), ots_article);
  
  if (!ots_load_xml_dictionary_buf(ots_article, dictionary_.c_str())) {
    ots_free_article(ots_article);
    dprintf("%s\n", "Couldn't load dictionary!");
    PostMessage(pp::Var("Couldn't load dictionary"));
    return;
  }
  
  // grade each sentence (how relevent is it to the text)
  ots_grade_doc(ots_article);
  
  // highlight what we are going to print 0% - 100% of the words
  ots_highlight_doc(ots_article, sumPercent);
  
  // print article in text
  size_t size;
  PostMessage(pp::Var(ots_get_doc_text(ots_article, &size)));
  article.clear();
  ots_article->lines = NULL; // free() function crash NaCl!
}

}  // namespace ots
