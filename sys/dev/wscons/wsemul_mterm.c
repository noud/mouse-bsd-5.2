/* This file is in the public domain. */

/* XXX paper over include file bugs */
/* Needed by <dev/wscons/wsdisplayvar.h> and likely others */
#include <sys/types.h>
/* Needed by <dev/wscons/wsconsio.h> */
#include <sys/time.h>
/* Needed by <dev/wscons/wsemulvar.h> */
#include <dev/wscons/wsconsio.h>
#include <dev/wscons/wsksymvar.h>
#include <dev/wscons/wsdisplayvar.h>

#include <sys/systm.h>
#include <sys/malloc.h>
#include <dev/wscons/wsemulvar.h>
#include <dev/wscons/wsksymdef.h>
/* Double XXX - wsdisplayvar.h can't tolerate multiple inclusion! */
/* #include <dev/wscons/wsdisplayvar.h> */

typedef struct state STATE;

struct state {
  const struct wsdisplay_emulops *ops;
  void *opcookie;
  int caps;
  unsigned int rows;
  unsigned int cols;
  long int defattr;
  long int revattr;
  long int kernattr;
  void *cbcookie;
  unsigned int atrow;
  unsigned int atcol;
  unsigned int flags;
#define SF_CONSOLE 0x00000001
#define SF_REVERSE 0x00000002
#define SF_INSERT  0x00000004
  int cseqstate;
#define CSQ_NORMAL 1
#define CSQ_CM_X   2
#define CSQ_CM_Y   3
#define CSQ_DCS    4
  } ;

static STATE cons_state;

static void *wsemul_mterm_cnattach(
	const struct wsscreen_descr *desc,
	void *desccookie,
	int atcol,
	int atrow,
	long int defattr )
{
 STATE *s;
 int rv;

 s = &cons_state;
 s->ops = desc->textops;
 s->opcookie = desccookie;
 s->caps = desc->capabilities;
 s->rows = desc->nrows;
 s->cols = desc->ncols;
 s->defattr = defattr;
 s->revattr = defattr; /* XXX fixed in _attach */
#if defined(WS_KERNEL_FG) || defined(WS_KERNEL_BG) ||	\
	defined(WS_KERNEL_COLATTR) || defined(WS_KERNEL_MONOATTR)
#ifndef WS_KERNEL_FG
#define WS_KERNEL_FG WSCOL_WHITE
#endif
#ifndef WS_KERNEL_BG
#define WS_KERNEL_BG WSCOL_BLACK
#endif
#ifndef WS_KERNEL_COLATTR
#define WS_KERNEL_COLATTR 0
#endif
#ifndef WS_KERNEL_MONOATTR
#define WS_KERNEL_MONOATTR 0
#endif
 if (s->caps & WSSCREEN_WSCOLORS)
  { rv = (*s->ops->allocattr)( s->opcookie,
			       WS_KERNEL_FG, WS_KERNEL_BG,
			       WS_KERNEL_COLATTR | WSATTR_WSCOLORS,
			       &s->kernattr );
  }
 else
  { rv = (*s->ops->allocattr)( s->opcookie,
			       0, 0,
			       WS_KERNEL_MONOATTR,
			       &s->kernattr );
  }
 if (rv) s->kernattr = defattr;
#else
 s->kernattr = defattr;
#endif
 s->cbcookie = 0;
 s->atrow = atrow;
 s->atcol = atcol;
 s->flags = SF_CONSOLE;
 s->cseqstate = CSQ_NORMAL;
 return(s);
}

