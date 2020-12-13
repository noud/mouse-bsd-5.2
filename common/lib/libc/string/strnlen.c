// This file is in the public domain.
#if !defined(_KERNEL) && !defined(_STANDALONE)
#include <string.h>
#else
#include <lib/libkern/libkern.h>
#endif
size_t strnlen(const char *s, size_t maxlen)
{
 int x;

 for (x=0;x<maxlen;x++) if (! s[x]) return(x);
 return(maxlen);
}
