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

#ifndef OTS_H_
#define OTS_H_

#include "ots/libots.h"

#include <string>
#include "ppapi/c/pp_file_info.h"

#include "ppapi/cpp/instance.h"
//#include "ppapi/cpp/rect.h"
//#include "ppapi/cpp/size.h"

namespace pp {
class FileIO;
class FileRef;
class FileSystem;
}  // namespace pp

namespace ots {

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurrence of the <embed> tag that has these
// attributes:
//     type="application/x-nacl"
//     nacl="ots.nmf"
class Ots : public pp::Instance {
 public:
  explicit Ots(PP_Instance instance);
  virtual ~Ots();

  // Open the file (if available) that stores the dictionary.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  // Called by the browser to handle the postMessage() call in Javascript.
  virtual void HandleMessage(const pp::Var& var_message);

  void Update();
  void UpdateDictionaryFromBuffer();
  void UpdateDictionaryFromFile();
  void WriteDictionaryToFile();
  void DefaultDictionaryString();
  void SummarizeText();

  PP_FileInfo file_info_;
  int32_t bytes_to_read_;
  int64_t offset_;
  pp::FileIO* file_io_;
  pp::FileRef* file_ref_;
  pp::FileSystem* file_system_;
  std::string bytes_buffer_;
  bool write_to_file_;

 private:
  Ots(const Ots&);  // Disallow copy
  void UpdateDictionaryDisplay();
  std::string dictionary_;
  std::string article;
  OtsArticle* ots_article;
  int sumPercent;
};

}  // namespace ots

#endif  // OTS_H_
