#include "stdlib.h"
#include "stdio.h"

int CliqueCount(int *g, int graphSize)
{
  int i;
  int j;
  int k;
  int l;
  int m;
  int n;
  int count = 0;
  int subgraphSize = 6;
    
  for(i=0;i < graphSize-subgraphSize+1; i++)
    {
      for(j=i+1;j < graphSize-subgraphSize+2; j++)
	{
	  for(k=j+1;k < graphSize-subgraphSize+3; k++) 
	    { 
	      if((g[i*graphSize+j] == g[i*graphSize+k]) && 
		 (g[i*graphSize+j] == g[j*graphSize+k]))
		{
		  for(l=k+1;l < graphSize-subgraphSize+4; l++) 
		    { 
		      if((g[i*graphSize+j] == g[i*graphSize+l]) && 
			 (g[i*graphSize+j] == g[j*graphSize+l]) && 
			 (g[i*graphSize+j] == g[k*graphSize+l]))
			{
			  for(m=l+1;m < graphSize-subgraphSize+5; m++) 
			    {
			      if((g[i*graphSize+j] == g[i*graphSize+m]) && 
				 (g[i*graphSize+j] == g[j*graphSize+m]) &&
				 (g[i*graphSize+j] == g[k*graphSize+m]) && 
				 (g[i*graphSize+j] == g[l*graphSize+m])) {
				for(n=m+1; n < graphSize-subgraphSize+6; n++)
				  {
				    if((g[i*graphSize+j]
					== g[i*graphSize+n]) &&
				       (g[i*graphSize+j] 
					== g[j*graphSize+n]) &&
				       (g[i*graphSize+j] 
					== g[k*graphSize+n]) &&
				       (g[i*graphSize+j] 
					== g[l*graphSize+n]) &&
				       (g[i*graphSize+j] 
					== g[m*graphSize+n])) {
				      count++;
				    }
				  }
			      }
			    }
			}
		    }
		}
	    }
	}
    }
  return(count);
}

int CliqueCountOptimized(int *g, int gsize, int edgeIndex, int currentCount)
{
  int i;
  int j;
  int k;
  int l;
  int m;
  int sgsize = 6-1;
  int comp_color;
  
  for (i = 0; i < gsize-sgsize+1; i++)
    {
      for (j = i+1; j < gsize-sgsize+2; j++)
	{
	  comp_color = g[i*gsize+j];
	  if(edgeIndex != i*gsize+j)
	    {
	      for (k = j+1; k < gsize-sgsize+3; k++) 
		{ 
		  if ((comp_color == g[i*gsize+k]) && 
		      (comp_color == g[j*gsize+k]) &&
		      (edgeIndex != i*gsize+k) &&
		      (edgeIndex != j*gsize+k))
		    {
		      for (l = k+1; l < gsize-sgsize+4; l++) 
			{ 
			  if ((comp_color == g[i*gsize+l]) && 
			      (comp_color == g[j*gsize+l]) && 
			      (comp_color == g[k*gsize+l]) &&
			      (edgeIndex != i*gsize+l) &&
			      (edgeIndex != j*gsize+l) &&
			      (edgeIndex != k*gsize+l))
			    {
			      for (m = l+1; m < gsize-sgsize+5; m++) 
				{
				  if ((comp_color == g[i*gsize+m]) && 
				      (comp_color == g[j*gsize+m]) &&
				      (comp_color == g[k*gsize+m]) && 
				      (comp_color == g[l*gsize+m]) &&
				      (edgeIndex != i*gsize+m) &&
				      (edgeIndex != j*gsize+m) &&
				      (edgeIndex != k*gsize+m) &&
				      (edgeIndex != l*gsize+m))
				    {
				      if (comp_color == g[edgeIndex])
					{
					  currentCount++;
					}
				      else
					currentCount--;
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
  return currentCount;
}

int main()
{
  int* g = (int*)malloc(99*99*sizeof(int));
  int i;
  int count, last_count, opt_count;
  int flip_pos;

  // Randomize graph
  for(i = 0; i < 99*99; i++)
    {
      g[i] = rand() % 2;
    }

  // Get initial count
  last_count = CliqueCount(g, 99);

  // Flip one bit and test
  flip_pos = 1;
  g[flip_pos] = 1 - g[flip_pos];
  count = CliqueCount(g, 99);
  opt_count = CliqueCountOptimized(g, 99, flip_pos, last_count);
  last_count = count;

  printf("Expected: %d, Calculated: %d\n", count, opt_count);
  
  // Clean up
  free(g);

  return 0;
}
