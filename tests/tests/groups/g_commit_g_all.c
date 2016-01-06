#include <stdio.h>
#include <stdlib.h>
#include <test_utils.h>

int main(int argc, char *argv[])
{
  ASSERT (gaspi_proc_init(GASPI_BLOCK));

  ASSERT( gaspi_group_commit (GASPI_GROUP_ALL, GASPI_BLOCK) );

  ASSERT (gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

  ASSERT (gaspi_proc_term(GASPI_BLOCK));

  return EXIT_SUCCESS;

}
