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

#include "FileSystemWrapper.h"
//------------------------------------------------------------------------------

static int ITEM_Compare(void* Left, void* Right){
 FILE_SYSTEM::ITEM* left  = (FILE_SYSTEM::ITEM*)Left;
 FILE_SYSTEM::ITEM* right = (FILE_SYSTEM::ITEM*)Right;

 return left->Name.CompareNoCase(right->Name);
}
//------------------------------------------------------------------------------

FILE_SYSTEM::FILE_SYSTEM(){
 Files  .Compare = ITEM_Compare;
 Folders.Compare = ITEM_Compare;
}
//------------------------------------------------------------------------------

FILE_SYSTEM::~FILE_SYSTEM(){
 Clear();
}
//------------------------------------------------------------------------------

void FILE_SYSTEM::Clear(){
 ITEM* Item;

 Item = (ITEM*)Files.First();
 while(Item){
  delete Item;
  Item = (ITEM*)Files.Next();
 }
 Files.Clear();

 Item = (ITEM*)Folders.First();
 while(Item){
  delete Item;
  Item = (ITEM*)Folders.Next();
 }
 Folders.Clear();
}
//------------------------------------------------------------------------------

unsigned FILE_SYSTEM::GetDrives(){
 return GetLogicalDrives();
}
//------------------------------------------------------------------------------

void FILE_SYSTEM::GetHomeFolder(STRING* Path){
 DWORD   Size = MAX_PATH*2;
 wchar_t WidePath[Size];
 WidePath[0] = 0;

 HANDLE Token = 0;
 if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &Token)) return;
 if(!GetUserProfileDirectory(Token, WidePath, &Size)) return;
 CloseHandle(Token);

 *Path = WidePath;
}
//------------------------------------------------------------------------------

void FILE_SYSTEM::SetPath(const char* Parent){
 ITEM*           Item;
 STRING          Path;
 HANDLE          Search;
 WIN32_FIND_DATA FindData;

 if(!Parent) return;

 Clear();

 if(Parent[0] && Parent[1] != ':'){
  Path = Parent;
 }else{
  Path  = "\\\\?\\"; // Use long paths
  Path += Parent;
 }
 if(Path.UTF8()[Path.Length8()-1] != '\\') Path += '\\';
 Path += '*';

 Search = FindFirstFile(Path.UTF16(), &FindData);

 if(Search != INVALID_HANDLE_VALUE){
  do{
   if( // Ignore "." and ".." names
    wcscmp(FindData.cFileName, L"." ) &&
    wcscmp(FindData.cFileName, L"..")
   ){
    Item = new ITEM;
    Item->Name       = FindData.cFileName;
    Item->Created    = FindData.ftCreationTime  .dwHighDateTime * 0x1p32L +
                       FindData.ftCreationTime  .dwLowDateTime;
    Item->Accessed   = FindData.ftLastAccessTime.dwHighDateTime * 0x1p32L +
                       FindData.ftLastAccessTime.dwLowDateTime;
    Item->Modified   = FindData.ftLastWriteTime .dwHighDateTime * 0x1p32L +
                       FindData.ftLastWriteTime .dwLowDateTime;
    Item->Size       = FindData.nFileSizeHigh * 0x1p32L +
                       FindData.nFileSizeLow;
    Item->Attributes = FindData.dwFileAttributes;

    if(Item->Attributes & FILE_ATTRIBUTE_DIRECTORY) Folders.Insert(Item);
    else                                            Files  .Insert(Item);
   }
  }while(FindNextFile(Search, &FindData));
  FindClose(Search);
 }
}
//------------------------------------------------------------------------------

FILE_SYSTEM::ITEM* FILE_SYSTEM::FirstFile(){
 return (ITEM*)Files.First();
}
//------------------------------------------------------------------------------

FILE_SYSTEM::ITEM* FILE_SYSTEM::NextFile(){
 return (ITEM*)Files.Next();
}
//------------------------------------------------------------------------------

FILE_SYSTEM::ITEM* FILE_SYSTEM::FirstFolder(){
 return (ITEM*)Folders.First();
}
//------------------------------------------------------------------------------

FILE_SYSTEM::ITEM* FILE_SYSTEM::NextFolder(){
 return (ITEM*)Folders.Next();
}
//------------------------------------------------------------------------------

