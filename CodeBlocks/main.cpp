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

#include "main.h"
//------------------------------------------------------------------------------

void OnResize(){
 int  x, y;
 RECT Rect;
 GetClientRect(Window, &Rect);

 x = Rect.right / 2 - 76;
 Task       ->SetWidth(x);
 Source     ->SetWidth(x);
 Destination->SetWidth(x);
 Incremental->SetWidth(x);
 Contents   ->SetWidth(x);
 Log        ->SetWidth(x);

 x += 80;
 TaskButton       ->SetLeft(x);
 SourceButton     ->SetLeft(x);
 DestinationButton->SetLeft(x);
 IncrementalButton->SetLeft(x);
 LogButton        ->SetLeft(x);

 x += 34;
 Folders->SetLeft (x);
 Folders->SetWidth(Rect.right - x - 6);

 TaskList->SetWidth(Rect.right - 14);

 y = Rect.bottom / 3;
 if(y < 146) y = 146;
 Folders->SetHeight(y);

 y += 12;
 TaskList->SetTop   (y);
 TaskList->SetHeight(Rect.bottom - y - 8);
}
//------------------------------------------------------------------------------

void OnLoadScriptClick(){
 OPENFILENAME Options;
 ZeroMemory(&Options, sizeof(OPENFILENAME));

 Options.lStructSize  = sizeof(OPENFILENAME);
 Options.hwndOwner    = Window;
 Options.hInstance    = Instance;
 Options.lpstrFilter  = L"Backup Script (*.BSc)\0*.BSc\0\0";
 Options.nFilterIndex = 1;
 Options.lpstrFile    = new wchar_t[MAX_PATH+1];
 Options.nMaxFile     = MAX_PATH;
 Options.lpstrTitle   = L"Load Script";
 Options.lpstrDefExt  = L"BSc";
 Options.Flags        = OFN_ENABLESIZING  |
                        OFN_FILEMUSTEXIST |
                        OFN_LONGNAMES     |
                        OFN_PATHMUSTEXIST;

 Options.lpstrFile[0] = 0;

 if(GetOpenFileName(&Options)){
  UNICODE_CODEC Codec;
  char*    UTF8 = Codec.GetUTF8(Options.lpstrFile);
  Script  (UTF8);
  delete[] UTF8;
 }

 delete[] Options.lpstrFile;
}
//------------------------------------------------------------------------------

