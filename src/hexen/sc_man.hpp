#ifndef __SC_MAN_HPP__
#define __SC_MAN_HPP__


void    SC_Open(const char *name);
void    SC_OpenLump(const char *name);
void    SC_OpenFile(const char *name);
void    SC_Close(void);
bool SC_GetString(void);
void    SC_MustGetString(void);
void    SC_MustGetStringName(char *name);
bool SC_GetNumber(void);
void    SC_MustGetNumber(void);
void    SC_UnGet(void);
// bool SC_Check(void);
bool SC_Compare(const char *text);
int     SC_MatchString(const char **strings);
int     SC_MustMatchString(const char **strings);
void    SC_ScriptError(const char *message);

extern char       *sc_String;
extern int         sc_Number;
extern int         sc_Line;
extern bool     sc_End;
extern bool     sc_FileScripts;
extern bool     sc_Crossed;
extern const char *sc_ScriptsDir;


#endif // __SC_MAN_HPP__