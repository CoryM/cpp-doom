#ifndef __SB_BAR_HPP__
#define __SB_BAR_HPP__

//----------------------
// STATUS BAR (SB_bar.c)
//----------------------

extern int inv_ptr; // in sb_bar.cpp
extern int curpos;
void SB_Init(void);
void SB_SetClassData(void);
boolean SB_Responder(event_t * event);
void SB_Ticker(void);
void SB_Drawer(void);
void Draw_TeleportIcon(void);
void Draw_SaveIcon(void);
void Draw_LoadIcon(void);


#endif // __SB_BAR_HPP__