void OnSaveScriptClick(){
 OPENFILENAME Options;
 ZeroMemory(&Options, sizeof(OPENFILENAME));

 Options.lStructSize  = sizeof(OPENFILENAME);
 Options.hwndOwner    = Window;
 Options.hInstance    = Instance;
 Options.lpstrFilter  = L"Backup Script (*.BSc)\0*.BSc\0\0";
 Options.nFilterIndex = 1;
 Options.lpstrFile    = new wchar_t[MAX_PATH+1];
 Options.nMaxFile     = MAX_PATH;
 Options.lpstrTitle   = L"Load Script";
 Options.lpstrDefExt  = L"BSc";
 Options.Flags        = OFN_ENABLESIZING    |
                        OFN_LONGNAMES       |
                        OFN_OVERWRITEPROMPT |
                        OFN_PATHMUSTEXIST;

 Options.lpstrFile[0] = 0;

 if(GetSaveFileName(&Options)){
  SYSTEMTIME Time;
  GetLocalTime(&Time);

  STRING Buffer;
  Buffer.Set("% Backup script exported on ");
  Buffer.Append((int)Time.wYear  ); Buffer.Append('-');
  if(Time.wMonth  < 10) Buffer.Append('0');
  Buffer.Append((int)Time.wMonth ); Buffer.Append('-');
  if(Time.wDay    < 10) Buffer.Append('0');
  Buffer.Append((int)Time.wDay   ); Buffer.Append(", ");
  if(Time.wHour   < 10) Buffer.Append('0');
  Buffer.Append((int)Time.wHour  ); Buffer.Append(':');
  if(Time.wMinute < 10) Buffer.Append('0');
  Buffer.Append((int)Time.wMinute); Buffer.Append(':');
  if(Time.wSecond < 10) Buffer.Append('0');
  Buffer.Append((int)Time.wSecond); Buffer.Append("\r\n");

  Engine->Pause();
   ENGINE::TYPE Type;
   STRING       Source;
   STRING       Destination;
   bool         Contents   , PrevContents = false;
   STRING       Incremental, PrevIncremental;
   STRING       Log        , PrevLog;

   int   ID = Engine->GetID();
   while(ID){
    Engine->GetDescription(
     ID,
     &Type,
     &Source,
     &Destination,
     &Contents,
     &Incremental,
     &Log
    );

    if(Type != ENGINE::Exit){
     if(PrevContents != Contents){
      Buffer.Append("\r\n[Contents]\r\n");
      if(Contents) Buffer.Append("On\r\n");
      else         Buffer.Append("Off\r\n");
      PrevContents = Contents;
     }

     if(PrevIncremental.Compare(&Incremental)){
      Buffer.Append("\r\n[Incremental]\r\n");
      if(Incremental.Length()) Buffer.Append(&Incremental);
      else                     Buffer.Append("Off");
      Buffer.Append("\r\n");
      PrevIncremental.Set(&Incremental);
     }

     if(PrevLog.Compare(&Log)){
      Buffer.Append("\r\n[Log]\r\n");
      if(Log.Length()) Buffer.Append(&Log);
      else             Buffer.Append("Off");
      Buffer.Append("\r\n");
      PrevLog.Set(&Log);
     }
    }

    switch(Type){
     case ENGINE::Backup:
      Buffer.Append("\r\n[Backup]\r\n");
      Buffer.Append(&Source     ); Buffer.Append("\r\n");
      Buffer.Append(&Destination); Buffer.Append("\r\n");
      break;

     case ENGINE::Synchronise:
      Buffer.Append("\r\n[Synchronise]\r\n");
      Buffer.Append(&Source     ); Buffer.Append("\r\n");
      Buffer.Append(&Destination); Buffer.Append("\r\n");
      break;

     case ENGINE::Compare:
      Buffer.Append("\r\n[Compare]\r\n");
      Buffer.Append(&Source     ); Buffer.Append("\r\n");
      Buffer.Append(&Destination); Buffer.Append("\r\n");
      break;

     case ENGINE::Clean:
      Buffer.Append("\r\n[Clean]\r\n");
      Buffer.Append(&Source     ); Buffer.Append("\r\n");
      Buffer.Append(&Destination); Buffer.Append("\r\n");
      break;

     case ENGINE::Exit:
      Buffer.Append("\r\n[Exit]\r\n");
      break;

     default:
      break;
    }

    ID = Engine->GetNext(ID);
   }
  Engine->Resume();

  FileWrapper File;
  if(File.Open(Options.lpstrFile, FileWrapper::faCreate)){
   File.Write(Buffer.String, Buffer.Length());
   File.Close();
  }
 }

 delete[] Options.lpstrFile;
}
//------------------------------------------------------------------------------

struct TASK{
 int ListIndex;
 int TaskID;
};
//------------------------------------------------------------------------------

int TASK_Compare(void* Left, void* Right){
 TASK* left  = (TASK*)Left;
 TASK* right = (TASK*)Right;

 if(left->TaskID < right->TaskID) return -1;
 if(left->TaskID > right->TaskID) return  1;
 return 0;
}
//------------------------------------------------------------------------------

