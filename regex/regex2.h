/*
 * First, the stuff that ends up in the outside-world include file
 = typedef off_t regoff_t;
 = typedef struct {
 = 	int re_magic;
 = 	size_t re_nsub;		// number of parenthesized subexpressions
 = 	const char *re_endp;	// end pointer for REG_PEND
 = 	struct re_guts *re_g;	// none of your business :-)
 = } regex_t;
 = typedef struct {
 = 	regoff_t rm_so;		// start of match
 = 	regoff_t rm_eo;		// end of match
 = } regmatch_t;
 */
/*
 * internals of regex_t
 */
#define	MAGIC1	((('r'^0200)<<8) | 'e')

/*
 * The internal representation is a *strip*, a sequence of
 * operators ending with an endmarker.  (Some terminology etc. is a
 * historical relic of earlier versions which used multiple strips.)
 * Certain oddities in the representation are there to permit running
 * the machinery backwards; in particular, any deviation from sequential
 * flow must be marked at both its source and its destination.  Some
 * fine points:
 *
 * - OPLUS_ and O_PLUS are *inside* the loop they create.
 * - OQUEST_ and O_QUEST are *outside* the bypass they create.
 * - OCH_ and O_CH are *outside* the multi-way branch they create, while
 *   OOR1 and OOR2 are respectively the end and the beginning of one of
 *   the branches.  Note that there is an implicit OOR2 following OCH_
 *   and an implicit OOR1 preceding O_CH.
 *
 * In state representations, an operator's bit is on to signify a state
 * immediately *preceding* "execution" of that operator.
 */
typedef long sop;		/* strip operator */
typedef long sopno;
#define	OPRMASK	0x7c000000
#define	OPDMASK	0x03ffffff
#define	OPSHIFT	(26)
#define	OP(n)	((n)&OPRMASK)
#define	OPND(n)	((n)&OPDMASK)
#define	SOP(op, opnd)	((op)|(opnd))
/* operators			   meaning	operand			*/
/*						(back, fwd are offsets)	*/
#define	OEND	(1<<OPSHIFT)	/* endmarker	-			*/
#define	OCHAR	(2<<OPSHIFT)	/* character	unsigned char		*/
#define	OBOL	(3<<OPSHIFT)	/* left anchor	-			*/
#define	OEOL	(4<<OPSHIFT)	/* right anchor	-			*/
#define	OANY	(5<<OPSHIFT)	/* .		-			*/
#define	OANYOF	(6<<OPSHIFT)	/* [...]	set number		*/
#define	OBACK_	(7<<OPSHIFT)	/* begin \d	paren number		*/
#define	O_BACK	(8<<OPSHIFT)	/* end \d	paren number		*/
#define	OPLUS_	(9<<OPSHIFT)	/* + prefix	fwd to suffix		*/
#define	O_PLUS	(10<<OPSHIFT)	/* + suffix	back to prefix		*/
#define	OQUEST_	(11<<OPSHIFT)	/* ? prefix	fwd to suffix		*/
#define	O_QUEST	(12<<OPSHIFT)	/* ? suffix	back to prefix		*/
#define	OLPAREN	(13<<OPSHIFT)	/* (		fwd to )		*/
#define	ORPAREN	(14<<OPSHIFT)	/* )		back to (		*/
#define	OCH_	(15<<OPSHIFT)	/* begin choice	fwd to OOR2		*/
#define	OOR1	(16<<OPSHIFT)	/* | pt. 1	back to OOR1 or OCH_	*/
#define	OOR2	(17<<OPSHIFT)	/* | pt. 2	fwd to OOR2 or O_CH	*/
#define	O_CH	(18<<OPSHIFT)	/* end choice	back to OOR1		*/
#define	OBOW	(19<<OPSHIFT)	/* begin word	-			*/
#define	OEOW	(20<<OPSHIFT)	/* end word	-			*/

