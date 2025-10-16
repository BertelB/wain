#error Do not use
/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef REGEXP_H_INC
#define REGEXP_H_INC

/* #define NSUBEXP  10 */
#define MAX_ERR_MSG_SIZE 128

struct RegexpType
{
  char regstart;       /* Internal use only. */
  char reganch;        /* Internal use only. */
  char *regmust;       /* Internal use only. */
  int regmlen;         /* Internal use only. */
  char program[1];     /* Unwarranted chumminess with compiler. */
};

class RegExpClass
{
  RegexpType *prog;
  const char *regparse;    /* Input-scan pointer. */
  int regnpar;             /* () count. */
  char regdummy;
  char *regcode;           /* Code-emit pointer; &regdummy = don't. */
  long regsize;            /* Code size. */

  const char *reginput;    /* String-input pointer. */
  const char *regbol;      /* Beginning of input, for ^ check. */

  char *reg(int paren, int *flagp);
  char *regbranch(int *flagp);
  char *regpiece(int *flagp);
  char *regatom(int *flagp);
  char *regnode(char op);
  char *regnext(char *p);
  void regc(char b);
  void reginsert(char op, char *opnd);
  void regtail(char *p, char *val);
  void regoptail(char *p, char *val);
  const char *regtry(const char *string);
  const char *regmatch(char *prog, int nest);
  int  regrepeat(char *p);
  char err_msg[MAX_ERR_MSG_SIZE];
  void regerror(const char *msg);
  int  reg_len;
  int MaxLen;
public:
  RegExpClass();
  ~RegExpClass();
  BOOL compile(const char *exp);
  const char *find(const char *string, size_t *len = 0);
  const char *get_last_error(void);
  void clear_error(void);
};

#endif