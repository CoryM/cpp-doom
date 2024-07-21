#ifndef __MN_MENU_HPP__
#define __MN_MENU_HPP__

//-----------------
// MENU (MN_menu.c)
//-----------------
#include "d_event.hpp"
#include "doomtype.hpp"

void MN_Init(void);
void MN_ActivateMenu(void);
void MN_DeactivateMenu(void);
boolean MN_Responder(event_t * event);
void MN_Ticker(void);
void MN_Drawer(void);
void MN_DrTextA(const char *text, int x, int y);
void MN_DrTextAYellow(const char *text, int x, int y);
int MN_TextAWidth(const char *text);
void MN_DrTextB(const char *text, int x, int y);
int MN_TextBWidth(const char *text);

extern int messageson; // in mn_menu.cpp

#endif // __MN_MENU_HPP__