FILE_SYSTEM::ITEM* FILE_SYSTEM::Detail(const char* File){
 ITEM*           Item = 0;
 STRING          Path;
 HANDLE          Search;
 WIN32_FIND_DATA FindData;

 if(!File) return 0;

 Clear();

 if(File[0] && File[1] != ':'){
  Path  = File;
 }else{
  Path  = "\\\\?\\"; // Use long paths
  Path += File;
 }

 if(Path.UTF8()[Path.Length8()-1] == '\\') Path.UTF8()[Path.Length8()-1] = 0;

 Search = FindFirstFile(Path.UTF16(), &FindData);

 if(Search != INVALID_HANDLE_VALUE){
  Item = new ITEM;
  Item->Name       = FindData.cFileName;
  Item->Created    = FindData.ftCreationTime  .dwHighDateTime * 0x1p32L +
                     FindData.ftCreationTime  .dwLowDateTime;
  Item->Accessed   = FindData.ftLastAccessTime.dwHighDateTime * 0x1p32L +
                     FindData.ftLastAccessTime.dwLowDateTime;
  Item->Modified   = FindData.ftLastWriteTime .dwHighDateTime * 0x1p32L +
                     FindData.ftLastWriteTime .dwLowDateTime;
  Item->Size       = FindData.nFileSizeHigh * 0x1p32L +
                     FindData.nFileSizeLow;
  Item->Attributes = FindData.dwFileAttributes;

  if(Item->Attributes & FILE_ATTRIBUTE_DIRECTORY) Folders.Insert(Item);
  else                                            Files  .Insert(Item);

  FindClose(Search);
 }
 return Item;
}
//------------------------------------------------------------------------------

bool FILE_SYSTEM::CreateFolder(const char* Folder){
 bool          Result;
 STRING        Path;

 if(!Folder) return false;

 if(Folder[0] && Folder[1] != ':'){
  Path  = Folder;
 }else{
  Path  = "\\\\?\\"; // Use long paths
  Path += Folder;
 }

 Result = CreateDirectory(Path.UTF16(), 0);

 return Result;
}
//------------------------------------------------------------------------------

bool FILE_SYSTEM::Rename(const char* From, const char* To){
 bool          Result;
 STRING        FromString, ToString;

 if(!From) return false;
 if(!To  ) return false;

 if(From[0] && From[1] != ':'){
  FromString  = From;
 }else{
  FromString  = "\\\\?\\"; // Use long paths
  FromString += From;
 }

 if(To[0] && To[1] != ':'){
  ToString  = To;
 }else{
  ToString  = "\\\\?\\"; // Use long paths
  ToString += To;
 }

 if(FromString.UTF8()[FromString.Length8()-1] == '\\')
  FromString.UTF8()[FromString.Length8()-1] = 0;
 if(ToString  .UTF8()[ToString  .Length8()-1] == '\\')
  ToString  .UTF8()[ToString  .Length8()-1] = 0;

 Result = MoveFileEx(
  FromString.UTF16(),
  ToString  .UTF16(),
  0 // Fail when drives are different
 );

 return Result;
}
//------------------------------------------------------------------------------

bool FILE_SYSTEM::Delete(const char* File){
 bool          Result;
 STRING        Path;

 if(!File) return false;

 if(File[0] && File[1] != ':'){
  Path  = File;
 }else{
  Path  = "\\\\?\\"; // Use long paths
  Path += File;
 }

 if(Path.UTF16()[Path.Length16()-1] == '\\'){
  Path.UTF16()[Path.Length16()-1] = 0;
  Result = RemoveDirectory(Path.UTF16());
 }else{
  Result = DeleteFile(Path.UTF16());
 }

 return Result;
}
//------------------------------------------------------------------------------

void FILE_SYSTEM::GetComponentTime(long double Time, SYSTEMTIME* ComponentTime){
 FILETIME FileTime;
 FileTime.dwLowDateTime  = fmodl (Time , 0x1p32L);
 FileTime.dwHighDateTime = floorl(Time / 0x1p32L);
 FileTimeToSystemTime(&FileTime, ComponentTime);
}
//------------------------------------------------------------------------------