/***
typedef struct {
  uch *ptr;		// -> uch [csetsize]
  uch mask;		// bit within array
  uch hash;		// hash code 
} cset;
***/

typedef struct {
  int nralloc;
  int nr;
  int *rbuf;
  int invert;
} cset;

/* note that CHadd and CHsub are unsafe, and CHIN doesn't yield 0/1 */
//#define	CHadd(cs, c)	((cs)->ptr[(uch)(c)] |= (cs)->mask, (cs)->hash += (c))
//#define	CHsub(cs, c)	((cs)->ptr[(uch)(c)] &= ~(cs)->mask, (cs)->hash -= (c))
//#define	CHIN(cs, c)	((cs)->ptr[(uch)(c)] & (cs)->mask)

#define CHadd(cs, c) chadd(cs, c)
#define CHrange(cs, from, to) chrange(cs, from, to)
#define CHIN(cs, c)  chin(cs, c)

#ifdef REGEX2_CHADD
static
void chadd(cset *cs, int ch)
{
  if( cs->nr >= cs->nralloc ){
    cs->nralloc += 10;
    cs->rbuf = realloc(cs->rbuf, sizeof(int)*cs->nralloc);
    if( cs->rbuf == 0 ){
      fprintf(stderr, "out of memory (chadd)\n");
      exit(1);
    }
  }
  cs->rbuf[cs->nr++] = (ch<<16)|ch;
}
#endif

#ifdef REGEX2_CHRANGE
static
void chrange(cset *cs, int from, int to)
{
  if( cs->nr >= cs->nralloc ){
    cs->nralloc += 10;
    cs->rbuf = realloc(cs->rbuf, sizeof(int)*cs->nralloc);
    if( cs->rbuf == 0 ){
      fprintf(stderr, "out of memory (chadd)\n");
      exit(1);
    }
  }
  cs->rbuf[cs->nr++] = (to<<16)|from;
}
#endif

#ifdef REGEX2_CHIN
static
int chin_x(cset *cs, int ch)
{
  int i, pair, low, high;

  for(i=0;i<cs->nr;++i){
    pair = cs->rbuf[i];
    low = pair & 0xFFFF;
    high = (pair>>16) & 0xFFFF;
    if( low <= ch && ch <= high )
      return 1;
  }
  return 0;
}

static
int chin(cset *cs, int ch)
{
  return cs->invert ? (!chin_x(cs, ch)) : chin_x(cs, ch);
}
#endif     // REGEX2_CHIN

/* stuff for character categories */
typedef unsigned char cat_t;

/*
 * main compiled-expression structure
 */
struct re_guts {
	int magic;
#		define	MAGIC2	((('R'^0200)<<8)|'E')
	sop *strip;		/* malloced area for strip */
	int csetsize;		/* number of bits in a cset vector */
	int ncsets;		/* number of csets in use */
	cset *sets;		/* -> cset [ncsets] */
  /* uch *setbits; */		/* -> uch[csetsize][ncsets/CHAR_BIT] */
	int cflags;		/* copy of regcomp() cflags argument */
	sopno nstates;		/* = number of sops */
	sopno firststate;	/* the initial OEND (normally 0) */
	sopno laststate;	/* the final OEND */
	int iflags;		/* internal flags */
#		define	USEBOL	01	/* used ^ */
#		define	USEEOL	02	/* used $ */
#		define	BAD	04	/* something wrong */
	int nbol;		/* number of ^ used */
	int neol;		/* number of $ used */
	char *must;		/* match must contain this string */
	int mlen;		/* length of must */
	size_t nsub;		/* copy of re_nsub */
	int backrefs;		/* does it use back references? */
	sopno nplus;		/* how deep does it nest +s? */
};

/* misc utilities */
/* #define	OUT	(CHAR_MAX+1) */	/* a non-character value */
#define OUT     0x10000
#define	ISWORD(c)	(isalnum(c) || (c) == '_')
