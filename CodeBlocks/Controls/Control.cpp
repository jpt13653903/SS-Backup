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

#include "Control.h"
//------------------------------------------------------------------------------

CONTROL::CONTROL(
 const wchar_t* Class, DWORD Style,
 int            Left , int   Top,
 int            Width, int   Height
){
 Handle = CreateWindow(
  Class,
  L"",
  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | Style,
  Left , Top,
  Width, Height,
  Window,
  0,
  Instance,
  0
 );
 SendMessage(Handle, WM_SETFONT, (WPARAM)Font, 0);
}
//------------------------------------------------------------------------------

CONTROL::~CONTROL(){
 DestroyWindow(Handle);
}
//------------------------------------------------------------------------------

void CONTROL::SetFocus(){
 ::SetFocus(Handle);
}
//------------------------------------------------------------------------------

void CONTROL::SetLeft(int Left){
 RECT  Rect;
 POINT Point;

 GetWindowRect(Handle, &Rect);
 Point.x = Rect.left;
 Point.y = Rect.top;
 ScreenToClient(Window, &Point);

 SetWindowPos(
  Handle, 0,
  Left, Point.y,
  0, 0,
  SWP_NOSIZE | SWP_NOZORDER
 );
}
//------------------------------------------------------------------------------

void CONTROL::SetTop(int Top){
 RECT  Rect;
 POINT Point;

 GetWindowRect(Handle, &Rect);
 Point.x = Rect.left;
 Point.y = Rect.top;
 ScreenToClient(Window, &Point);

 SetWindowPos(
  Handle, 0,
  Point.x, Top,
  0, 0,
  SWP_NOSIZE | SWP_NOZORDER
 );
}
//------------------------------------------------------------------------------

void CONTROL::SetWidth(int Width){
 RECT Rect;

 GetWindowRect(Handle, &Rect);

 SetWindowPos(
  Handle, 0,
  0, 0,
  Width, Rect.bottom - Rect.top,
  SWP_NOMOVE | SWP_NOZORDER
 );
}
//------------------------------------------------------------------------------

void CONTROL::SetHeight(int Height){
 RECT Rect;

 GetWindowRect(Handle, &Rect);

 SetWindowPos(
  Handle, 0,
  0, 0,
  Rect.right - Rect.left, Height,
  SWP_NOMOVE | SWP_NOZORDER
 );
}
//------------------------------------------------------------------------------

void CONTROL::SetCaption(const wchar_t* Text){
 SetWindowText(Handle, Text);
}
//------------------------------------------------------------------------------