static void *wsemul_mterm_attach(
	int consolep,
	const struct wsscreen_descr *desc,
	void *desccookie,
	int atcol,
	int atrow,
	void *cbcookie,
	long int defattr )
{
 STATE *s;
 int rv;

 if (consolep)
  { s = &cons_state;
    if (! (s->flags & SF_CONSOLE)) panic("wsemul_mterm_attach: console isn't");
  }
 else
  { s = malloc(sizeof(STATE),M_DEVBUF,M_WAITOK);
    s->ops = desc->textops;
    s->opcookie = desccookie;
    s->caps = desc->capabilities;
    s->rows = desc->nrows;
    s->cols = desc->ncols;
    s->defattr = defattr;
    s->revattr = defattr; /* XXX */
    s->kernattr = defattr; /* unused except for console */
    /* cbcookie set below */
    s->atrow = atrow;
    s->atcol = atcol;
    s->flags = 0;
    s->cseqstate = CSQ_NORMAL;
  }
 s->cbcookie = cbcookie;
 rv = 1;
 if (rv && (s->caps & WSSCREEN_REVERSE))
  { rv = (*s->ops->allocattr)( s->opcookie,
			       0, 0,
			       WSATTR_REVERSE,
			       &s->revattr );
  }
 if (rv && (s->caps & WSSCREEN_WSCOLORS))
  { /* This is a bit gross.  We'd really like to allocate an attribute that is
       the reverse of defattr.  But, while there is ops->allocattr to allocate
       attribute values given fg and bg values, but there is no way to bust
       open an attribute value to see what its fg and bg colours are.  This is
       semi-inevitable, since the attribute might not store the fg and bg
       values anywhere - but if WSSCREEN_WSCOLORS, it does have to.  For the
       time being, we punt on this, using WS_DEFAULT_FG and WS_DEFAULT_BG if
       they're defined, assuming white and black if not.  If defattr had been
       allocated by us, we could remember the colours we used to allocate it,
       but it wasn't - it was passed in, either to us or to _cnattach. */
    rv = (*s->ops->allocattr)( s->opcookie,
#ifdef WS_DEFAULT_BG
			       WS_DEFAULT_BG,
#else
			       WSCOL_BLACK,
#endif
#ifdef WS_DEFAULT_FG
					      WS_DEFAULT_FG,
#else
					      WSCOL_WHITE,
#endif
			       WSATTR_WSCOLORS,
			       &s->revattr );
  }
 if (rv)
  { s->revattr = s->defattr;
  }
 return(s);
}

static void limitcurs(STATE *s)
{
 if (s->atrow >= s->rows) s->atrow = s->rows - 1;
 if (s->atcol >= s->cols) s->atcol = s->cols - 1;
}

static void do_cr(STATE *s)
{
 s->atcol = 0;
}

static void do_sf(STATE *s)
{
 if (s->rows > 1) (*s->ops->copyrows)(s->opcookie,1,0,s->rows-1);
 (*s->ops->eraserows)(s->opcookie,s->rows-1,1,s->defattr);
}

static void do_lf(STATE *s)
{
 s->atrow ++;
 if (s->atrow < s->rows) return;
 s->atrow = s->rows - 1;
 do_sf(s);
}

static void do_wrap(STATE *s)
{
 do_cr(s);
 do_lf(s);
}

static void do_ic(STATE *s)
{
 if (s->atcol < s->cols-1)
  { (*s->ops->copycols)(s->opcookie,s->atrow,s->atcol,s->atcol+1,s->cols-(s->atcol+1));
  }
 (*s->ops->erasecols)(s->opcookie,s->atrow,s->atcol,1,s->defattr);
}

static void do_printing(STATE *s, unsigned char c, int kernel)
{
 if (s->flags & SF_INSERT) do_ic(s);
 (*s->ops->putchar)(s->opcookie,s->atrow,s->atcol,c,kernel?s->kernattr:(s->flags&SF_REVERSE)?s->revattr:s->defattr);
 s->atcol ++;
 if (s->atcol >= s->cols) do_wrap(s);
}

static void do_al(STATE *s)
{
 if (s->atrow < s->rows-1)
  { (*s->ops->copyrows)(s->opcookie,s->atrow,s->atrow+1,s->rows-1-s->atrow);
  }
 (*s->ops->eraserows)(s->opcookie,s->atrow,1,s->defattr);
}

static void do_beep(STATE *s)
{
 wsdisplay_emulbell(s->cbcookie);
}

static void do_bs(STATE *s)
{
 if (s->atcol > 0)
  { s->atcol --;
  }
 else if (s->atrow > 0)
  { s->atcol = s->cols - 1;
    s->atrow --;
  }
}

static void do_ce(STATE *s)
{
 (*s->ops->erasecols)(s->opcookie,s->atrow,s->atcol,s->cols-s->atcol,s->defattr);
}

static void do_cd(STATE *s)
{
 do_ce(s);
 if (s->atrow < s->rows-1)
  { (*s->ops->eraserows)(s->opcookie,s->atrow+1,s->rows-(s->atrow+1),s->defattr);
  }
}

