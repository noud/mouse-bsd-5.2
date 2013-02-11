/* { dg-do compile } */
/* { dg-options "-O1 -fdump-tree-optimized" } */

int f(_Bool x)
{
  return (x == 1);
}

/* There should be no == 1 which is produced by the front-end as
   bool_var == 1 is the same as bool_var. */
/* { dg-final { scan-tree-dump-times "== 1" 0 "optimized"} } */

/* There should be no adde for powerpc. Checking if we actually optimizated
   away the comparision.  */
/* { dg-final { scan-assembler-times "adde" 0 { target powerpc-*-* } } } */

/* { dg-final { cleanup-tree-dump "optimized" } } */
