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

#include "Engine.h"
//------------------------------------------------------------------------------

#define BufferSize  0x100000   //   1 MB
#define BufferLimit 0x10000000 // 256 MB
//------------------------------------------------------------------------------

DWORD WINAPI EngineMain(LPVOID Parameter){
 ENGINE* Engine = (ENGINE*)Parameter;
 Engine->Run();
 return 0;
}
//------------------------------------------------------------------------------

ENGINE::ENGINE(){
 Allocated = BufferSize;
 Buffer    = new char[Allocated];

 Tasks  = TasksTail = 0;
 LastID = 0;

 List      = ListLast = 0;
 ListSize  = DoneSize = 0;
 StartTime = 0;

 Running      = true;
 GUI_Thread   = GetCurrentThreadId();
// SuspendCount = 0;

// Handle = CreateThread(0, 0, EngineMain, (LPVOID)this, 0, 0);

 MutEx.Obtain();
 SuspendCount = 1;
 Handle = CreateThread(0, 0, EngineMain, (LPVOID)this, CREATE_SUSPENDED, 0);
}
//------------------------------------------------------------------------------

ENGINE::~ENGINE(){
 Running = false;

 while(SuspendCount) Resume();

 WaitForSingleObject(Handle, INFINITE);
 CloseHandle        (Handle);

 TASK* Task;
 while(Tasks){
  Task  = Tasks;
  Tasks = Tasks->Next;
  delete Task;
 }

 LIST* Item;
 while(List){
  Item = List;
  List = List->Next;
  delete Item;
 }

 delete[] Buffer;
}
//------------------------------------------------------------------------------

bool ENGINE::Valid(){
 if(Running && Tasks && Tasks->ID == CurrentID) return true;
 return false;
}
//------------------------------------------------------------------------------

void ENGINE::UpdateStatus(){
 if(!Valid()) return;

 Tasks->Status.Set((double)(DoneSize / ListSize) * 100.0, 4, true);
 Tasks->Status.Append('%');

 DWORD Time = GetTickCount();
 if(DoneSize == 0){
  Tasks->Remaining = 0;
 }else{
  Tasks->Remaining = (Time - StartTime) * (ListSize / DoneSize - 1.0) / 1e3 + 1;
 }
}
//------------------------------------------------------------------------------

