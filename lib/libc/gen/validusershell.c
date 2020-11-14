/* This file is in the public domain. */

#include "namespace.h"

#include <string.h>
#include <unistd.h>

#ifdef __weak_alias
__weak_alias(validusershell,_validusershell)
#endif

int validusershell(const char *shell)
{
 char *s;
 int rv;

 setusershell();
 while (1)
  { s = getusershell();
    if (! s)
     { rv = 0;
       break;
     }
    if (!strcmp(s,shell))
     { rv = 1;
       break;
     }
  }
 endusershell();
 return(rv);
}
