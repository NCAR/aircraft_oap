#if defined(__cplusplus)
extern "C" {
#endif

void *ResizeMem(void *object,int size);
bool StringInString(char *s1,char *s2,int caps);
char * TruncateString(char *s1,char *s2);
int StringCompare(char *s1,char *s2);
char * GetNextString( int *posn,char *record);
char * GetCurrentDir();
int SetCurrentDir(char *);
char * GetShellOutput(char *,void (*)(char *));
char * StripLeadingBlanks(char *);
char *GetNextRecord(int,FILE *,int);
char *GetNextToken(int*,char*,char *);
char **GetDirectoryNames(char *);
bool IsDirectory(char *);
char *ReplaceStringWithChar(char *,char *,char);
char *get_date_from_base(long);
char *get_time_from_base(long);
char * getAircraftName(int proj_num);
char *get_month(), *get_day(), *GetTime();
void init_date();
char *strupr();

#if defined(__cplusplus)
}
#endif
