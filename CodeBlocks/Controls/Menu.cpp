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
//------------------------------------------------------------------------------

MENU::MENU(){
 // Create the File menu
 File = CreatePopupMenu();
 AppendMenu(File, MF_STRING,  IDM_LoadScript, L"Load Script");
 AppendMenu(File, MF_STRING,  IDM_SaveScript, L"Save Script");
 AppendMenu(File, MF_SEPARATOR, 0, 0);
 AppendMenu(File, MF_STRING,  IDM_Exit     , L"Exit");

 // Create the Tasks menu
 Tasks = CreatePopupMenu();
 AppendMenu(Tasks, MF_STRING,  IDM_Add   , L"Add\tEnter");
 AppendMenu(Tasks, MF_STRING,  IDM_Remove, L"Remove\tShift+Delete");
 AppendMenu(Tasks, MF_SEPARATOR, 0, 0);
 AppendMenu(Tasks, MF_STRING,  IDM_Pause , L"Pause\tCtrl+P");

 // Create the Help
 Help = CreatePopupMenu();
 AppendMenu(Help, MF_STRING,  IDM_Manual, L"Online Manual");
 AppendMenu(Help, MF_SEPARATOR, 0, 0);
 AppendMenu(Help, MF_STRING,  IDM_About , L"About");

 // Create the menu
 Handle = CreateMenu();
 AppendMenu(Handle, MF_STRING | MF_POPUP, (UINT_PTR)File , L"File");
 AppendMenu(Handle, MF_STRING | MF_POPUP, (UINT_PTR)Tasks, L"Tasks");
 AppendMenu(Handle, MF_STRING | MF_POPUP, (UINT_PTR)Help , L"Help");

 // Create the keyboard shortcuts
 ACCEL Accelerators[3];

 Accelerators[0].fVirt = FVIRTKEY;
 Accelerators[0].key   = VK_RETURN;
 Accelerators[0].cmd   = IDM_Add;

 Accelerators[1].fVirt = FSHIFT | FVIRTKEY;
 Accelerators[1].key   = VK_DELETE;
 Accelerators[1].cmd   = IDM_Remove;

 Accelerators[2].fVirt = FCONTROL | FVIRTKEY;
 Accelerators[2].key   = 'P';
 Accelerators[2].cmd   = IDM_Pause;

 AcceleratorTable = CreateAcceleratorTable(Accelerators, 3);

 // Set the default states;
 SetPaused(true);
}
//------------------------------------------------------------------------------

MENU::~MENU(){
 DestroyAcceleratorTable(AcceleratorTable);

 DestroyMenu(Handle);
 DestroyMenu(Help);
 DestroyMenu(Tasks);
 DestroyMenu(File);
}
//------------------------------------------------------------------------------

bool MENU::GetPaused(){
 return Paused;
}
//------------------------------------------------------------------------------

void MENU::SetPaused(bool Value){
 MENUITEMINFO MenuInfo;
 MenuInfo.cbSize     = sizeof(MENUITEMINFO);
 MenuInfo.fMask      = MIIM_STRING;
 MenuInfo.fType      = MFT_STRING;
 MenuInfo.cch        = 32;

 MenuInfo.dwTypeData = new wchar_t[32];

 if(Value) wcscpy(MenuInfo.dwTypeData, L"Resume\tCtrl+P");
 else      wcscpy(MenuInfo.dwTypeData, L"Pause\tCtrl+P" );

 SetMenuItemInfo(Tasks, IDM_Pause, 0, &MenuInfo);

 delete[] MenuInfo.dwTypeData;

 Paused = Value;
}
//------------------------------------------------------------------------------
