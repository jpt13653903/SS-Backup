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

#include "List.h"
//------------------------------------------------------------------------------

LIST::LIST(
 int Left, int Top, int Width, int Height
): CONTROL(
 WC_LISTVIEW,
 WS_BORDER  | WS_TABSTOP |
 LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
 Left, Top, Width, Height
){
 ItemCount = 0;

 // http://msdn.microsoft.com/en-us/library/windows/desktop/hh298360%28v=vs.85%29.aspx

 ListView_SetExtendedListViewStyle(Handle, LVS_EX_FULLROWSELECT);

 LVCOLUMN Column;
 Column.mask       = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
 Column.fmt        = LVCFMT_LEFT;
 Column.cx         = 100;
 Column.pszText    = new wchar_t[0x10];
 Column.cchTextMax = 0;

 int j = 0;
 wcscpy(Column.pszText, L"Task");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Status");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Remaining");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Source");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Destination");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Incremental");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Contents");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 wcscpy(Column.pszText, L"Log");
 Column.iSubItem = j;
 ListView_InsertColumn(Handle, j++, &Column);

 delete[] Column.pszText;
}
//------------------------------------------------------------------------------

LIST::~LIST(){
}
//------------------------------------------------------------------------------

int LIST::AddTask(
 const char* Task,
 const char* Source,
 const char* Destination,
 bool        Contents,
 const char* Incremental,
 const char* Log
){
 UNICODE_CODEC Codec;

 LVITEM Item;
 Item.mask       = LVIF_TEXT;
 Item.iItem      = ItemCount;
 Item.cchTextMax = 0;

 Item.iSubItem = 0;
 Item.pszText  = Codec.GetWideString(Task);
 ListView_InsertItem(Handle, &Item);
 delete[] Item.pszText;
 ItemCount++;

 Item.iSubItem = 3;
 Item.pszText  = Codec.GetWideString(Source);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;

 Item.iSubItem = 4;
 Item.pszText  = Codec.GetWideString(Destination);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;

 Item.iSubItem = 5;
 Item.pszText  = Codec.GetWideString(Incremental);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;

 Item.iSubItem = 6;
 if(Contents) Item.pszText = Codec.GetWideString("On");
 else         Item.pszText = Codec.GetWideString("Off");
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;

 Item.iSubItem = 7;
 Item.pszText  = Codec.GetWideString(Log);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;

 SetStatus   (Item.iItem, "Paused");
 SetRemaining(Item.iItem, "-");

 EnforceColumnWidths();

 return Item.iItem;
}
//------------------------------------------------------------------------------

void LIST::SetStatus(int Index, const char* Status){
 if(Index <  0        ) return;
 if(Index >= ItemCount) return;

 UNICODE_CODEC Codec;

 LVITEM Item;
 Item.mask       = LVIF_TEXT;
 Item.iItem      = Index;
 Item.cchTextMax = 0;

 Item.iSubItem = 1;
 Item.pszText  = Codec.GetWideString(Status);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;
}
//------------------------------------------------------------------------------

void LIST::SetRemaining(int Index, const char* Remaining){
 if(Index <  0        ) return;
 if(Index >= ItemCount) return;

 UNICODE_CODEC Codec;

 LVITEM Item;
 Item.mask       = LVIF_TEXT;
 Item.iItem      = Index;
 Item.cchTextMax = 0;

 Item.iSubItem = 2;
 Item.pszText  = Codec.GetWideString(Remaining);
 ListView_SetItem(Handle, &Item);
 delete[] Item.pszText;
}
//------------------------------------------------------------------------------

void LIST::RemoveTask(int Index){
 if(Index <  0        ) return;
 if(Index >= ItemCount) return;

 ListView_DeleteItem(Handle, Index);
 ItemCount--;

 EnforceColumnWidths();
}
//------------------------------------------------------------------------------

void LIST::EnforceColumnWidths(){
 if(ItemCount){
  ListView_SetColumnWidth(Handle, 0, LVSCW_AUTOSIZE);
  ListView_SetColumnWidth(Handle, 1, LVSCW_AUTOSIZE);
  ListView_SetColumnWidth(Handle, 2, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 3, LVSCW_AUTOSIZE);
  ListView_SetColumnWidth(Handle, 4, LVSCW_AUTOSIZE);
  ListView_SetColumnWidth(Handle, 5, LVSCW_AUTOSIZE);
  ListView_SetColumnWidth(Handle, 6, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 7, LVSCW_AUTOSIZE);
 }else{
  ListView_SetColumnWidth(Handle, 0, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 1, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 2, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 3, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 4, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 5, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 6, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetColumnWidth(Handle, 7, LVSCW_AUTOSIZE_USEHEADER);
 }
}
//------------------------------------------------------------------------------

int LIST::GetIndex(){
 return ListView_GetSelectionMark(Handle);
}
//------------------------------------------------------------------------------
