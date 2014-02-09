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

#include "Menu.h"
#include "Tree.h"
#include "List.h"
#include "Label.h"
#include "Button.h"
#include "Engine.h"
#include "TextBox.h"
#include "ComboBox.h"
//------------------------------------------------------------------------------

#define MinWidth  640
#define MinHeight 350
//------------------------------------------------------------------------------

// Controls
MENU* Menu;

LABEL    * TaskLabel;
BUTTON   * TaskButton;
COMBO_BOX* Task;

LABEL   * SourceLabel;
BUTTON  * SourceButton;
TEXT_BOX* Source;

LABEL   * DestinationLabel;
BUTTON  * DestinationButton;
TEXT_BOX* Destination;

LABEL   * IncrementalLabel;
BUTTON  * IncrementalButton;
TEXT_BOX* Incremental;

LABEL    * ContentsLabel;
COMBO_BOX* Contents;

LABEL   * LogLabel;
BUTTON  * LogButton;
TEXT_BOX* Log;

TREE* Folders;
LIST* TaskList;
//------------------------------------------------------------------------------

// Event handlers
void OnResize();

void OnLoadScriptClick();
void OnSaveScriptClick();
void OnExitClick      ();

void OnAddClick   ();
void OnRemoveClick();
void OnPauseClick ();

void OnButtonClick(HWND Sender);

void OnManualClick();
void OnAboutClick ();

void OnFoldersExpanding(HTREEITEM Item);
void OnFoldersCollapsed(HTREEITEM Item);

void OnTimer();
//------------------------------------------------------------------------------

// Other functionality
ENGINE*  Engine;
LLRBTree Tasks;

int  IncludeLevel   = 0;
bool LookInContents = false;

char*    Buffer;
unsigned Index;
unsigned Length;
void     Script(const char* File);

void AddTask(
 ENGINE::TYPE Task,
 const char*  Source,
 const char*  Destination,
 bool         Contents,
 const char*  Incremental,
 const char*  Log
);
//------------------------------------------------------------------------------
