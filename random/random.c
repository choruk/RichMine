#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <cilk/cilk.h>
#include "fifo.h"
#include "random.h"

#define MAXSIZE (512)
#define TABOOSIZE (500)
#define BIGCOUNT (9999999)

/***
 *** example of very simple search for R(6,6) counter examples
 ***
 *** starts with a small randomized graph and works its way up to successively
 *** larger graphs one at a time
 ***
 *** uses a taboo list of size #TABOOSIZE# to hold and encoding of and edge
 *** (i,j)+clique_count
 ***/

/*
 * PrintGraph
 *
 * prints in the right format for the read routine
 */
void PrintGraph(int *g, int graphSize)
{
  int i, j;
  
  fprintf(stdout,"%d\n",graphSize);
  
  for(i=0; i < graphSize; i++)
    {
      for(j=0; j < graphSize; j++)
	{
	  fprintf(stdout,"%d ",g[i*graphSize+j]);
	}
      fprintf(stdout,"\n");
    }
  fflush(stdout);
  
  return;
}

void CopyGraph(int *oldGraph, int *newGraph, int graphSize)
{
  int i, j;
  
  for (i=0; i < graphSize; i++)
    {
      for (j=0; j < graphSize; j++)
	{
	  newGraph[i*graphSize+j] = oldGraph[i*graphSize+j];
	}
    }
}

/*
***
*** returns the number of monochromatic cliques in the graph presented to
*** it
***
*** graph is stored in row-major order
*** only checks values above diagonal
*/
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

double getSeconds() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (double) (tp.tv_sec + ((1e-6)*tp.tv_usec));
}