void AddTask(
 ENGINE::TYPE Task,
 const char*  Source,
 const char*  Destination,
 bool         Contents,
 const char*  Incremental,
 const char*  Log
){
 TASK* TempTask;

 if(Task == ENGINE::Exit){
  TempTask = new TASK;

  TempTask->ListIndex = TaskList->AddTask("Exit", "", "", false, "", "");
  TempTask->TaskID    = Engine  ->Add    ( Task , "", "", false, "", "");

  Tasks.Insert(TempTask);
  return;
 }

 // Sanity check
 STRING source, destination, incremental, log;

 source     .Set(Source);
 destination.Set(Destination);
 incremental.Set(Incremental);
 log        .Set(Log);

 if(!source.Length()){
  MessageBoxA(
   Window,
   "Source is empty.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(!destination.Length()){
  MessageBoxA(
   Window,
   "Destination is empty.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(!source.CompareNoCase(&destination)){
  MessageBoxA(
   Window,
   "Source and destination must be different.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(Task != ENGINE::Backup && source.String[source.Length()-1] != '\\'){
  MessageBoxA(
   Window,
   "Only backup tasks may have a file as source input.\n"
   "Folders must have a trailing '\\'.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(destination.String[destination.Length()-1] != '\\'){
  MessageBoxA(
   Window,
   "Destination must be a folder.\n"
   "Folders must have a trailing '\\'.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(incremental.Length() && incremental.String[incremental.Length()-1] != '\\'){
  MessageBoxA(
   Window,
   "Incremental must be a folder.\n"
   "Folders must have a trailing '\\'.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 if(log.Length() && log.String[log.Length()-1] == '\\'){
  MessageBoxA(
   Window,
   "Log must be a file.",
   "Error adding task",
   MB_ICONERROR
  );
  return;
 }

 STRING TaskString;
 switch(Task){
  case ENGINE::Backup:
   TaskString.Set("Backup");
   break;

  case ENGINE::Synchronise:
   TaskString.Set("Synchronise");
   break;

  case ENGINE::Compare:
   TaskString.Set("Compare");
   break;

  case ENGINE::Clean:
   TaskString.Set("Clean");
   break;

  default:
   return;
 }

 TempTask = new TASK;

 TempTask->ListIndex = TaskList->AddTask(
  TaskString.String,
  Source,
  Destination,
  Contents,
  Incremental,
  Log
 );

 TempTask->TaskID = Engine->Add(
  Task,
  Source,
  Destination,
  Contents,
  Incremental,
  Log
 );

 Tasks.Insert(TempTask);
}
//------------------------------------------------------------------------------

void OnExitClick(){
 AddTask(ENGINE::Exit, "", "", false, "", "");
}
//------------------------------------------------------------------------------

void OnAddClick(){
 bool         ContentsCheck;
 STRING       task, source, destination, contents, incremental, log;
 ENGINE::TYPE TaskType = ENGINE::Exit;

 Task       ->GetItem(&task);
 Source     ->GetText(&source);
 Destination->GetText(&destination);
 Incremental->GetText(&incremental);
 Contents   ->GetItem(&contents);
 Log        ->GetText(&log);

 if(!contents.CompareNoCase("On")) ContentsCheck = true;
 else                              ContentsCheck = false;

 if     (!task.CompareNoCase("Backup"     )) TaskType = ENGINE::Backup;
 else if(!task.CompareNoCase("Synchronise")) TaskType = ENGINE::Synchronise;
 else if(!task.CompareNoCase("Compare"    )) TaskType = ENGINE::Compare;
 else if(!task.CompareNoCase("Clean"      )) TaskType = ENGINE::Clean;

 AddTask(
  TaskType,
  source     .String,
  destination.String,
  ContentsCheck,
  incremental.String,
  log        .String
 );
}
//------------------------------------------------------------------------------

void OnRemoveClick(){
 Engine->Pause();
  int Index = TaskList->GetIndex();

  // If the item index is valid...
  if(Index >= 0){
   // Find the task index link
   TASK* Task = (TASK*)Tasks.First();
   while(Task){
    if(Task->ListIndex == Index) break;
    Task = (TASK*)Tasks.Next();
   }

   // If the task exists...
   if(Task){
    // Fix the list indices after the task
    TASK* Temp = (TASK*)Tasks.Find(Task);
    Temp = (TASK*)Tasks.Next();
    while(Temp){
     Temp->ListIndex--;
     Temp = (TASK*)Tasks.Next();
    }

    // Delete the task from the list, engine and index
    TaskList->RemoveTask(Task->ListIndex);
    Engine  ->Remove    (Task->TaskID);
    Tasks    .Remove    (Task);
    delete               Task;
   }
  }
 Engine->Resume();
}
//------------------------------------------------------------------------------

void OnPauseClick(){
 if(Menu->GetPaused()){
  Menu      ->SetPaused (false);
  Engine    ->Resume    ();
  TaskButton->SetCaption(L"\x2759\x2759");
 }else{
  Menu      ->SetPaused (true);
  Engine    ->Pause     ();
  TaskButton->SetCaption(L"\x25B6");
 }
}
//------------------------------------------------------------------------------

void OnManualClick(){
 WinExec(
  "explorer.exe \"https://sourceforge.net/p/ss-backup/wiki/Home/\"",
  SW_MAXIMIZE
 );
}
//------------------------------------------------------------------------------

void OnAboutClick(){
 MessageBox(
  Window,
  L"Simply-Scripted Backup, Beta Version\n"
  L"Built on "__DATE__" at "__TIME__"\n"
  L"\n"
  L"Copyright (C) John-Philip Taylor\n"
  L"jpt13653903@gmail.com\n"
  L"\n"
  L"Developed using:\n"
  L"- Code::Blocks 12.11 <http://www.codeblocks.org/>\n"
  L"- MinGW (tdm-1) 4.7.1 <http://tdm-gcc.tdragon.net/>\n"
  L"\n"
  L"This program is free software: you can redistribute it and/or modify\n"
  L"it under the terms of the GNU General Public License as published by\n"
  L"the Free Software Foundation, either version 3 of the License, or\n"
  L"(at your option) any later version.\n"
  L"\n"
  L"This program is distributed in the hope that it will be useful,\n"
  L"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  L"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  L"GNU General Public License for more details.\n"
  L"\n"
  L"You should have received a copy of the GNU General Public License\n"
  L"along with this program.  If not, see <http://www.gnu.org/licenses/>",
  L"About",
  MB_ICONINFORMATION
 );
}
//------------------------------------------------------------------------------

void TreePath2RealPath(STRING* TreePath, STRING* Path){
 int         Index  = 0;
 char*       Buffer = TreePath->String; // Shortcut
 FILE_SYSTEM FileSystem;

 if(
  Buffer[0] && Buffer[0] == 'H' &&
  Buffer[1] && Buffer[1] == 'o' &&
  Buffer[2] && Buffer[2] == 'm' &&
  Buffer[3] && Buffer[3] == 'e'
 ){
  Index = 4;
  FileSystem.GetHomeFolder(Path);

 }else if(
  Buffer[0] && Buffer[0] == 'C' &&
  Buffer[1] && Buffer[1] == 'o' &&
  Buffer[2] && Buffer[2] == 'm' &&
  Buffer[3] && Buffer[3] == 'p' &&
  Buffer[4] && Buffer[4] == 'u' &&
  Buffer[5] && Buffer[5] == 't' &&
  Buffer[6] && Buffer[6] == 'e' &&
  Buffer[7] && Buffer[7] == 'r' &&
  Buffer[8] && Buffer[8] == '\\'
 ){
  Index = 9;
  while(Buffer[Index] && Buffer[Index] != '\\') Index++;
  if(Index > 11){ // Named
   Path->Append(Buffer[Index-3]);
   Path->Append(Buffer[Index-2]);
  }else{ // Unnamed
   Path->Append(Buffer[Index-2]);
   Path->Append(Buffer[Index-1]);
  }
 }
 while(Buffer[Index]) Path->Append(Buffer[Index++]);
}
//------------------------------------------------------------------------------

void OnFoldersExpanding(HTREEITEM Item){
 STRING      Path;     // The Windows path
 STRING      TreePath; // The Folders Tree path
 FILE_SYSTEM FileSystem;

 Folders->GetPath(Item, &TreePath);
 Folders->Clear  (Item);

 if(!TreePath.Compare("Computer")){
  STRING   Drive;
  wchar_t  Root[4] = L"X:\\";
  wchar_t  Name[MAX_PATH+1];
  unsigned Drives = FileSystem.GetDrives();

  for(int j = 0; j < 26; j++){
   if(Drives & 1){
    Root[0] = (wchar_t)j + L'A';
    Root[2] = L'\\';
    Name[0] = 0;
    if(GetVolumeInformation(Root, Name, MAX_PATH+1, 0, 0, 0, 0, 0)){
     Drive.Set(Name);
    }else{
     int Error = GetLastError();
     if(Error == ERROR_NOT_READY){
      Drives >>= 1;
      continue;
     }
     Drive.Set("");
    }
    Root[2] = 0;
    if(Drive.Length()){
     Drive.Append(" (");
     Drive.Append(Root);
     Drive.Append(')');
    }else{
     Drive.Append(Root);
    }
    Folders->AddItem(Item, Drive.String, TREE::Drive);
   }
   Drives >>= 1;
  }
 }else{
  TreePath2RealPath(&TreePath, &Path);
  Path.Append('\\');

  FileSystem.SetPath(Path.String);

  FILE_SYSTEM::ITEM* FileSystemItem;
  FileSystemItem = FileSystem.FirstFolder();
  while(FileSystemItem){
   Folders->AddItem(Item, FileSystemItem->Name.String, TREE::Folder);
   FileSystemItem = FileSystem.NextFolder();
  }
  FileSystemItem = FileSystem.FirstFile();
  while(FileSystemItem){
   Folders->AddItem(Item, FileSystemItem->Name.String, TREE::File);
   FileSystemItem = FileSystem.NextFile();
  }
 }
}
//------------------------------------------------------------------------------

void OnFoldersCollapsed(HTREEITEM Item){
 Folders->Clear  (Item);
 Folders->AddItem(Item, "Dummy File", TREE::File);
}
//------------------------------------------------------------------------------

void OnButtonClick(HWND Sender){
 STRING      Path;     // The Windows path
 STRING      TreePath; // The Folders Tree path
 FILE_SYSTEM FileSystem;

 Folders->CurrentPath(&TreePath);
 TreePath2RealPath   (&TreePath, &Path);

 if(Path.Length() > 2){
  FILE_SYSTEM::ITEM* Item = FileSystem.Detail(Path.String);
  if(Item){
   if(Item->Attributes & FILE_ATTRIBUTE_DIRECTORY) Path.Append('\\');
  }else{
   Path.Clear();
  }
 }else{
  Path.Append('\\');
 }

      if(Sender==SourceButton     ->Handle) Source     ->SetText(Path.String);
 else if(Sender==DestinationButton->Handle) Destination->SetText(Path.String);
 else if(Sender==IncrementalButton->Handle) Incremental->SetText(Path.String);
 else if(Sender==LogButton        ->Handle) Log        ->SetText(Path.String);
}
//------------------------------------------------------------------------------

void OnTimer(){
 TASK   Key;
 TASK*  Task;
 STRING Status;
 double Remaining;
 int    h, m, s;

 Key.TaskID = Engine->GetID();
 Task = (TASK*)Tasks.Find(&Key);

 // Synchronise the list with the engine
 while(Task && Task->ListIndex != 0){
  TaskList->RemoveTask(0);

  // The task with the smallest ID is the first one in the queue
  Task = (TASK*)Tasks.First();
  Tasks.Remove(Task);
  delete Task;

  Task = (TASK*)Tasks.First();
  while(Task){
   Task->ListIndex--;
   Task = (TASK*)Tasks.Next();
  }

  Key.TaskID = Engine->GetID(); // Might have been changed by engine thread
  Task = (TASK*)Tasks.Find(&Key);
 }

 if(!Key.TaskID){ // The task queue is empty
  Task = (TASK*)Tasks.First();
  while(Task){
   TaskList->RemoveTask(0);
   delete Task;
   Task = (TASK*)Tasks.Next();
  }
  Tasks.Clear();
  return;
 }

             Engine->GetStatus   (Key.TaskID, &Status);
 Remaining = Engine->GetRemaining(Key.TaskID);

 if(Menu->GetPaused()) TaskList->SetStatus(0, "Paused");
 else                  TaskList->SetStatus(0, Status.String);

 if(Remaining > 0.0){
  s         = fmod (Remaining , 60.0);
  Remaining = floor(Remaining / 60.0);
  m         = fmod (Remaining , 60.0);
  Remaining = floor(Remaining / 60.0);
  h         = fmod (Remaining , 60.0);
  Remaining = floor(Remaining / 60.0);

  if(h < 10) Status.Set('0');
  else       Status.Clear();
  Status.Append(h);
  Status.Append(':');

  if(m < 10) Status.Append('0');
  Status.Append(m);
  Status.Append(':');

  if(s < 10) Status.Append('0');
  Status.Append(s);

  TaskList->SetRemaining(0, Status.String);

 }else{
  TaskList->SetRemaining(0, "-");
 }

}
//------------------------------------------------------------------------------

LRESULT CALLBACK WindowProcedure(
 HWND   Handle,
 UINT   Message,
 WPARAM wParam,
 LPARAM lParam
){
 RECT*       Rect;
 NMHDR*      NotifyHeader;
 NMTREEVIEW* TreeViewNotify;

 switch(Message){
  case WM_CLOSE:
  case WM_DESTROY:
   OnExitClick();
   break;

  case WM_SIZE:
   OnResize();
   break;

  case WM_SIZING:
   Rect = (RECT*)lParam;
   if(Rect->right  < Rect->left + MinWidth)
      Rect->right  = Rect->left + MinWidth;
   if(Rect->bottom < Rect->top  + MinHeight)
      Rect->bottom = Rect->top  + MinHeight;
   OnResize();
   break;

  case WM_COMMAND:
   if((HWND)lParam == TaskButton->Handle && HIWORD(wParam) == BN_CLICKED){
    OnPauseClick();

   }else if(
    (
     (HWND)lParam == SourceButton     ->Handle ||
     (HWND)lParam == DestinationButton->Handle ||
     (HWND)lParam == IncrementalButton->Handle ||
     (HWND)lParam == LogButton        ->Handle
    ) &&
    HIWORD(wParam) == BN_CLICKED
   ){
    OnButtonClick((HWND)lParam);

   }else if(
    (HWND) lParam  == Task->Handle &&
    HIWORD(wParam) == CBN_SELCHANGE
   ){

   }else if(lParam == 0){
    switch(LOWORD(wParam)){
     case IDM_LoadScript:
      OnLoadScriptClick();
      break;

     case IDM_SaveScript:
      OnSaveScriptClick();
      break;

     case IDM_Exit:
      OnExitClick();
      break;

     case IDM_Add:
      OnAddClick();
      break;

     case IDM_Remove:
      OnRemoveClick();
      break;

     case IDM_Pause:
      OnPauseClick();
      break;

     case IDM_Manual:
      OnManualClick();
      break;

     case IDM_About:
      OnAboutClick();
      break;

     default:
      break;
    }
   }
   break;

  case WM_NOTIFY:
   NotifyHeader = (NMHDR*)lParam;
   if(NotifyHeader->hwndFrom == Folders->Handle){
    switch(NotifyHeader->code){

     case TVN_ITEMEXPANDING:
      TreeViewNotify = (NMTREEVIEW*)lParam;
      if(TreeViewNotify->action == TVE_EXPAND){
       OnFoldersExpanding(TreeViewNotify->itemNew.hItem);
      }
      break;

     case TVN_ITEMEXPANDED:
      TreeViewNotify = (NMTREEVIEW*)lParam;
      if(TreeViewNotify->action == TVE_COLLAPSE){
       OnFoldersCollapsed(TreeViewNotify->itemNew.hItem);
      }
      break;

     default:
      break;
    }
   }
   break;

  default:
   return DefWindowProc(Handle, Message, wParam, lParam);
 }
 return 0;
}
//------------------------------------------------------------------------------

void GetLine(STRING* Line){
 int j;
 Line->Clear();

 while(Index < Length){
  // Read the line
  while(Index < Length){
   if(Buffer[Index] == '\r' || Buffer[Index] == '\n') break;
   // Ignore leading spaces
   if(Line->Length() || (Buffer[Index] != ' ' && Buffer[Index] != '\t')){
    Line->Append(Buffer[Index]);
   }
   Index++;
  }
  // Ignore empty lines
  while(Index < Length && (Buffer[Index] == '\r' || Buffer[Index] == '\n')){
   Index++;
  }

  // Ignore trailing spaces
  j = Line->Length()-1;
  while(j >= 0 && (Line->String[j] == ' ' || Line->String[j] == '\t')) j--;
  Line->SetLength(j+1);

  // Ignore empty lines and comments
  if     (Line->String[0] == '%') Line->Clear();
  else if(Line->Length( )       ) return;
 }
}
//------------------------------------------------------------------------------

void Script(const char* Filename){
 int j;

 if(IncludeLevel > 1000) return; // Safety limit

 if(!IncludeLevel){
  Engine->Pause();        // Safer
  LookInContents = false; // Default, and inherited by child scripts
 }

 FileWrapper File;

 if(!File.Open(Filename, FileWrapper::faRead)){
  STRING s;
  s.Set   ("Unable to open script:\n");
  s.Append(Filename);
  MessageBoxA(Window, s.String, "Error", MB_ICONERROR);
  return;
 }

 Length = File.GetSize();
 Buffer = new char[Length+1];
 Buffer[Length] = 0;

 if(!Buffer){
  File.Close();
  return;
 }

 File.Read (Buffer, Length);
 File.Close();

 STRING Source;
 STRING Destination;
 STRING Incremental;
 STRING Log;
 STRING Line;
 STRING Temp;

 Index = 0;
 while(Index < Length){
  GetLine(&Line);
  while(Index < Length && Line.String[0] != '[') GetLine(&Line);

  if(!Line.CompareNoCase("[Incremental]")){
   GetLine(&Incremental);
   if(!Incremental.CompareNoCase("Off")) Incremental.Clear();

  }else if(!Line.CompareNoCase("[Contents]")){
   GetLine(&Line);
   if     (!Line.CompareNoCase("On" )) LookInContents = true;
   else if(!Line.CompareNoCase("Off")) LookInContents = false;
   // else ignore

  }else if(!Line.CompareNoCase("[Log]")){
   GetLine(&Log);
   if(!Log.CompareNoCase("Off")) Log.Clear();

  }else if(!Line.CompareNoCase("[Backup]")){
   GetLine(&Source);
   GetLine(&Destination);
   AddTask(
    ENGINE::Backup,
    Source     .String,
    Destination.String,
    LookInContents,
    Incremental.String,
    Log        .String
   );

  }else if(!Line.CompareNoCase("[Compare]")){
   GetLine(&Source);
   GetLine(&Destination);
   AddTask(
    ENGINE::Compare,
    Source     .String,
    Destination.String,
    LookInContents,
    Incremental.String,
    Log        .String
   );

  }else if(!Line.CompareNoCase("[Clean]")){
   GetLine(&Source);
   GetLine(&Destination);
   AddTask(
    ENGINE::Clean,
    Source     .String,
    Destination.String,
    LookInContents,
    Incremental.String,
    Log        .String
   );

  }else if(!Line.CompareNoCase("[Synchronise]")){
   GetLine(&Source);
   GetLine(&Destination);
   AddTask(
    ENGINE::Synchronise,
    Source     .String,
    Destination.String,
    LookInContents,
    Incremental.String,
    Log        .String
   );

  }else if(!Line.CompareNoCase("[Script]")){
   GetLine(&Line);

   char*    LocalBuffer = Buffer;
   unsigned LocalIndex  = Index;
   unsigned LocalLength = Length;
   IncludeLevel++;

   if(
    (Line.Length() < 2) ||
    (
     (Line.String[1] != ':') &&
     (Line.String[0] != '\\' || Line.String[0] != '\\')
    )
   ){
    for(j = 0; Filename[j]; j++);
    while(j && Filename[j] != '\\') j--;
    ((char*)Filename)[j] = 0;
    Temp.Set(Filename);
    ((char*)Filename)[j] = '\\';
    Temp.Append('\\');
    Temp.Append(&Line);
   }else{
    Temp.Set(&Line);
   }
   if(Temp.Length() < 4 || Temp.String[Temp.Length()-4] != '.'){
    Temp.Append(".BSc");
   }
   Script(Temp.String);

   IncludeLevel--;
   Index  = LocalIndex;
   Length = LocalLength;
   Buffer = LocalBuffer;

  }else if(!Line.CompareNoCase("[Exit]")){
   if(IncludeLevel){
    delete[] Buffer;
    return;
   }
   else AddTask(ENGINE::Exit, "", "", false, "", "");
  }
 }
 if(!IncludeLevel) Engine->Resume();

 delete[] Buffer;
}
//------------------------------------------------------------------------------

static HICON LoadIconSmall(WORD Icon){
 int    size = GetSystemMetrics(SM_CXSMICON);
 return (HICON)LoadImage(
  Instance,
  MAKEINTRESOURCE(Icon),
  IMAGE_ICON,
  size, size,
  LR_SHARED
 );
}
//------------------------------------------------------------------------------

int WINAPI WinMain(
 HINSTANCE hInstance,
 HINSTANCE hPrevInstance,
 LPSTR     CommandLine,
 int       CmdShow
){
 INITCOMMONCONTROLSEX icex;
 icex.dwSize = sizeof(icex);
 icex.dwICC  = 0xFFFF; // Load everything
 InitCommonControlsEx(&icex);

 MSG      Message;
 unsigned Time = GetTickCount();

 Tasks.Compare = TASK_Compare;

 // Store the Instance handle in the global variable
 Instance = hInstance;

 // Register the Window Class
 WNDCLASSEX Class;
 Class.cbSize        = sizeof(WNDCLASSEX);
 Class.hInstance     = Instance;
 Class.lpszClassName = L"BackupClass";
 Class.lpfnWndProc   = WindowProcedure;
 Class.style         = 0;
 Class.hIcon         = LoadIcon(Instance, MAKEINTRESOURCE(MainIcon));
 Class.hIconSm       = LoadIconSmall(MainIcon);
 Class.hCursor       = LoadCursor(0, IDC_ARROW);
 Class.lpszMenuName  = 0;
 Class.cbClsExtra    = 0;
 Class.cbWndExtra    = 0;
 Class.hbrBackground = (HBRUSH)COLOR_WINDOW;

 if(!RegisterClassEx(&Class)) return 2;

 // Create the menu
 Menu = new MENU;

 // Get the system font
 NONCLIENTMETRICS Metrics;
 Metrics.cbSize = sizeof(NONCLIENTMETRICS);
 SystemParametersInfo(SPI_GETNONCLIENTMETRICS, Metrics.cbSize, &Metrics, 0);
 Font = CreateFontIndirect(&Metrics.lfCaptionFont);

 // Create the window
 RECT Desktop;
 GetWindowRect(GetDesktopWindow(), &Desktop);

 Window = CreateWindowEx(
  0,
  L"BackupClass",
  L"SS-Backup",
  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
  (Desktop.right  - 800) / 2,
  (Desktop.bottom - 400) / 2,
  800, 400,
  HWND_DESKTOP,
  Menu->Handle,
  Instance,
  0
 );
 SendMessage(Window, WM_SETFONT, (WPARAM)Font, 0);

 // Create the controls
 TaskLabel         = new LABEL    (  8,  11,  64, L"Task:");
 SourceLabel       = new LABEL    (  8,  35,  64, L"Source:");
 DestinationLabel  = new LABEL    (  8,  60,  64, L"Destination:");
 IncrementalLabel  = new LABEL    (  8,  85,  64, L"Incremental:");
 ContentsLabel     = new LABEL    (  8, 111,  64, L"Contents:");
 LogLabel          = new LABEL    (  8, 135,  64, L"Log:");

 Task              = new COMBO_BOX( 76,   7, 200);
 Source            = new TEXT_BOX ( 76,  33, 200);
 Destination       = new TEXT_BOX ( 76,  58, 200);
 Incremental       = new TEXT_BOX ( 76,  83, 200);
 Contents          = new COMBO_BOX( 76, 107, 200);
 Log               = new TEXT_BOX ( 76, 133, 200);

 TaskButton        = new BUTTON   (280,   8,  30, L"\x25B6");
 SourceButton      = new BUTTON   (280,  33,  30, L"\x21D0");
 DestinationButton = new BUTTON   (280,  58,  30, L"\x21D0");
 IncrementalButton = new BUTTON   (280,  83,  30, L"\x21D0");
 LogButton         = new BUTTON   (280, 133,  30, L"\x21D0");

 Folders  = new TREE(314, 8, 200, 146);
 TaskList = new LIST(8, 158, 506, 200);

 Task->AddItem("Backup");
 Task->AddItem("Synchronise");
 Task->AddItem("Compare");
 Task->AddItem("Clean");

 Contents->AddItem("On");
 Contents->AddItem("Off");
 Contents->SetItem(1);

 Folders->Clear();
 Folders->AddItem(0, "Home"    , TREE::Folder  );
 Folders->AddItem(0, "Computer", TREE::Computer);

 // Show the window
 ShowWindow(Window, SW_SHOW);
 Folders->SetFocus();

 OnResize();

 // Run the message handlers so everything is initialised properly
 while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
  TranslateMessage(&Message);
  DispatchMessage (&Message);
 }

 // Create the engine thread
 Engine = new ENGINE;

 // Get the command-line
 int       ArgC;
 STRING    Arg;
 wchar_t** ArgV = CommandLineToArgvW(GetCommandLine(), &ArgC);

 // Process the command-line
 if(ArgV){
  for(int j = 1; j < ArgC; j++){
   Arg.Set(ArgV[j]);
   if(!Arg.CompareNoCase("-run")){
    if(Menu->GetPaused()) OnPauseClick();
   }else{
    Script(Arg.String);
   }
  }
  LocalFree(ArgV);
 }

 // Run the message loop
 while(true){
  while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
   if(Message.message == WM_QUIT) break;
   if(!TranslateAccelerator(Window, Menu->AcceleratorTable, &Message)){
    if(!IsDialogMessage(Window, &Message)){
     TranslateMessage(&Message);
     DispatchMessage (&Message);
    }
   }
  }
  if(Message.message == WM_QUIT) break;

  if(GetTickCount() - Time > 100){
   OnTimer();
   Time = GetTickCount();
  }
  Sleep(1);
 }

 // Clean-up
 delete TaskLabel;
 delete TaskButton;
 delete Task;

 delete SourceLabel;
 delete SourceButton;
 delete Source;

 delete DestinationLabel;
 delete DestinationButton;
 delete Destination;

 delete IncrementalLabel;
 delete IncrementalButton;
 delete Incremental;

 delete ContentsLabel;
 delete Contents;

 delete LogLabel;
 delete LogButton;
 delete Log;

 delete Folders;
 delete TaskList;

 DestroyWindow(Window);
 DeleteObject (Font);
 delete        Menu;

 TASK* Task = (TASK*)Tasks.First();
 while(Task){
  delete Task;
  Task = (TASK*)Tasks.Next();
 }

 delete Engine;

 return Message.wParam;
}
//------------------------------------------------------------------------------
