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

#ifndef Menu_h
#define Menu_h
//------------------------------------------------------------------------------

#include "Global.h"
//------------------------------------------------------------------------------

// File
#define IDM_LoadScript 11
#define IDM_SaveScript 12
#define IDM_Exit       13

// Tasks
#define IDM_Add       21
#define IDM_Remove    22
#define IDM_Pause     23

// Help
#define IDM_Manual    31
#define IDM_About     32
//------------------------------------------------------------------------------

class MENU{
 private:
  HMENU File;
  HMENU Tasks;
  HMENU Help;

  bool Paused;

 public:
  HMENU  Handle;
  HACCEL AcceleratorTable;

  MENU();
 ~MENU();

  bool GetPaused();
  void SetPaused(bool Value);
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
