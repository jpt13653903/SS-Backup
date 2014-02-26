//==============================================================================
// Copyright (C) John-Philip Taylor
// jpt13653903@gmail.com
//
// This file is part of a library
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

#ifndef FileSystemWrapper_h
#define FileSystemWrapper_h
//------------------------------------------------------------------------------

#include "WinHeader.h"
#include "UnicodeString.h"
#include "LLRBTree.h"
//------------------------------------------------------------------------------

// All string are UTF-8
//------------------------------------------------------------------------------

class FILE_SYSTEM{
 public:
  struct ITEM{
   STRING      Name;
   long double Created;
   long double Accessed;
   long double Modified;
   long double Size;
   unsigned    Attributes;
  };

 private:
  LLRBTree Files;
  LLRBTree Folders;

  void Clear();

 public:
  FILE_SYSTEM();
 ~FILE_SYSTEM();

  unsigned GetDrives(); // Returns a bit-mask of mounted drives
  void     GetHomeFolder(STRING* Path);

  // Call to run a search on the Path and populate the trees
  void SetPath(const char* Path);

  // Memory managed by this class
  ITEM* FirstFile  ();
  ITEM* NextFile   ();
  ITEM* FirstFolder();
  ITEM* NextFolder ();
  ITEM* Detail     (const char* File); // Returns null if non-existant

  void GetComponentTime(long double Time, SYSTEMTIME* ComponentTime);

  bool CreateFolder(const char* Folder);

  // Returns false if a copy-delete cycle is required
  bool Rename(const char* From, const char* To);
  bool Delete(const char* File);
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
