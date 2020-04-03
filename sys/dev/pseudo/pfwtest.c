#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

extern const char *__progname;

typedef struct ftn FTN;

struct ftn {
  FTN *u;
  FTN *l;
  FTN *r;
  int bal;
  unsigned int addr;
  } ;

static FTN *ftnr;

static void panic(const char *) __attribute__((__noreturn__));
static void panic(const char *s)
{
 fprintf(stderr,"%s: panic: %s\n",__progname,s);
 fflush(0);
 abort();
}

static void dumptree(FTN *) __attribute__((__unused__));
static void dumptree(FTN *n)
{
 static void dumpit(FTN *n, int depth)
  { if (n == 0) return;
    dumpit(n->l,depth+2);
    printf("%*s",depth,"");
    if (n->bal == 0)
     { putchar('=');
     }
    else
     { char c;
       int i;
       c = (n->bal < 0) ? '-' : '+';
       for (i=abs(n->bal);i>0;i--) putchar(c);
     }
    printf("%08x\n",n->addr);
    dumpit(n->r,depth+2);
  }

 dumpit(n,0);
}

static int rebalance(FTN **up, FTN *uptr)
{
 FTN *u;
 FTN *f;
 FTN *b;
 FTN *c;

 u = *up;
 switch (u->bal)
  { case 0:
       return(0);
       break;
    case -1: case 1:
       return(1);
       break;
    case -2:
       if (u->l->bal <= 0)
	{ u->bal = -1 - u->l->bal;
	  u->l->bal ++;
	  *up = u->l;
	  u->l->u = uptr;
	  f = u->l->r;
	  u->l->r = u;
	  u->u = u->l;
	  u->l = f;
	  if (f) f->u = u;
	  if (u->bal) return(1);
	}
       else if (u->l->bal > 0)
	{ f = u->l->r;
	  b = f->l;
	  c = f->r;
	  *up = f;
	  f->u = uptr;
	  f->l = u->l;
	  f->l->u = f;
	  f->r = u;
	  u->u = f;
	  f->l->r = b;
	  if (b) b->u = f->l;
	  u->l = c;
	  if (c) c->u = u;
	  f->l->bal = (f->bal > 0) ? -1 : 0;
	  f->r->bal = (f->bal < 0) ?  1 : 0;
	  f->bal = 0;
	}
       return(0);
       break;
    case 2:
       if (u->r->bal >= 0)
	{ u->bal = 1 - u->r->bal;
	  u->r->bal --;
	  *up = u->r;
	  u->r->u = uptr;
	  f = u->r->l;
	  u->r->l = u;
	  u->u = u->r;
	  u->r = f;
	  if (f) f->u = u;
	  if (u->bal) return(1);
	}
       else if (u->r->bal < 0)
	{ f = u->r->l;
	  b = f->r;
	  c = f->l;
	  *up = f;
	  f->u = uptr;
	  f->r = u->r;
	  f->r->u = f;
	  f->l = u;
	  u->u = f;
	  f->r->l = b;
	  if (b) b->u = f->r;
	  u->r = c;
	  if (c) c->u = u;
	  f->r->bal = (f->bal < 0) ?  1 : 0;
	  f->l->bal = (f->bal > 0) ? -1 : 0;
	  f->bal = 0;
	}
       return(0);
       break;
  }
 panic("pfw: impossible balance");
}

static int insert(FTN *f, FTN **up, FTN *uptr)
{
 FTN *u;

 u = *up;
 if (! u)
  { *up = f;
    f->u = uptr;
    return(1);
  }
 if (f->addr < u->addr)
  { if (insert(f,&u->l,u))
     { u->bal --;
       return(rebalance(up,uptr));
     }
  }
 else if (f->addr > u->addr)
  { if (insert(f,&u->r,u))
     { u->bal ++;
       return(rebalance(up,uptr));
     }
  }
 else
  { panic("pfw: inserting duplicate");
  }
 return(0);
}

static void search_insert(FTN *f)
{
 f->l = 0;
 f->r = 0;
 f->bal = 0;
 insert(f,&ftnr,0);
}