static void do_cl(STATE *s)
{
 (*s->ops->eraserows)(s->opcookie,0,s->rows,s->defattr);
 s->atrow = 0;
 s->atcol = 0;
}

static void do_dc(STATE *s)
{
 if (s->atcol < s->cols-1)
  { (*s->ops->copycols)(s->opcookie,s->atrow,s->atcol+1,s->atcol,s->cols-(s->atcol+1));
  }
 (*s->ops->erasecols)(s->opcookie,s->atrow,s->cols-1,1,s->defattr);
}

static void do_dl(STATE *s)
{
 if (s->atrow < s->rows-1)
  { (*s->ops->copyrows)(s->opcookie,s->atrow+1,s->atrow,s->rows-(s->atrow+1));
  }
 (*s->ops->eraserows)(s->opcookie,s->rows-1,1,s->defattr);
}

static void do_do(STATE *s)
{
 s->atrow ++;
 limitcurs(s);
}

static void do_ho(STATE *s)
{
 s->atrow = 0;
 s->atcol = 0;
}

static void do_ll(STATE *s)
{
 s->atrow = s->rows - 1;
 s->atcol = 0;
}

static void do_nd(STATE *s)
{
 s->atcol ++;
 limitcurs(s);
}

static void do_sr(STATE *s)
{
 if (s->rows > 1) (*s->ops->copyrows)(s->opcookie,0,1,s->rows-1);
 (*s->ops->eraserows)(s->opcookie,0,1,s->defattr);
}

static void do_tab(STATE *s)
{
 s->atcol = (s->atcol + 8) & ~7U;
 if (s->atcol >= s->cols) do_wrap(s);
}

static void do_up(STATE *s)
{
 if (s->atrow > 0) s->atrow --;
}

static void kernel_output(STATE *s, int c)
{
 switch (c)
  { default:
       do_printing(s,c,1);
       break;
    case '\7': /* ^G */
       do_beep(s);
       break;
    case '\10': /* ^H */
       do_bs(s);
       break;
    case '\11': /* ^I */
       do_tab(s);
       break;
    case '\12': /* ^J */
       do_lf(s);
       break;
    case '\15': /* ^M */
       do_cr(s);
       break;
  }
}

static void regular_output(STATE *s, int c)
{
 switch (s->cseqstate)
  { default:
       s->cseqstate = CSQ_NORMAL;
       /* fall through */
    case CSQ_NORMAL:
       switch (c)
	{ default:
	     if (((c >= ' ') && (c <= '~')) || ((c >= 0x80+' ') && (c <= 0xff))) /* XXX iso-latin-1 */
	      { do_printing(s,c,0);
	      }
	     else if ((c >= 0x80) && (c < 0x80+' ')) /* XXX C1 -> graphics in C0 positions */
	      { do_printing(s,c-0x80,0);
	      }
	     break;
	  case '\1': /* ^A */
	     do_al(s);
	     break;
	  case '\2': /* ^B */
	     do_cd(s);
	     break;
	  case '\3': /* ^C */
	     do_ce(s);
	     break;
	  case '\6': /* ^F */
	     s->cseqstate = CSQ_CM_Y;
	     s->atrow = 0;
	     break;
	  case '\7': /* ^G */
	     do_beep(s);
	     break;
	  case '\10': /* ^H */
	     do_bs(s);
	     break;
	  case '\11': /* ^I */
	     do_tab(s);
	     break;
	  case '\12': /* ^J */
	     do_lf(s);
	     break;
	  case '\13': /* ^K */
	     do_dl(s);
	     break;
	  case '\14': /* ^L */
	     do_cl(s);
	     break;
	  case '\15': /* ^M */
	     do_cr(s);
	     break;
	  case '\16': /* ^N */
	     do_do(s);
	     break;
	  case '\17': /* ^O */
	     s->flags &= ~SF_INSERT;
	     break;
	  case '\20': /* ^P */
	     do_ho(s);
	     break;
	  case '\21': /* ^Q */
	     s->flags |= SF_INSERT;
	     break;
	  case '\22': /* ^R */
	     do_ll(s);
	     break;
	  case '\23': /* ^S */
	     do_nd(s);
	     break;
	  case '\24': /* ^T */
	     s->flags &= ~SF_REVERSE;
	     break;
	  case '\25': /* ^U */
	     do_sf(s);
	     break;
	  case '\26': /* ^V */
	     s->flags |= SF_REVERSE;
	     break;
	  case '\27': /* ^W */
	     do_sr(s);
	     break;
	  case '\30': /* ^X */
	     do_up(s);
	     break;
	  case '\31': /* ^Y */
	     do_dc(s);
	     break;
	  case '\37': /* ^_ */
	     s->cseqstate = CSQ_DCS;
	     break;
	}
       break;
    case CSQ_CM_X:
       switch (c)
	{ case '0': c = 0; if (0) {
	  case '1': c = 1; } if (0) {
	  case '2': c = 2; } if (0) {
	  case '3': c = 3; } if (0) {
	  case '4': c = 4; } if (0) {
	  case '5': c = 5; } if (0) {
	  case '6': c = 6; } if (0) {
	  case '7': c = 7; } if (0) {
	  case '8': c = 8; } if (0) {
	  case '9': c = 9; }
	     s->atcol = (s->atcol * 10) + c;
	     limitcurs(s);
	     break;
	  default:
	     s->cseqstate = CSQ_NORMAL;
	     break;
	}
       break;
    case CSQ_CM_Y:
       switch (c)
	{ case '0': c = 0; if (0) {
	  case '1': c = 1; } if (0) {
	  case '2': c = 2; } if (0) {
	  case '3': c = 3; } if (0) {
	  case '4': c = 4; } if (0) {
	  case '5': c = 5; } if (0) {
	  case '6': c = 6; } if (0) {
	  case '7': c = 7; } if (0) {
	  case '8': c = 8; } if (0) {
	  case '9': c = 9; }
	     s->atrow = (s->atrow * 10) + c;
	     limitcurs(s);
	     break;
	  default:
	     s->cseqstate = CSQ_CM_X;
	     s->atcol = 0;
	     break;
	}
       break;
    case CSQ_DCS:
       if ((c < 0x20) || ((c >= 0x7f) && (c < 0xa0)))
	{ s->cseqstate = CSQ_NORMAL;
	}
       break;
  }
}