bool ENGINE::Flush(FileWrapper* File, const char* Buffer, unsigned Used){
 unsigned Index = 0;

 while(Valid() && Index < Used){
  UpdateStatus();

  MutEx.Release();
   if(Used - Index <= BufferSize){
    if(File->Write(Buffer + Index, Used - Index) != Used - Index){
     LogBuffer.Append("Could not write to file:\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
     MutEx.Obtain();
     return false;
    }
    DoneSize += Used - Index;
    Index     = Used;

   }else{
    if(File->Write(Buffer + Index, BufferSize) != BufferSize){
     LogBuffer.Append("Could not write to file:\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
     MutEx.Obtain();
     return false;
    }
    DoneSize += BufferSize;
    Index    += BufferSize;
   }
  MutEx.Obtain();
 }

 return true;
}
//------------------------------------------------------------------------------

bool ENGINE::Copy(LIST* Item){
 unsigned    Size, Used;
 long double Remaining;

 FileWrapper Source;
 FileWrapper Destination;

 if(!Source.Open(Item->Source.String, FileWrapper::faRead)){
  LogBuffer.Append("Could not open file for reading:\r\n ");
  LogBuffer.Append(&Item->Source);
  LogBuffer.Append("\r\n");
  LogBuffer.Append(" Error code ");
  AppendError     (&LogBuffer);
  LogBuffer.Append("\r\n");
  return false;
 }

 if(!Destination.Open(Item->Destination.String, FileWrapper::faCreate)){
  Source.Close();
  LogBuffer.Append("Could not open file for writing:\r\n ");
  LogBuffer.Append(&Item->Destination);
  LogBuffer.Append("\r\n");
  LogBuffer.Append(" Error code ");
  AppendError     (&LogBuffer);
  LogBuffer.Append("\r\n");
  return false;
 }

 Remaining = Source.GetSize();

 // Limit the buffer size to 256 MB, the file size and a power of 2
 Size = 1;
 while(Size < BufferLimit && Size < Remaining) Size *= 2;

 // Allocate the buffer
 if(Size > Allocated){
  delete[] Buffer;
  Buffer = new char[Size];
  while(!Buffer){
   Size  /= 2;
   Buffer = new char[Size];
  }
  Allocated = Size;

 // Shorten the buffer
 }else if(Size >= BufferSize && Allocated / Size >= 4){
  delete[] Buffer;
  Buffer = new char[Size];
  while(!Buffer){
   Size  /= 2;
   Buffer = new char[Size];
  }
  Allocated = Size;
 }
 Used = 0;

 // The file size could have changed from time of preparation to now.
 // Also, double the size so that reading and writing is reflected in progress.
 ListSize  += 2*Remaining - Item->Size;
 Item->Size = 2*Remaining;

 while(Valid() && Remaining > 0){
  UpdateStatus();

  if(Remaining <= BufferSize){
   if(Used + Remaining > Size){
    if(!Flush(&Destination, Buffer, Used)) return false;
    Used = 0;
   }

   MutEx.Release();
    if(Source.Read(Buffer + Used, (unsigned)Remaining) != Remaining){
     LogBuffer.Append("Could not read from file:\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
     MutEx.Obtain();
     return false;
    }
    DoneSize += Remaining;
    Used     += Remaining;
    Remaining = 0;
   MutEx.Obtain();

  }else{
   if(Used + BufferSize > Size){
    if(!Flush(&Destination, Buffer, Used)) return false;
    Used = 0;
   }

   MutEx.Release();
    if(Source.Read(Buffer + Used, (unsigned)BufferSize) != BufferSize){
     LogBuffer.Append("Could not read from file:\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
     MutEx.Obtain();
     return false;
    }
    DoneSize  += BufferSize;
    Used      += BufferSize;
    Remaining -= BufferSize;
   MutEx.Obtain();
  }
 }
 if(!Flush(&Destination, Buffer, Used)) return false;

 FILETIME Creation, Access, Modified;

 MutEx.Release();
  // Copy the time-stamps
  Source     .GetTime(&Creation, &Access, &Modified);
  Destination.SetTime(&Creation, &Access, &Modified);

  // Force the destination to be written to disk
//  Destination.Flush();

  // Close the handles
  Source     .Close();
  Destination.Close();
 MutEx.Obtain();

 if(!Valid()){ // Stopped mid-copy, so the destination is invalid => Delete it.
  FILE_SYSTEM FileSystem;
  MutEx.Release();
   FileSystem.Delete(Item->Destination.String);
  MutEx.Obtain();
  return false;
 }

 return true;
}
//------------------------------------------------------------------------------

void ENGINE::AppendError(STRING* String){
 int     j;
 DWORD   Error = GetLastError();
 wchar_t Message[0x100];

 FormatMessage(
  FORMAT_MESSAGE_FROM_SYSTEM,
  0,
  Error,
  0,
  Message,
  0x100,
  0
 );

 for(j = 0; Message[j]; j++);
 j--;
 while(j >= 0 && (Message[j] == '\r' || Message[j] == '\n')) j--;
 Message[j+1] = 0;

 String->Append((int)Error);
 String->Append(": ");
 String->Append(Message);
}
//------------------------------------------------------------------------------

void ENGINE::CreateFolder(char* Folder){
 int         j;
 FILE_SYSTEM FileSystem;

 // Check that the length >= 2
 if(!Folder || !Folder[0] || !Folder[1]) return;

 // Find the parent level
 for  (j  = 0;   Folder[j];         j++);
 while(j >= 0 && Folder[j] != '\\') j--;

 // Root, so return immediately
 if(j <= 2) return;

 // Remove the child level
 Folder[j] = 0;

 if(!FileSystem.Detail(Folder)){ // Folder does not exist
  CreateFolder(Folder); // Create parent levels

  if(FileSystem.CreateFolder(Folder)){ // Create this level
   LogBuffer.Append("Created folder:\r\n ");
   LogBuffer.Append(Folder);
   LogBuffer.Append("\r\n");
  }else{

   LogBuffer.Append("Failed to create folder:\r\n ");
   LogBuffer.Append(Folder);
   LogBuffer.Append("\r\n");
   LogBuffer.Append(" Error code ");
   AppendError     (&LogBuffer);
   LogBuffer.Append("\r\n");
  }
 }

 // Restore the child level
 Folder[j] = '\\';
}
//------------------------------------------------------------------------------

void ENGINE::ClearList(){
 LIST* Item;
 while(List){
  Item = List;
  List = List->Next;
  delete Item;
 }
 ListLast = 0;
 ListSize = DoneSize = 0;
}
//------------------------------------------------------------------------------

ENGINE::LIST* ENGINE::Enqueue(
 const char* Source,
 const char* Destination,
 STRING*     Name,
 long double Size,
 long double Modified,
 bool        Folder
){
 LIST* Item = new LIST;

 Item->Source     .Set   (Source);
 Item->Destination.Set   (Destination);
 Item->Source     .Append(Name);
 Item->Destination.Append(Name);
 Item->Size       = Size;
 Item->Modified   = Modified;
 Item->Next       = 0;

 if(Folder){
  Item->Source     .Append('\\');
  Item->Destination.Append('\\');
 }

 if(List) ListLast->Next = Item;
 else     List           = Item;

 ListLast  = Item;
 ListSize += Size;

 return Item;
}
//------------------------------------------------------------------------------

void ENGINE::BuildItems(const char* SourcePath, const char* DestinationPath){
 LIST*              Item;
 FILE_SYSTEM        FileSystem;
 FILE_SYSTEM::ITEM* File;

 MutEx.Release();
  FileSystem.SetPath(SourcePath);
 MutEx.Obtain();
 if(!Valid()) return;

 // First the folders
 File = FileSystem.FirstFolder();
 while(File){
  // First create the folder
  Item = Enqueue(SourcePath, DestinationPath, &File->Name, 0, 0, true);

  // Then backup its contents
  BuildItems(Item->Source.String, Item->Destination.String);

  File = FileSystem.NextFolder();
 }

 // Then the files
 File = FileSystem.FirstFile();
 while(File){
  Enqueue(
   SourcePath,
   DestinationPath,
  &File->Name,
   File->Size,
   File->Modified,
   false
  );

  File = FileSystem.NextFile();
 }
}
//------------------------------------------------------------------------------

bool ENGINE::IsFolder(STRING* File){
 return File->String[File->Length()-1] == '\\';
}
//------------------------------------------------------------------------------

char* ENGINE::GetPath(STRING* File){
 int   j;
 char* Path = new char[File->Length()+1];

 for(j = 0; File->String[j]; j++) Path[j] = File->String[j];
 j--;
 while(j >= 0 && Path[j] != '\\') j--;
 Path[j+1] = 0;

 return Path;
}
//------------------------------------------------------------------------------

bool ENGINE::ContentsEqual(
 FileWrapper* File,
 const char*  Buffer1,
       char*  Buffer2,
 unsigned     Used
){
 unsigned j;
 unsigned Index = 0;

 while(Valid() && Index < Used){
  UpdateStatus();

  MutEx.Release();
   if(Used - Index <= BufferSize){
    File->Read(Buffer2, Used - Index);
    for(j = 0; j < Used - Index; j++){
     if(Buffer1[Index+j] != Buffer2[j]) return false;
    }
    DoneSize += Used - Index;
    Index     = Used;

   }else{
    File->Read(Buffer2, BufferSize);
    for(j = 0; j < BufferSize; j++){
     if(Buffer1[Index+j] != Buffer2[j]) return false;
    }
    DoneSize += BufferSize;
    Index    += BufferSize;
   }
  MutEx.Obtain();
 }

 return true;
}
//------------------------------------------------------------------------------

bool ENGINE::ContentsEqual(LIST* Item){
 unsigned    Size, Used;
 long double Remaining;

 FileWrapper Source;
 FileWrapper Destination;

 if(!Source.Open(Item->Source.String, FileWrapper::faRead)){
  LogBuffer.Append("Could not open file for reading:\r\n");
  LogBuffer.Append(&Item->Source);
  LogBuffer.Append("\r\n");
  LogBuffer.Append("Error code ");
  AppendError     (&LogBuffer);
  LogBuffer.Append("\r\n");
  return false;
 }

 if(!Destination.Open(Item->Destination.String, FileWrapper::faRead)){
  Source.Close();
  LogBuffer.Append("Could not open file for reading:\r\n");
  LogBuffer.Append(&Item->Destination);
  LogBuffer.Append("\r\n");
  LogBuffer.Append("Error code ");
  AppendError     (&LogBuffer);
  LogBuffer.Append("\r\n");
  return false;
 }

 Remaining = Source.GetSize();

 if(Remaining != Destination.GetSize()){
  Source     .Close();
  Destination.Close();
  return false;
 }

 // Limit the buffer size to 256 MB, the file size and a power of 2
 Size = 1;
 while(Size < BufferLimit && Size < Remaining) Size *= 2;

 // Allocate the buffer
 if(Size > Allocated){
  delete[] Buffer;
  Buffer = new char[Size];
  while(!Buffer){
   Size  /= 2;
   Buffer = new char[Size];
  }
  Allocated = Size;

 // Shorten the buffer
 }else if(Size >= BufferSize && Allocated / Size >= 4){
  delete[] Buffer;
  Buffer = new char[Size];
  while(!Buffer){
   Size  /= 2;
   Buffer = new char[Size];
  }
  Allocated = Size;
 }
 Used = 0;

 // Add this compare to the status size
 ListSize += 2*Remaining;

 char* TempBuffer = new char[BufferSize];

 while(Valid() && Remaining > 0){
  UpdateStatus();

  if(Remaining <= BufferSize){
   if(Used + Remaining > Size){
    if(!ContentsEqual(&Destination, Buffer, TempBuffer, Used)){
     Source     .Close();
     Destination.Close();
     delete[] TempBuffer;
     return false;
    }
    Used = 0;
   }

   MutEx.Release();
    Source.Read(Buffer + Used, (unsigned)Remaining);
    Used     += Remaining;
    DoneSize += Remaining;
    Remaining = 0;
   MutEx.Obtain();

  }else{
   if(Used + BufferSize > Size){
    if(!ContentsEqual(&Destination, Buffer, TempBuffer, Used)){
     Source     .Close();
     Destination.Close();
     delete[] TempBuffer;
     return false;
    }
    Used = 0;
   }

   MutEx.Release();
    Source.Read(Buffer + Used, (unsigned)BufferSize);
    Used      += BufferSize;
    DoneSize  += BufferSize;
    Remaining -= BufferSize;
   MutEx.Obtain();
  }
 }
 if(!ContentsEqual(&Destination, Buffer, TempBuffer, Used)){
  Source     .Close();
  Destination.Close();
  delete[] TempBuffer;
  return false;
 }

 delete[] TempBuffer;

 FILETIME Creation1, Access1, Modified1;
 FILETIME Creation2, Access2, Modified2;

 MutEx.Release();
  Source     .GetTime(&Creation1, &Access1, &Modified1);
  Destination.GetTime(&Creation2, &Access2, &Modified2);

  Source     .Close();
  Destination.Close();
 MutEx.Obtain();

 if(
  Creation1.dwLowDateTime  != Creation2.dwLowDateTime  ||
  Access1  .dwLowDateTime  != Access2  .dwLowDateTime  ||
  Modified1.dwLowDateTime  != Modified2.dwLowDateTime  ||
  Creation1.dwHighDateTime != Creation2.dwHighDateTime ||
  Access1  .dwHighDateTime != Access2  .dwHighDateTime ||
  Modified1.dwHighDateTime != Modified2.dwHighDateTime
 ) return false;

 return true;
}
//------------------------------------------------------------------------------

bool ENGINE::MoveToIncremental(const char* File, const char* Incremental){
 int         j;
 STRING      Destination;
 FILE_SYSTEM FileSystem;

 Destination.Set(Incremental);
 for(j = 0; File[j]; j++){
  switch(File[j]){
   case '\\':
    if(!j || File[j+1] == '\\') Destination.Append("%5C");
    else                        Destination.Append('\\');
    break;

   case '/':
    Destination.Append("%2F");
    break;

   case ':':
    Destination.Append("%3A");
//    Destination.Append("-Drive");
    break;

   case '*':
    Destination.Append("%2A");
    break;

   case '?':
    Destination.Append("%3F");
    break;

   case '"':
    Destination.Append("%22");
    break;

   case '<':
    Destination.Append("%3C");
    break;

   case '>':
    Destination.Append("%3E");
    break;

   case '|':
    Destination.Append("%7C");
    break;

   default:
    Destination.Append(File[j]);
    break;
  }
 }

 char* Path = GetPath(&Destination);
 CreateFolder(Path);
 delete[] Path;

 if(FileSystem.Rename(File, Destination.String)){
  LogBuffer.Append("Moved file:\r\n");
  LogBuffer.Append(" Source:      ");
  LogBuffer.Append(File);
  LogBuffer.Append("\r\n");
  LogBuffer.Append(" Destination: ");
  LogBuffer.Append(Destination.String);
  LogBuffer.Append("\r\n");

 }else{
  LogBuffer.Append("Failed to move file, trying copy-delete cycle:\r\n");
  LogBuffer.Append(" Source:      ");
  LogBuffer.Append(File);
  LogBuffer.Append("\r\n");
  LogBuffer.Append(" Destination: ");
  LogBuffer.Append(Destination.String);
  LogBuffer.Append("\r\n");
  LogBuffer.Append(" Error code ");
  AppendError     (&LogBuffer);
  LogBuffer.Append("\r\n");

  LIST Item;
  Item.Source     .Set( File);
  Item.Destination.Set(&Destination);
  Item.Size     = 0;
  Item.Modified = 0;
  Item.Next     = 0;

  if(Copy(&Item)){
   LogBuffer.Append(" Copy successful\r\n");

   if(FileSystem.Delete(File)){
    LogBuffer.Append(" Delete successful\r\n");
   }else{
    LogBuffer.Append(" Failed to delete file:\r\n");
    LogBuffer.Append("  Error code ");
    AppendError     (&LogBuffer);
    LogBuffer.Append("\r\n");
    return false;
   }
  }else{
   LogBuffer.Append(" Failed to copy file:\r\n");
   return false;
  }
 }
 return true;
}
//------------------------------------------------------------------------------

void ENGINE::BackupList(STRING* Incremental){
 LIST*              Item;
 STRING             String;
 FILE_SYSTEM        FileSystem;
 FILE_SYSTEM::ITEM* Destination;

 while(Valid() && List){
  Item = List;
  List = List->Next;

  if(IsFolder(&Item->Destination)){ // Folder
   CreateFolder(Item->Destination.String);

  }else{ // File
   Destination = FileSystem.Detail(Item->Destination.String);

   if(!Destination || (
     Item ->Modified  > Destination->Modified
    ) || (
     Item ->Modified == Destination->Modified &&
     Item ->Size     != Destination->Size
    ) || (
     Item ->Modified == Destination->Modified &&
     Item ->Size     == Destination->Size     &&
     Tasks->Contents && !ContentsEqual(Item)
    )
   ){
    if(Destination && Incremental->Length()){
     MoveToIncremental(Item->Destination.String, Incremental->String);
    }

    if(Copy(Item)) LogBuffer.Append("Copied file:\r\n");
    else           LogBuffer.Append("Failed to copy file:\r\n");
    LogBuffer.Append(" Source:      ");
    LogBuffer.Append(&Item->Source);
    LogBuffer.Append("\r\n");
    LogBuffer.Append(" Destination: ");
    LogBuffer.Append(&Item->Destination);
    LogBuffer.Append("\r\n");

   }else{
    DoneSize += Item->Size;
   }
  }

  UpdateStatus();

  delete Item;

  MutEx.Release();
   Sleep(0);
  MutEx.Obtain();
 }
}
//------------------------------------------------------------------------------

void ENGINE::AppendLocalTime(STRING* String, bool Filename){
  SYSTEMTIME Time;
  GetLocalTime(&Time);

  String->Append((int)Time.wYear  ); String->Append('-');
  if(Time.wMonth  < 10) String->Append('0');
  String->Append((int)Time.wMonth ); String->Append('-');
  if(Time.wDay    < 10) String->Append('0');
  String->Append((int)Time.wDay   ); String->Append(", ");
  if(Time.wHour   < 10) String->Append('0');
  String->Append((int)Time.wHour  ); String->Append(Filename ? '\'' : ':');
  if(Time.wMinute < 10) String->Append('0');
  String->Append((int)Time.wMinute); String->Append(Filename ? '\'' : ':');
  if(Time.wSecond < 10) String->Append('0');
  String->Append((int)Time.wSecond); String->Append('.');
  if(Time.wMilliseconds < 100) String->Append('0');
  if(Time.wMilliseconds <  10) String->Append('0');
  String->Append((int)Time.wMilliseconds);
}
//------------------------------------------------------------------------------

void ENGINE::StartLog(const char* Title){
 LogBuffer.Set   (Title);
 LogBuffer.Append(" - ");
 AppendLocalTime (&LogBuffer, false);
 LogBuffer.Append("\r\n");
 LogBuffer.Append(" Source:      ");
 LogBuffer.Append(&Tasks->Source);
 LogBuffer.Append("\r\n");
 LogBuffer.Append(" Destination: ");
 LogBuffer.Append(&Tasks->Destination);
 LogBuffer.Append("\r\n\r\n");
}
//------------------------------------------------------------------------------

void ENGINE::EndLog(){
 LogBuffer.Append("\r\nDone - ");
 AppendLocalTime (&LogBuffer, false);
 LogBuffer.Append("\r\n");
 LogBuffer.Append(
  "----------------------------------------"
  "----------------------------------------"
 );
 LogBuffer.Append("\r\n\r\n");

 if(Valid() && Tasks->Log.Length()){
  FileWrapper File;
  if(
   File.Open(Tasks->Log.String, FileWrapper::faWrite ) ||
   File.Open(Tasks->Log.String, FileWrapper::faCreate)
  ){
   File.Write(LogBuffer.String, LogBuffer.Length());
   File.Close();
  }
 }
 LogBuffer.Clear();
}
//------------------------------------------------------------------------------

void ENGINE::DoBackup(){
 StartLog("Backup");

 Tasks->Status.Set("Preparing");
 Tasks->Remaining = 0;

 // Clear the list
 ClearList();

 // Find the paths
 char* SourcePath      = GetPath(&Tasks->Source);
 char* DestinationPath = GetPath(&Tasks->Destination);

 STRING Incremental;
 if(Tasks->Incremental.Length()){
  Incremental.Set(&Tasks->Incremental);
  if(Incremental.String[Incremental.Length()-1] != '\\'){
   Incremental.Append('\\');
  }
  AppendLocalTime(&Incremental, true);
  Incremental.Append('\\');
 }

 // Make sure the destination path exists
 CreateFolder(DestinationPath);

 // Build the list (recursively)
 if(IsFolder(&Tasks->Source)){ // Folder
  BuildItems(SourcePath, DestinationPath);

 }else{ // File
  FILE_SYSTEM        FileSystem;
  FILE_SYSTEM::ITEM* File = FileSystem.Detail(Tasks->Source.String);

  if(File){
   Enqueue(
    SourcePath,
    DestinationPath,
   &File->Name,
    File->Size,
    File->Modified,
    false
   );
  }
 }

 // Do the backup
 StartTime = GetTickCount();
 BackupList(&Incremental);

 delete[] SourcePath;
 delete[] DestinationPath;

 EndLog();
}
//------------------------------------------------------------------------------

void ENGINE::DoSynchronise(){
 StartLog("Synchronise");

 Tasks->Status.Set("Preparing");
 Tasks->Remaining = 0;

 // Clear the list
 ClearList();

 // Find the paths
 char* SourcePath      = GetPath(&Tasks->Source);
 char* DestinationPath = GetPath(&Tasks->Destination);

 STRING Incremental;
 if(Tasks->Incremental.Length()){
  Incremental.Set(&Tasks->Incremental);
  if(Incremental.String[Incremental.Length()-1] != '\\'){
   Incremental.Append('\\');
  }
  AppendLocalTime(&Incremental, true);
  Incremental.Append('\\');
 }

 // Make sure the paths exist
 CreateFolder(SourcePath);
 CreateFolder(DestinationPath);

 // Build the list (recursively)
 BuildItems(SourcePath, DestinationPath);

 // Do the backup one way
 ListSize *= 2; // This is only half the operation
 StartTime = GetTickCount();
 BackupList(&Incremental);

 // Do the backup the other way
 if(Valid()){
  long double TempSize = DoneSize;

  Tasks->Status.Set("Preparing");
  ClearList();
  ListSize = DoneSize = TempSize;

  BuildItems(DestinationPath, SourcePath);
  BackupList(&Incremental);
 }

 delete[] SourcePath;
 delete[] DestinationPath;

 EndLog();
}
//------------------------------------------------------------------------------

void ENGINE::DoCompare(){
 LIST*              Item;
 FILE_SYSTEM        FileSystem;
 FILE_SYSTEM::ITEM* Destination;

 StartLog("Compare");

 Tasks->Status.Set("Preparing");
 Tasks->Remaining = 0;

 // Clear the list
 ClearList();

 // Find the paths
 char* SourcePath      = GetPath(&Tasks->Source);
 char* DestinationPath = GetPath(&Tasks->Destination);

 // Make sure the paths exist
 Destination = FileSystem.Detail(SourcePath);
 if(!Destination){
  LogBuffer.Append("Folder does not exist:\r\n ");
  LogBuffer.Append(SourcePath);
  LogBuffer.Append("\r\n");
 }
 Destination = FileSystem.Detail(DestinationPath);
 if(!Destination){
  LogBuffer.Append("Folder does not exist:\r\n ");
  LogBuffer.Append(DestinationPath);
  LogBuffer.Append("\r\n");
 }

 // Build the list (recursively)
 BuildItems(SourcePath, DestinationPath);
 BuildItems(DestinationPath, SourcePath);

 // Compare
 StartTime = GetTickCount();

 while(Valid() && List){
  Item = List;
  List = List->Next;

  if(IsFolder(&Item->Destination)){ // Folder
   Destination = FileSystem.Detail(Item->Destination.String);
   if(!Destination){
    LogBuffer.Append("Folder - Only one instance exists:\r\n ");
    LogBuffer.Append(&Item->Source);
    LogBuffer.Append("\r\n");
   }

  }else{ // File
   Destination = FileSystem.Detail(Item->Destination.String);

   if(!Destination){
    LogBuffer.Append("File - Only one instance exists:\r\n ");
    LogBuffer.Append(&Item->Source);
    LogBuffer.Append("\r\n");

   }else{
    if(Item->Modified > Destination->Modified){
     LogBuffer.Append("File - Different time-stamps:\r\n");
     LogBuffer.Append(" Newer: ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
     LogBuffer.Append(" Older: ");
     LogBuffer.Append(&Item->Destination);
     LogBuffer.Append("\r\n");
    }
    if(Item->Size > Destination->Size){
     LogBuffer.Append("File - Different sizes:\r\n");
     LogBuffer.Append(" Larger: ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
     LogBuffer.Append(" Smaller: ");
     LogBuffer.Append(&Item->Destination);
     LogBuffer.Append("\r\n");
    }
    if(
     Item ->Modified == Destination->Modified &&
     Item ->Size     == Destination->Size     &&
     Tasks->Contents && !ContentsEqual(Item)
    ){
     LogBuffer.Append("File - Different contents:\r\n ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n ");
     LogBuffer.Append(&Item->Destination);
     LogBuffer.Append("\r\n");
    }
   }

   DoneSize += Item->Size;
  }

  UpdateStatus();

  delete Item;

  MutEx.Release();
   Sleep(0);
  MutEx.Obtain();
 }

 delete[] SourcePath;
 delete[] DestinationPath;

 EndLog();
}
//------------------------------------------------------------------------------

void ENGINE::DoClean(){
 LIST*              Item;
 LIST*              Temp;
 FILE_SYSTEM        FileSystem;
 FILE_SYSTEM::ITEM* Destination;

 StartLog("Clean");

 Tasks->Status.Set("Preparing");
 Tasks->Remaining = 0;

 // Clear the list
 ClearList();

 // Find the paths
 char* SourcePath      = GetPath(&Tasks->Source);
 char* DestinationPath = GetPath(&Tasks->Destination);

 STRING Incremental;
 if(Tasks->Incremental.Length()){
  Incremental.Set(&Tasks->Incremental);
  if(Incremental.String[Incremental.Length()-1] != '\\'){
   Incremental.Append('\\');
  }
  AppendLocalTime(&Incremental, true);
  Incremental.Append('\\');
 }

 // Build the list (recursively)
 BuildItems(DestinationPath, SourcePath);

 // Invert the order of the queue
 // This makes deleting folders easier
 Temp = List;
 List = 0;
 while(Temp){
  Item = Temp;
  Temp = Temp->Next;

  Item->Next = List;
  List       = Item;
 }

 // Compare
 StartTime = GetTickCount();

 while(Valid() && List){
  Item = List;
  List = List->Next;

  if(IsFolder(&Item->Destination)){ // Folder
   Destination = FileSystem.Detail(Item->Destination.String);
   if(!Destination){
    if(FileSystem.Delete(Item->Source.String)){
     LogBuffer.Append("Folder removed:\r\n ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
    }else{
     LogBuffer.Append("Failed to remove folder:\r\n ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
    }
   }

  }else{ // File
   Destination = FileSystem.Detail(Item->Destination.String);
   if(!Destination){
    if(Incremental.Length()){
     MoveToIncremental(Item->Source.String, Incremental.String);
    }else if(FileSystem.Delete(Item->Source.String)){
     LogBuffer.Append("File deleted:\r\n ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
    }else{
     LogBuffer.Append("Failed to delete file:\r\n ");
     LogBuffer.Append(&Item->Source);
     LogBuffer.Append("\r\n");
     LogBuffer.Append(" Error code ");
     AppendError     (&LogBuffer);
     LogBuffer.Append("\r\n");
    }
   }
   DoneSize += Item->Size;
  }

  UpdateStatus();

  delete Item;

  MutEx.Release();
   Sleep(0);
  MutEx.Obtain();
 }

 delete[] SourcePath;
 delete[] DestinationPath;

 EndLog();
}
//------------------------------------------------------------------------------

void ENGINE::DoExit(){
 TASK* Task;
 while(Tasks){
  Task  = Tasks;
  Tasks = Tasks->Next;
  delete Task;
 }
 TasksTail = 0;

 PostThreadMessage(GUI_Thread, WM_QUIT, 0, 0);
}
//------------------------------------------------------------------------------

void ENGINE::Run(){
 while(Running){
  MutEx.Obtain();
   if(Tasks){
    CurrentID = Tasks->ID;
    switch(Tasks->Type){
     case Backup:
      DoBackup();
      break;

     case Synchronise:
      DoSynchronise();
      break;

     case Compare:
      DoCompare();
      break;

     case Clean:
      DoClean();
      break;

     case Exit:
      DoExit();
      break;

     default:
      break;
    }
    Remove(CurrentID);
   }
  MutEx.Release();
  Sleep(1);
 }
}
//------------------------------------------------------------------------------

void ENGINE::Pause(){
 MutEx.Obtain(); // Make sure the mutex is released by the tread before pausing
 SuspendCount = SuspendThread(Handle)+1;

 if(SuspendCount == 1) PauseTime = GetTickCount();
}
//------------------------------------------------------------------------------

void ENGINE::Resume(){
 if(SuspendCount == 0) return;

 if(SuspendCount == 1) StartTime += GetTickCount() - PauseTime;

 SuspendCount = ResumeThread(Handle)-1;
 MutEx.Release();
}
//------------------------------------------------------------------------------

int ENGINE::Add(
 TYPE        Type,
 const char* Source,
 const char* Destination,
 bool        Contents,
 const char* Incremental,
 const char* Log
){
 MutEx.Obtain();
  TASK* Task       = new TASK;

  Task->ID          = ++LastID;
  Task->Type        = Type;
  Task->Contents    = Contents;
  Task->Remaining   = 0.0;

  Task->Log        .Set(Log);
  Task->Status     .Set("Paused");
  Task->Source     .Set(Source);
  Task->Destination.Set(Destination);
  Task->Incremental.Set(Incremental);

  Task->Next = 0;
  Task->Prev = TasksTail;

  if(TasksTail) TasksTail->Next = Task;
  else          Tasks           = Task;

  TasksTail = Task;
 MutEx.Release();

 return LastID;
}
//------------------------------------------------------------------------------

void ENGINE::Remove(int ID){
 MutEx.Obtain();
  TASK* Task = Tasks;
  while(Task){
   if(Task->ID == ID) break;
   Task = Task->Next;
  }
  if(Task){
   if(Task->Prev) Task->Prev->Next = Task->Next;
   if(Task->Next) Task->Next->Prev = Task->Prev;

   if(Task == Tasks    ) Tasks     = Tasks    ->Next;
   if(Task == TasksTail) TasksTail = TasksTail->Prev;

   delete Task;
  }
 MutEx.Release();
}
//------------------------------------------------------------------------------

int ENGINE::GetID(){
 int Result = 0;
 MutEx.Obtain();
  if(Tasks) Result = Tasks->ID;
 MutEx.Release();
 return Result;
}
//------------------------------------------------------------------------------

void ENGINE::GetStatus(int ID, STRING* Status){
 MutEx.Obtain();
  TASK* Task = Tasks;
  while(Task){
   if(Task->ID == ID) break;
   Task = Task->Next;
  }
  if(Task) Status->Set(&Task->Status);
 MutEx.Release();
}
//------------------------------------------------------------------------------

double ENGINE::GetRemaining(int ID){
 double Result = 0;
 MutEx.Obtain();
  TASK* Task = Tasks;
  while(Task){
   if(Task->ID == ID) break;
   Task = Task->Next;
  }
  if(Task) Result = Task->Remaining;
 MutEx.Release();
 return Result;
}
//------------------------------------------------------------------------------

int ENGINE::GetNext(int ID){
 int Result = 0;
 MutEx.Obtain();
  TASK* Task = Tasks;
  while(Task){
   if(Task->ID == ID) break;
   Task = Task->Next;
  }
  if(Task) Task   = Task->Next;
  if(Task) Result = Task->ID;
 MutEx.Release();
 return Result;
}
//------------------------------------------------------------------------------

void ENGINE::GetDescription(
 int     ID,
 TYPE  * Type,
 STRING* Source,
 STRING* Destination,
 bool  * Contents,
 STRING* Incremental,
 STRING* Log
){
 MutEx.Obtain();
  TASK* Task = Tasks;
  while(Task){
   if(Task->ID == ID) break;
   Task = Task->Next;
  }
  if(Task){
   *Type     = Task->Type;
   *Contents = Task->Contents;

   Log        ->Set(&Task->Log        );
   Source     ->Set(&Task->Source     );
   Destination->Set(&Task->Destination);
   Incremental->Set(&Task->Incremental);
  }
 MutEx.Release();
}
//------------------------------------------------------------------------------