int main(int argc,char *argv[])
{
  int *g;
  int *new_g;
  int graphSize;
  int count;
  int i, j;
  int bestCount;
  int bestI, bestJ;
  void *tabooList;

  double startTime = getSeconds();
  double currentTime;
  long elapsed, sElapsed, mElapsed, hElapsed;

  char *numProc = "16";
  __cilkrts_set_param("nworkers", numProc);
  int p = atoi(numProc);

  /*
   * start with graph of size 99
   */
  /*
  graphSize = 99;
  g = (int *)malloc(graphSize*graphSize*sizeof(int));
  if(g == NULL) {
    exit(1);
  }
  */
  /*
   * randomize buffer
   */
  /*
  size_t m = 0;
  for(m = 0; m < graphSize*graphSize; m++)
    {
      g[m] = rand() % 2;
    }
  */

  // Read in system_best.txt file
  FILE *fp;
  fp = fopen("system_best.txt", "r");
  if(fp == NULL) {
    fprintf(stdout, "Can't open system_best.txt - we are up to date\n");
  } else {
    fprintf(stdout, "Opening system_best.txt - there is a better graph available\n");

    // Read the system_best.txt graph size
    char* buf = (char*) malloc(256);
    int gs;
    fscanf(fp, "%d", &gs);
    printf("Size: %d\n", gs);
    graphSize = 777;
    /*while(fgets(buf, sizeof buf, fp) != NULL) {
      memcpy(&graphSize, buf, sizeof(graphSize));
      }*/

    // Read the system_best.txt clique count
    fscanf(fp, "%d", &gs);
    printf("Count: %d\n", gs);
    count = gs;
    /*while(fgets(buf, sizeof buf, fp) != NULL) {
      memcpy(&count, buf, sizeof(count));
      }*/

    // Read the system_best.txt graph
    g = (int *)malloc(graphSize*graphSize*sizeof(int));
    if(g == NULL) {
      exit(1);
    }
    while(fgets((char*)g, sizeof g, fp) != NULL) {
    }
    
    // Close the system_best.txt file and clean up
    free(buf);
    fclose(fp);

    // Verify
    printf("Size: %d\n", graphSize);
    printf("Count: %d\n", count);
    PrintGraph(g,graphSize);
	  
  }

  exit(1);

  // initial allocation of graphs for each processor
  int **graphs = (int **)malloc(p * sizeof(int*));
  cilk_for(i = 0; i < p; i++)
    {
      graphs[i] = (int *)malloc(graphSize*graphSize*sizeof(int));
      CopyGraph(g, graphs[i], graphSize);
    }
	
  // divide rows of matrix up evenly amongst processors
  struct GraphChunk *graphChunks = (struct GraphChunk *)malloc(p * sizeof(struct GraphChunk));
  // graphSize == # of rows
  int chunkSize = graphSize / p;
  int leftOver = graphSize % p;
  for (i = 0; i < p; i++)
    {
      graphChunks[i].size = chunkSize;
      graphChunks[i].offset = 0;
      if (leftOver > 0)
	{
	  graphChunks[i].size++;
	  leftOver--;
	}
      if (i != 0)
	{
	  graphChunks[i].offset = graphChunks[i-1].offset + graphChunks[i-1].size;
	}
    }
	
  // malloc edge flip results
  struct EdgeFlip *edgeFlipResults = (struct EdgeFlip *)malloc(p * sizeof(struct EdgeFlip));

  /*
   * make a fifo to use as the taboo list
   */
  tabooList = FIFOInitEdge(TABOOSIZE);
  if(tabooList == NULL) {
    exit(1);
  }
	
  /*
   * while we do not have a publishable result
   */
  while(graphSize < 102)
    {
      /*
       * find out how we are doing
       */
      count = CliqueCount(g,graphSize);

      /*
       * if we have a counter example
       */
      if(count == 0)
	{
	  printf("Eureka!  Counter-example found!\n");
	  fflush(stdout);
	  PrintGraph(g,graphSize);

	  /*
	   * keep going
	   */
	  continue;
	}

      /*
       * otherwise, we need to consider flipping an edge
       *
       * let's speculative flip each edge, record the new count,
       * and unflip the edge.  We'll then remember the best flip and
       * keep it next time around
       *
       * only need to work with upper triangle of matrix =>
       * notice the indices
       */
      bestCount = BIGCOUNT;
      cilk_for (i = 0; i < p; i++)
	{
	  edgeFlipResults[i].count = 0;
	  edgeFlipResults[i].bestCount = bestCount;
	  edgeFlipResults[i].bestI = 0;
	  edgeFlipResults[i].bestJ = 0;
	}

      // cilk_for over all processors
      int k;
      cilk_for(k = 0; k < p; k++)
	{
	  int myI, myJ;
	  for(myI = graphChunks[k].offset; myI < graphChunks[k].offset+graphChunks[k].size; myI++)
	    {
	      for(myJ = myI+1; myJ < graphSize; myJ++)
		{
		  /*
		   * flip it
		   */
		  graphs[k][myI*graphSize+myJ] = 1 - graphs[k][myI*graphSize+myJ];
		  edgeFlipResults[k].count = CliqueCount(graphs[k], graphSize);
			    
		  /*
		   * is it better and the i,j,count not taboo?
		   */
		  if((edgeFlipResults[k].count < edgeFlipResults[k].bestCount) && !FIFOFindEdgeCount(tabooList, myI, myJ, edgeFlipResults[k].count))
		    {
		      edgeFlipResults[k].bestCount = edgeFlipResults[k].count;
		      edgeFlipResults[k].bestI = myI;
		      edgeFlipResults[k].bestJ = myJ;
		    }
			    
		  /*
		   * flip it back
		   */
		  graphs[k][myI*graphSize+myJ] = 1 - graphs[k][myI*graphSize+myJ];
		}
	    }
	}

      for (i = 0; i < p; i++)
	{
	  if (edgeFlipResults[i].bestCount < bestCount)
	    {
	      bestCount = edgeFlipResults[i].bestCount;
	      bestI = edgeFlipResults[i].bestI;
	      bestJ = edgeFlipResults[i].bestJ;
	    }
	}
		
      if(bestCount == BIGCOUNT) {
	printf("no best edge found, terminating\n");
	exit(1);
      }
		
      /*
       * keep the best flip we saw
       */
      int newColor = 1 - g[bestI*graphSize+bestJ];
      g[bestI*graphSize+bestJ] = newColor;

      // We also need to flip this edge in all of the copied graphs
      cilk_for (i = 0; i < p; i++)
	{
	  graphs[i][bestI*graphSize+bestJ] = newColor;
	}
		
      /*
       * taboo this graph configuration so that we don't visit
       * it again
       */
      count = CliqueCount(g,graphSize);
      //FIFOInsertEdge(tabooList,best_i,best_j);
      FIFOInsertEdgeCount(tabooList,bestI,bestJ,count);

      currentTime = getSeconds();
      elapsed = currentTime - startTime;
      sElapsed = elapsed % 60;
      mElapsed = (elapsed / 60) % 60;
      hElapsed = elapsed / 3600;
		
      printf("%luh%lum%lus - ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
	     hElapsed,
	     mElapsed,
	     sElapsed,
	     graphSize,
	     bestCount,
	     bestI,
	     bestJ,
	     g[bestI*graphSize+bestJ]);
      fflush(stdout);

      /*
       * rinse and repeat
       */
    }

  // clean up
  FIFODeleteGraph(tabooList);
  free(edgeFlipResults);
  free(graphChunks);
  for (i = 0; i < p; i++)
    free(graphs[i]);
  free(graphs);
  free(g);
	
  return(0);
}
