/*
Copyright (c) Fraunhofer ITWM - Carsten Lojewski <lojewski@itwm.fhg.de>, 2013

This file is part of GPI-2.

GPI-2 is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.

GPI-2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GPI-2. If not, see <http://www.gnu.org/licenses/>.
*/

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GPI2_Env.h"

#ifdef LOADLEVELER
inline int
gaspi_handle_env(gaspi_context *ctx)
{
  char *socketPtr, *typePtr, *mfilePtr, *numaPtr;
  socketPtr = getenv ("GASPI_SOCKET");
  numaPtr = getenv ("GASPI_SET_NUMA_SOCKET");
  typePtr = getenv ("MP_CHILD");
  mfilePtr = getenv ("GASPI_MFILE");

 
  if (socketPtr)
    {
      if(typePtr)
        {
          ctx->localSocket = MAX (atoi (socketPtr), 0);

          int _my_id = atoi(typePtr);

          char *ntasks = getenv("MP_COMMON_TASKS");
          if(ntasks)
            {
              //first token has the number of partners                                                                                                                                                                                                                          
              char *s = strtok(ntasks, ":");
              do
                {
                  s = strtok(NULL, ":");
                  if(s)
                    {
                      if(atoi(s) < _my_id)
                        ctx->localSocket++;
                    }
                }
              while(s != NULL);
            }
        }
    }
  if (numaPtr)
    {
      if(atoi(numaPtr) == 1)
        {
          cpu_set_t sock_mask;
          if (gaspi_get_affinity_mask (ctx->localSocket, &sock_mask) < 0)
            {
              gaspi_print_error ("Failed to get affinity mask. NUMA not enabled!");
            }
          else
            {
              char mtyp[16];
              gaspi_machine_type (mtyp);
              if (strncmp (mtyp, "x86_64", 6) == 0)
                {
                  if (sched_setaffinity (0, sizeof (cpu_set_t), &sock_mask) != 0)
                    {
                      gaspi_print_error ("Failed to set affinity (NUMA)");
                    }
                }
            }
        }
    }
  if (!typePtr)
    {
      gaspi_print_error ("No node type defined (GASPI_TYP (or MP_CHILD))");
      return -1;
    }
  int _proc_number = atoi(typePtr);

#ifdef DEBUG
  printf("Rank %d is localSocket %d\n",
	 _proc_number,
	 ctx->localSocket);
#endif

  if(_proc_number == 0)
    {
      ctx->procType = MASTER_PROC;
    }
  
  else if (_proc_number > 0)
    {
      ctx->procType = WORKER_PROC;
    }

  else
    {
      gaspi_print_error ("Incorrect node type!\n");
      return -1;
    }

  if (mfilePtr)
    {
      snprintf (ctx->mfile, 1024, "%s", mfilePtr);
    }

  return 0;
}

#else //default environment (gaspi_run w/ ssh)
inline int
gaspi_handle_env(gaspi_context *ctx) 
{
  char *socketPtr, *typePtr, *mfilePtr, *numaPtr;
  socketPtr = getenv ("GASPI_SOCKET");
  numaPtr = getenv ("GASPI_SET_NUMA_SOCKET");
  typePtr = getenv ("GASPI_TYPE");
  mfilePtr = getenv ("GASPI_MFILE");

  if(socketPtr)
    //  ctx->localSocket = MIN(MAX(atoi(socketPtr),0),3);
    ctx->localSocket = atoi(socketPtr);
  
  if(numaPtr)
    {
      cpu_set_t sock_mask;
      if(gaspi_get_affinity_mask (ctx->localSocket, &sock_mask) < 0)
	{
	  gaspi_print_error ("Failed to get affinity mask");
	}
      else
	{
	  char mtyp[16];
	  gaspi_machine_type (mtyp);
	  if(strncmp (mtyp, "x86_64", 6) == 0){
	    if(sched_setaffinity (0, sizeof (cpu_set_t), &sock_mask) != 0)
	      {
		gaspi_print_error ("Failed to set affinity (NUMA)");
	      }
	  }
	}
    }
  
  if(!typePtr)
    {
      gaspi_print_error ("No node type defined (GASPI_TYPE)");
      return -1;
    }
  
  
  if(strcmp (typePtr, "GASPI_WORKER") == 0)
    {
      ctx->procType = WORKER_PROC;
    }
  
  else if (strcmp (typePtr, "GASPI_MASTER") == 0)
    {
      ctx->procType = MASTER_PROC;
    }
  
  else
    {
      gaspi_print_error ("Incorrect node type!\n");
      return -1;
    }
  
  if (mfilePtr)
    {
      snprintf (ctx->mfile, 1024, "%s", mfilePtr);
    }
  
  return 0;
}

#endif
