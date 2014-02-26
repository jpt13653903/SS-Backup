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

#ifndef Tree_h
#define Tree_h
//------------------------------------------------------------------------------

#include "Control.h"
#include "UnicodeString.h"
//------------------------------------------------------------------------------

// All string are UTF-8
//------------------------------------------------------------------------------

class TREE: public CONTROL{
 public:
  enum TYPE{
   File     = 0,
   Drive    = 1,
   Folder   = 2,
   Computer = 3
  };

 private:
  HIMAGELIST ImageList;

 public:
  TREE(int Left, int Top, int Width, int Height);
 ~TREE();

  void Clear  (HTREEITEM Parent = 0);
  void AddItem(HTREEITEM Parent, const char* Text, TYPE Type);

  void GetPath    (HTREEITEM Item, STRING* Path);
  void CurrentPath(                STRING* Path);
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