static void wsemul_mterm_output(void *sv, const u_char *data, u_int n, int kernelp)
{
 STATE *s;
 const unsigned char *dp;
 int c;

 s = sv;
 if (kernelp && !(s->flags & SF_CONSOLE))
  { panic("kernel output not to console");
  }
 dp = (const void *) data;
 (*s->ops->cursor)(s->opcookie,0,s->atrow,s->atcol);
 for (;n>0;n--)
  { c = *dp++ & 0xff;
    if (kernelp)
     { kernel_output(s,c);
     }
    else
     { regular_output(s,c);
     }
  }
 (*s->ops->cursor)(s->opcookie,1,s->atrow,s->atcol);
}

static int wsemul_mterm_translate(
	void *cookie __attribute__((__unused__)),
	keysym_t key,
	const char **out )
{
 static char outbuf;

 if ( (KS_GROUP(key) == KS_GROUP_Keypad) &&
      ! (key & 0x80) )
  { outbuf = key & 0xff;
    *out = &outbuf;
    return(1);
  }
 return(0);
}

static void wsemul_mterm_detach(void *sv, u_int *rowp, u_int *colp)
{
 STATE *s;

 s = sv;
 *rowp = s->atrow;
 *colp = s->atcol;
 if (s != &cons_state) free(s,M_DEVBUF);
}

static void wsemul_mterm_resetop(void *sv, enum wsemul_resetops op)
{
 STATE *s;

 s = sv;
 switch (op)
  { case WSEMUL_RESET:
       s->flags &= ~(SF_REVERSE|SF_INSERT);
       break;
    case WSEMUL_CLEARSCREEN:
       (*s->ops->eraserows)(s->opcookie,0,s->rows,s->defattr);
       s->atrow = 0;
       s->atcol = 0;
       (*s->ops->cursor)(s->opcookie,1,0,0);
       break;
    default:
       break;
  }
}

const struct wsemul_ops wsemul_mterm_ops
 = { "mterm",
     &wsemul_mterm_cnattach,
     &wsemul_mterm_attach,
     &wsemul_mterm_output,
     &wsemul_mterm_translate,
     &wsemul_mterm_detach,
     &wsemul_mterm_resetop,
     0,
     0 };
