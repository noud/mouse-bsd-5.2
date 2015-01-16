#include <stdlib.h>
#include <sys/sysctl.h>

#include "ps.h"

extern struct kinfo_proc2 *kinfo;

typedef struct node NODE;

struct node {
  NODE *parent;
  NODE *child;
  NODE *sibling;
  } ;

static PROCAUX *auxvec;
static int naux;
static int (*basecmp)(const void *, const void *);

static int *pidx;
static int *ppidx;
static NODE *nodes;
static NODE *roots;
static int *kidvec;

static int cmp_pid(const void *a, const void *b)
{
 return(kinfo[auxvec[*(const int *)a].inx].p_pid-kinfo[auxvec[*(const int *)b].inx].p_pid);
}

static int cmp_ppid(const void *a, const void *b)
{
 return(kinfo[auxvec[*(const int *)a].inx].p_ppid-kinfo[auxvec[*(const int *)b].inx].p_ppid);
}

static int cmp_kid(const void *a, const void *b)
{
 struct kinfo_proc2 *pa;
 struct kinfo_proc2 *pb;

 pa = kinfo + auxvec[*(const int *)a].inx;
 pb = kinfo + auxvec[*(const int *)b].inx;
 if ((pa->p_ppid == 0) && (pa->p_pid != 1))
  { if ((pb->p_ppid == 0) && (pb->p_pid != 1))
     { return((*basecmp)(auxvec+*(const int *)a,auxvec+*(const int *)b));
     }
    else
     { return(-1);
     }
  }
 else
  { if ((pb->p_ppid == 0) && (pb->p_pid != 1))
     { return(1);
     }
    else
     { return((*basecmp)(auxvec+*(const int *)a,auxvec+*(const int *)b));
     }
  }
}

static int cmp_order(const void *a, const void *b)
{
 return(((const PROCAUX *)a)->order-((const PROCAUX *)b)->order);
}

static int find_pid_x(int pid)
{
 int l;
 int m;
 int h;
 int p;

 l = -1;
 h = naux;
 while (h-l > 1)
  { m = (h + l) / 2;
    p = kinfo[auxvec[pidx[m]].inx].p_pid;
    if (p <= pid) l = m;
    if (p >= pid) h = m;
  }
 return((l==h)?pidx[l]:-1);
}

static NODE *sort_sibs(NODE *list)
{
 int i;
 NODE *n;

 i = 0;
 for (n=list;n;n=n->sibling) kidvec[i++] = n - nodes;
 if (i > 1) qsort(kidvec,i,sizeof(int),&cmp_kid);
 list = 0;
 for (i--;i>=0;i--)
  { n = &nodes[kidvec[i]];
    n->sibling = list;
    list = n;
  }
 return(list);
}

static void buildtree(NODE **listp, NODE *parentp, int parentx)
{
 int i;
 int p;
 NODE *n;
 NODE *list;

 list = 0;
 for (i=naux-1;i>=0;i--)
  { /*
     * This is annoying.  PPID is 0 for system processes - but one of
     *	them has PID 0!  PPID 0 seems to mean "no parent" rather than
     *	"parent is PID 0", so that's how we treat it.
     */
    p = kinfo[auxvec[i].inx].p_ppid;
    p = p ? find_pid_x(p) : -1;
    if (p == parentx)
     { n = nodes + i;
       n->parent = parentp;
       n->sibling = list;
       list = n;
       buildtree(&n->child,n,i);
       auxvec[i].indent = 0;
     }
  }
 *listp = sort_sibs(list);
}

static void check_orphans(void)
{
 int i;
 NODE *n;
 int any;

 any = 0;
 for (i=naux-1;i>=0;i--)
  { if (auxvec[i].indent < 0)
     { n = nodes + i;
       n->parent = 0;
       n->sibling = roots;
       roots = n;
       any = 1;
     }
  }
 if (any) roots = sort_sibs(roots);
}

static void gen_order(void)
{
 int x;

 void walk(NODE *list, int indent)
  { PROCAUX *a;
    for (;list;list=list->sibling)
     { a = auxvec + (list - nodes);
       a->indent = indent;
       a->order = x ++;
       walk(list->child,indent+1);
     }
  }

 x = 0;
 walk(roots,0);
}

void sort_hierarchical(PROCAUX *vec, int veclen, int (*cmp)(const void *, const void *))
{
 int i;

 auxvec = vec;
 naux = veclen;
 basecmp = cmp;
 pidx = malloc(naux*sizeof(int));
 ppidx = malloc(naux*sizeof(int));
 nodes = malloc(naux*sizeof(NODE));
 kidvec = malloc(naux*sizeof(int));
 for (i=naux-1;i>=0;i--)
  { pidx[i] = i;
    ppidx[i] = i;
    nodes[i].child = 0;
    auxvec[i].indent = -1;
  }
 qsort(pidx,naux,sizeof(int),&cmp_pid);
 qsort(ppidx,naux,sizeof(int),&cmp_ppid);
 roots = 0;
 buildtree(&roots,0,-1);
 check_orphans();
 gen_order();
 qsort(auxvec,naux,sizeof(PROCAUX),&cmp_order);
 free(pidx);
 free(ppidx);
 free(nodes);
 free(kidvec);
}
