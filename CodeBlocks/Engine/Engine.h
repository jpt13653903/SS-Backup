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

// This module runs the task execution engine in its own thread, with a FIFO
// queue of tasks as the main interface between the engine and GUI.
//------------------------------------------------------------------------------

#ifndef Engine_h
#define Engine_h
//------------------------------------------------------------------------------

#include "MutEx.h"
#include "FileWrapper.h"
#include "FileSystemWrapper.h"
//------------------------------------------------------------------------------

class ENGINE{
 public:
  enum TYPE{
   Backup,
   Synchronise,
   Compare,
   Clean,
   Exit
  };

 private:
  struct TASK{
   int    ID;
   TYPE   Type;
   STRING Status;
   double Remaining; // Seconds, approximate
   STRING Source;
   STRING Destination;
   bool   Contents;
   STRING Incremental;
   STRING Log;

   TASK* Next;
   TASK* Prev;
  };
  TASK* Tasks;     // FIFO Queue of tasks
  TASK* TasksTail; // Tail of the queue

  struct LIST{
   STRING Source;
   STRING Destination;

   long double Size;     // Of the source
   long double Modified; // Of the source

   LIST* Next;
  };
  LIST*       List;     // FIFO Queue of potential copy operations
  LIST*       ListLast; // Tail of the queue
  long double ListSize; // Bytes
  long double DoneSize; // Bytes
  DWORD       StartTime, PauseTime;
  void        ClearList();
  LIST*       Enqueue  (
   const char* Source,
   const char* Destination,
   STRING*     Name,
   long double Size,
   long double Modified,
   bool        Folder
  );

  char*    Buffer;
  unsigned Allocated;

  STRING LogBuffer;
  void   AppendError    (STRING* String);
  void   AppendLocalTime(STRING* String, bool Filename);

  int    LastID;    // The ID last assigned to a task
  int    CurrentID; // The ID expected at the head of the queue
  bool   Running;
  DWORD  SuspendCount;
  DWORD  GUI_Thread;
  MUTEX  MutEx;
  HANDLE Handle;

  bool Valid       ();
  void UpdateStatus();

  bool inline IsFolder (STRING* File);
  char*       GetPath  (STRING* File);

  bool Flush(FileWrapper* File, const char* Buffer, unsigned Used);
  bool Copy (LIST*        Item);

  bool ContentsEqual(
   FileWrapper* File,
   const char*  Buffer1,
         char*  Buffer2,
   unsigned     Used
  );
  bool ContentsEqual(LIST*        Item);

  bool MoveToIncremental(const char* File, const char* Incremental);

  void CreateFolder(      char* Folder);
  void BuildItems  (const char* SourcePath, const char* DestinationPath);

  void StartLog(const char* Title);
  void EndLog  ();

  void BackupList   (STRING* Incremental);
  void DoBackup     ();
  void DoSynchronise();
  void DoCompare    ();
  void DoClean      ();
  void DoExit       ();

 public:
  ENGINE();
 ~ENGINE();

  void Run(); // Internal use only

  // Use these to pause / resume the thread
  void Pause ();
  void Resume();

  // Adds a task to the queue and returns its ID
  int Add(
   TYPE        Type,
   const char* Source,
   const char* Destination,
   bool        Contents,
   const char* Incremental,
   const char* Log
  );
  void Remove(int ID);

  int    GetID       (); // Of the task busy being executed
  void   GetStatus   (int ID, STRING* Status);
  double GetRemaining(int ID);

  // Used for saving the task queue to a script file.  The thread will
  // typically be paused while reading the queue.
  int  GetNext(int ID);
  void GetDescription(
   int     ID,
   TYPE  * Type,
   STRING* Source,
   STRING* Destination,
   bool  * Contents,
   STRING* Incremental,
   STRING* Log
  );
};
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