static void search_delete(FTN *f)
{
 FTN *u;
 FTN *l;
 FTN *r;
 FTN **up;
 int dr;
 FTN *s;

 u = f->u;
 l = f->l;
 r = f->r;
 up = u ? (u->l == f) ? &u->l : &u->r : &ftnr;
 dr = u ? (u->l == f) ? 1 : -1 : 0;
 if (! f->r)
  { if (! f->l)
     { *up = 0;
     }
    else
     { f->l->u = u;
       *up = f->l;
     }
  }
 else if (! f->l)
  { f->r->u = u;
    *up = f->r;
  }
 else if (! f->r->l)
  { f->r->l = f->l;
    f->l->u = f->r;
    f->r->u = u;
    *up = f->r;
    u = f->r;
    u->bal = f->bal;
    dr = -1;
  }
 else
  { s = f->r;
    while (s->l) s = s->l;
    s->u->l = s->r;
    if (s->r) s->r->u = s->u;
    s->l = f->l;
    f->l->u = s;
    s->r = f->r;
    f->r->u = s;
    s->bal = f->bal;
    f = s->u;
    s->u = u;
    *up = s;
    u = f;
    dr = 1;
  }
 if (u)
  { u->bal += dr;
    while <"delrebal"> (1)
     { switch (u->bal)
	{ case 0:
	     if (u->u)
	      { u->u->bal += (u == u->u->l) ? 1 : -1;
		u = u->u;
		continue;
	      }
	     break <"delrebal">;
	     break;
	  case -1:
	  case 1:
	     break <"delrebal">;
	     break;
	  case -2:
	  case 2:
	      { FTN *v;
		v = u->u;
		if (v)
		 { int ob;
		   ob = v->bal;
		   v->bal += (u == v->l) ? 1 : -1;
		   if (rebalance((u==v->l)?&v->l:&v->r,v))
		    { v->bal = ob;
		      break <"delrebal">;
		    }
		   u = v;
		   continue;
		 }
		rebalance(&ftnr,0);
		break <"delrebal">;
	      }
	     break;
	  default:
	     panic("pfw: impossible delete rebalance");
	     break;
	}
     }
  }
}

static int desn;
static int ncont;
static int acont;
static unsigned int *cont;

static void verify_tree(void)
{
 int x;

 static void walk(FTN *n)
  { if (n == 0) return;
    walk(n->l);
    if (x >= ncont) panic("verify past ncont");
    if (n->addr != cont[x]) panic("verify value wrong");
    x ++;
    walk(n->r);
  }

 static int bcheck(FTN *n)
  { int l;
    int r;
    if (n == 0) return(0);
    l = bcheck(n->l);
    r = bcheck(n->r);
    if ( ((l == r+1) && (n->bal == -1)) ||
	 ((l == r) && (n->bal == 0)) ||
	 ((l == r-1) && (n->bal == 1)) ) return(((l>r)?l:r)+1);
    panic("balance wrong");
  }

 x = 0;
 walk(ftnr);
 if (x != ncont) panic("verify ncont wrong");
 bcheck(ftnr);
}

static FTN *val_present(unsigned int v)
{
 FTN *f;

 f = ftnr;
 while (1)
  { if (! f) return(0);
    if (v == f->addr) return(f);
    f = (v < f->addr) ? f->l : f->r;
  }
}

static void val_insert(unsigned int val)
{
 FTN *f;
 int a;
 int b;
 int c;

 f = malloc(sizeof(FTN));
 f->addr = val;
 search_insert(f);
 if (ncont >= acont) cont = realloc(cont,(acont=ncont+16)*sizeof(*cont));
 a = -1;
 c = ncont;
 while (c-a > 1)
  { b = (a + c) >> 1;
    if (val < cont[b]) c = b; else a = b;
  }
 if ((a >= 0) && (val == cont[a])) panic("impossible insert");
 if (c < ncont) bcopy(cont+c,cont+c+1,(ncont-c)*sizeof(*cont));
 cont[c] = val;
 ncont ++;
}

static void val_delete(unsigned int val)
{
 FTN *f;
 int a;
 int b;
 int c;

 f = val_present(val);
 if (! f) panic("delete: missing");
 search_delete(f);
 free(f);
 a = -1;
 c = ncont;
 while (c-a > 1)
  { b = (a + c) >> 1;
    if (val < cont[b]) c = b; else a = b;
  }
 if (val != cont[a]) panic("impossible delete");
 if (c < ncont) bcopy(cont+c,cont+a,(ncont-c)*sizeof(*cont));
 ncont --;
}

static int want_insert(void)
{
 int e;
 int i;

 if (ncont == 0) return(1);
 if (ncont < desn)
  { e = ncont;
    for (i=0;e<desn;i++,e<<=1) ;
    return(random()&~((~0U)<<i));
  }
 e = desn + desn - ncont;
 if (e <= 0) return(0);
 for (i=0;e<desn;i++,e<<=1) ;
 return(!(random()&~((~0U)<<i)));
}

int main(void);
int main(void)
{
 int oloops;
 int nloops;
 unsigned int val;

 ftnr = 0;
 ncont = 0;
 acont = 0;
 cont = 0;
 desn = 8;
 oloops = 0;
 while (1)
  {  { int n;
       int od;
       od = desn;
       n = (random() & 7) + 1;
       desn = (random() & ~((~0U) << n)) | (1 << n);
       printf("desn %d\n",desn);
       nloops = ((desn > od) ? desn : od) << 3;
     }
    oloops ++;
    for (;nloops>0;nloops--)
     { verify_tree();
       if (want_insert())
	{ do val = random(); while (val_present(val));
	  printf("%d.%d: insert %08x\n",oloops,nloops,val);
	  val_insert(val);
	}
       else
	{ val = cont[random()%ncont];
	  printf("%d.%d: delete %08x\n",oloops,nloops,val);
	  val_delete(val);
	}
     }
  }
}
