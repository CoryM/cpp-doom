#ifndef __SC_MAN_HPP__
#define __SC_MAN_HPP__


void    SC_Open(const char *name);
void    SC_OpenLump(const char *name);
void    SC_OpenFile(const char *name);
void    SC_Close(void);
boolean SC_GetString(void);
void    SC_MustGetString(void);
void    SC_MustGetStringName(char *name);
boolean SC_GetNumber(void);
void    SC_MustGetNumber(void);
void    SC_UnGet(void);
// boolean SC_Check(void);
boolean SC_Compare(const char *text);
int     SC_MatchString(const char **strings);
int     SC_MustMatchString(const char **strings);
void    SC_ScriptError(const char *message);

extern char       *sc_String;
extern int         sc_Number;
extern int         sc_Line;
extern boolean     sc_End;
extern boolean     sc_FileScripts;
extern boolean     sc_Crossed;
extern const char *sc_ScriptsDir;


#endif // __SC_MAN_HPP__