//==============================================================================
// Copyright (C) John-Philip Taylor
// jpt13653903@gmail.com
//
// This file is part of Simply-Scripted Backup
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//==============================================================================

#ifndef List_h
#define List_h
//------------------------------------------------------------------------------

#include "Control.h"
//------------------------------------------------------------------------------

// Note that all strings are UTF-8
//------------------------------------------------------------------------------

class LIST: public CONTROL{
 private:
  int ItemCount;

 public:
  LIST(int Left, int Top, int Width, int Height);
 ~LIST();

  void EnforceColumnWidths();

  // Returns the index
  int AddTask(
   const char* Task,
   const char* Source,
   const char* Destination,
   bool        Contents,
   const char* Incremental,
   const char* Log
  );

  void SetStatus   (int Index, const char* Status);
  void SetRemaining(int Index, const char* Remaining);
  void RemoveTask  (int Index);

  int GetIndex(); // Of currently selected item
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
