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

#include <ppapi/cpp/module.h>

#include "ots.h"

namespace ots {
// The Module class. The browser calls the CreateInstance() method to create
// an instance of your NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class OtsModule : public pp::Module {
 public:
  OtsModule() : pp::Module() {}
  virtual ~OtsModule() {}

  // Create and return a OtsInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new Ots(instance);
  }
};
}  // namespace ots

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new ots::OtsModule();
}
}  // namespace pp
