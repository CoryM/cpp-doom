#ifndef __R_DRAW_HPP__
#define __R_DRAW_HPP__

extern boolean BorderNeedRefresh;
extern boolean BorderTopRefresh;

void R_DrawViewBorder(void);

void R_DrawTopBorder(void);
// if the view size is not full screen, draws a border around it

#endif //__R_DRAW_HPP__