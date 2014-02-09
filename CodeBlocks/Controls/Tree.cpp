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

#include "Tree.h"
//------------------------------------------------------------------------------

TREE::TREE(
 int Left, int Top, int Width, int Height
): CONTROL(
 WC_TREEVIEW,
 WS_BORDER    | WS_TABSTOP |
 TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
 Left, Top, Width, Height
){
 ImageList = ImageList_Create(16, 16, ILC_COLOR32, 4, 4);

 HICON File     = LoadIcon(Instance, MAKEINTRESOURCE(FileIcon));
 HICON Drive    = LoadIcon(Instance, MAKEINTRESOURCE(DriveIcon));
 HICON Folder   = LoadIcon(Instance, MAKEINTRESOURCE(FolderIcon));
 HICON Computer = LoadIcon(Instance, MAKEINTRESOURCE(ComputerIcon));

 ImageList_AddIcon(ImageList, File);
 ImageList_AddIcon(ImageList, Drive);
 ImageList_AddIcon(ImageList, Folder);
 ImageList_AddIcon(ImageList, Computer);

 TreeView_SetImageList(Handle, ImageList, TVSIL_NORMAL);
}
//------------------------------------------------------------------------------

TREE::~TREE(){
 ImageList_Destroy(ImageList);
}
//------------------------------------------------------------------------------

void TREE::Clear(HTREEITEM Parent){
 HTREEITEM Item;
 HTREEITEM NextItem;

 if(Parent){
  Item = TreeView_GetChild(Handle, Parent);
  while(Item){
   NextItem = TreeView_GetNextSibling(Handle, Item);
   TreeView_DeleteItem               (Handle, Item);
   Item     = NextItem;
  }
 }else{
  TreeView_DeleteAllItems(Handle);
 }
}
//------------------------------------------------------------------------------

void TREE::AddItem(HTREEITEM Parent, const char* Text, TYPE Type){
 if(!Text) return;

 UNICODE_CODEC Codec;

 TVINSERTSTRUCT Insert;
 if(Parent) Insert.hParent = Parent;
 else       Insert.hParent = TVI_ROOT;

 Insert.hInsertAfter = TVI_LAST;

 TVITEM* Item = &Insert.item;
 Item->mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
 Item->iImage     = Item->iSelectedImage = (int)Type;
 Item->pszText    = Codec.GetWideString(Text);
 Item->cchTextMax = 0;
 Item->cChildren  = 1;

 Insert.hParent = TreeView_InsertItem(Handle, &Insert);
 delete[] Item->pszText;

 if(Type != File){
  Item->pszText = Codec.GetWideString("Dummy File");
  Item->iImage  = Item->iSelectedImage = (int)File;
  TreeView_InsertItem(Handle, &Insert);
  delete[] Item->pszText;
 }
}
//------------------------------------------------------------------------------

void TREE::GetPath(HTREEITEM Item, STRING* Path){
 if(!Item){ // This is the root of the tree
  Path->Clear();
  return;
 }

 // Get the path of the parent
 GetPath(TreeView_GetParent(Handle, Item), Path);

 // Find the item text
 TVITEM Node;
 Node.mask       = TVIF_HANDLE | TVIF_TEXT;
 Node.hItem      = Item;
 Node.cchTextMax = 0x10000; // 64 kB
 Node.pszText    = new wchar_t[Node.cchTextMax];

 TreeView_GetItem(Handle, &Node);

 // Append this name
 if(Path->Length()) Path->Append('\\');
 Path->Append(Node.pszText);

 // Clean-up
 delete[] Node.pszText;
}
//------------------------------------------------------------------------------

void TREE::CurrentPath(STRING* Path){
 GetPath(TreeView_GetSelection(Handle), Path);
}
//------------------------------------------------------------------------------
