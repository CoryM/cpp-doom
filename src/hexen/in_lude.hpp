#ifndef __IN_LUDE_HPP__
#define __IN_LUDE_HPP__

#include "../doomtype.hpp"
#include "h2def.hpp"

extern boolean intermission;
extern char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE]; // in in_lude.cpp

void IN_Start(void);
void IN_Ticker(void);
void IN_Drawer(void);



#endif // __IN_LUDE_HPP__