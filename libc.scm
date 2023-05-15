;; storage allocation

extern void *malloc(int size);
extern void free(void *ptr);
extern void *realloc(void *ptr, int newsize);
extern void *calloc(int count, int eltsize);

;; character

extern int islower(int ch);
extern int isupper(int ch);
extern int isalpha(int ch);
extern int isdigit(int ch);
extern int isalnum(int ch);
extern int isxdigit(int ch);
extern int ispunct(int ch);
extern int isspace(int ch);
extern int isblank(int ch);
extern int isgraph(int ch);
extern int isprint(int ch);
extern int iscntrl(int ch);
extern int isascii(int ch);

extern int tolower(int ch);
extern int toupper(int ch);
extern int toascii(int ch);

;; string

extern int strlen(char *s);
extern int strnlen(char *s, int maxlen);
extern void *memcpy(void *to, void *from, int size);
extern void *mempcpy(void *to, void *from, int size);
extern void *memmove(void *to, void *from, int size);
extern void *memccpy(void *to, void *from, int ch, int size);
extern void *memset(void *block, int ch, int n);
extern char *strcpy(char *to, char *from);
extern char *strncpy(char *to, char *from, int size);
extern char *strdup(char *s);
extern char *strndup(char *s, int size);
extern char *strcat(char *to, char *from);
extern char *strncat(char *to, char *from, int size);
extern void bcopy(void *from, void *to, int size);
extern void bzero(void *block, int size);
extern int memcmp(void *a1, void *a2, int size);
extern int strcmp(char *s1, char *s2);
extern int strcasecmp(char *s1, char *s2);
extern int strncmp(char *s1, char *s2, int size);
extern int bcmp(void *a1, void *a2, int size);

;; search

extern void *memchr(void *block, int ch, int size);
extern char *strchr(char *s, int ch);
extern char *strrchr(char *s, int ch);
extern char *strstr(char *haystack, char *needle);
extern void *memmem(void *haystack, int haystack_len, void *needle,
                    int needle_len);
extern int strspn(char *s, char *skipset);
extern int strcspn(char *s, char *stopset);
extern char *strpbrk(char *s, char *stopset);
extern char *strtok(char *newstring, char *delimiters);
extern char *strtok_r(char *newstring, char *delimiters, char **save_ptr);
extern char *strsep(char **string_ptr, char *delimiters);

;; standard streams

typedef struct _IO_FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
extern FILE *fopen(char *filename, char *opentype);
extern FILE *freopen(char *filename, char *opentype, FILE *stream);
extern int fclose(FILE *stream);
extern int fcloseall(void);
extern int fputc(int ch, FILE *stream);
extern int fputs(char *s, FILE *stream);
extern int puts(char *s);
extern int fgetc(FILE *stream);
extern int getchar(void);


var EOF = -1;

