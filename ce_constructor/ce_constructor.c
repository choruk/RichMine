#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cilk/cilk.h>
#include "fifo.h"
#include "ce_constructor.h"

#define SYMMETRIC_RAMSEY_NUMBER 6
#define TABOO_SIZE 500
#define BIG_COUNT 9999999

void printUsageAndExit()
{
  printf("Unexpected usage!\n");
  printf("usage: ./ce_constructor []");
  exit(-1);
}

/*
 * PrintGraph
 * - copied from Rich's example code
 * prints in the right format for the read routine
 */
void PrintGraph(int *g, int gsize)
{
	int i;
	int j;

	fprintf(stdout,"%d\n",gsize);

	for(i=0; i < gsize; i++)
	{
		for(j=0; j < gsize; j++)
		{
			fprintf(stdout,"%d ",g[i*gsize+j]);
		}
		fprintf(stdout,"\n");
	}

	return;
}

/*
 *** - copied from Rich's example code
 *** returns the number of monochromatic cliques in the graph presented to
 *** it
 ***
 *** graph is stored in row-major order
 *** only checks values above diagonal
 */

int CliqueCount(int *g, int gsize)
{
  int i;
  int j;
  int k;
  int l;
  int m;
  int n;
  int count=0;
  int sgsize = SYMMETRIC_RAMSEY_NUMBER;
    
  for (i = 0; i < gsize-sgsize+1; i++)
  {
    for (j = i+1; j < gsize-sgsize+2; j++)
    {
      for (k = j+1; k < gsize-sgsize+3; k++) 
      { 
        if ((g[i*gsize+j] == g[i*gsize+k]) && 
            (g[i*gsize+j] == g[j*gsize+k]))
        {
          for (l = k+1; l < gsize-sgsize+4; l++) 
          { 
            if ((g[i*gsize+j] == g[i*gsize+l]) && 
                (g[i*gsize+j] == g[j*gsize+l]) && 
                (g[i*gsize+j] == g[k*gsize+l]))
            {
              for (m = l+1; m < gsize-sgsize+5; m++) 
              {
                if ((g[i*gsize+j] == g[i*gsize+m]) && 
                    (g[i*gsize+j] == g[j*gsize+m]) &&
                    (g[i*gsize+j] == g[k*gsize+m]) && 
                    (g[i*gsize+j] == g[l*gsize+m]))
                {
                  for(n = m+1; n < gsize-sgsize+6; n++)
                  {
                    if ((g[i*gsize+j] == g[i*gsize+n]) &&
                        (g[i*gsize+j] == g[j*gsize+n]) &&
                        (g[i*gsize+j] == g[k*gsize+n]) &&
                        (g[i*gsize+j] == g[l*gsize+n]) &&
                        (g[i*gsize+j] == g[m*gsize+n]))
                    {
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
  return count;
}

/*
 * CopyGraph 
 *
 * copys the contents of old_g to corresponding locations in new_g
 * leaving other locations in new_g alone
 * that is
 * 	new_g[i,j] = old_g[i,j]
 */
void CopyGraph(int *old_g, int o_gsize, int *new_g, int n_gsize)
{
	int i;
	int j;

	/*
	 * new g must be bigger or equal
	 */
	if (n_gsize < o_gsize)
		return;

	cilk_for (i=0; i < o_gsize; i++)
	{
		for (j=0; j < o_gsize; j++)
		{
			new_g[i*n_gsize+j] = old_g[i*o_gsize+j];
		}
	}

	return;
}

int main(int argc, char *argv[])
{
  char *numProc = "4";
  if (argc > 1)
  {
    numProc = argv[1];
  }
  __cilkrts_set_param("nworkers", numProc);
  printf("# of workers: %d\n", __cilkrts_get_nworkers());
  int p = atoi(numProc);
  int count, bestCount;
  int i, j, bestI, bestJ;
  int *newGraph;
  int initialGraphSize = 8;
	/*
	 * start with graph of size 8
	 */
	int graphSize = initialGraphSize;
	int *graph = (int *)malloc(graphSize*graphSize*sizeof(int));
	if (graph == NULL) {
		exit(1);
	}
	/*
	 * make a fifo to use as the taboo list
	 */
	void *tabooList = FIFOInitEdge(TABOO_SIZE);
	if (tabooList == NULL) {
		exit(1);
	}
	/*
	 * start out with all zeros
	 */
	memset(graph, 0, graphSize*graphSize*sizeof(int));
	/*
	 * while we do not have a publishable result
	 */
	while(graphSize < 102)
	{
		/*
		 * find out how we are doing
		 */
		count = CliqueCount(graph, graphSize);
		/*
		 * if we have a counter example
		 */
		if (count == 0)
		{
			printf("Eureka!  Counter-example found!\n");
			PrintGraph(graph, graphSize);
			/*
			 * make a new graph one size bigger
			 */
			newGraph = (int *)malloc((graphSize+1) * (graphSize+1) * sizeof(int));
			if (newGraph == NULL)
				exit(1);
			/*
			 * copy the old graph into the new graph leaving the
			 * last row and last column alone
			 */
			CopyGraph(graph, graphSize, newGraph, graphSize+1);
			/*
			 * zero out the last column and last row
			 */
			for (i = 0; i < (graphSize+1); i++)
			{
				newGraph[i * (graphSize+1) + graphSize] = 0; // last column
				newGraph[graphSize * (graphSize+1) + i] = 0; // last row
			}
			/*
			 * throw away the old graph and make new one the
			 * graph
			 */
			free(graph);
			graph = newGraph;
			graphSize = graphSize+1;
			/*
			 * reset the taboo list for the new graph
			 */
			tabooList = FIFOResetEdge(tabooList);
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
    // Need a copy of graph for each processor
    int **graphs = (int **)malloc(p * sizeof(int*));
    cilk_for (i = 0; i < p; i++)
    {
      graphs[i] = (int *)malloc(graphSize*graphSize*sizeof(int));
      CopyGraph(graph, graphSize, graphs[i], graphSize);
    }
    // Now we need to divide the "rows" of the matrix up evenly among processors
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
    // We use this to check later
    bestCount = BIG_COUNT;
    struct EdgeFlip *edgeFlipResults = (struct EdgeFlip *)malloc(p * sizeof(struct EdgeFlip));
    cilk_for (i = 0; i < p; i++)
    {
      edgeFlipResults[i].count = 0;
      edgeFlipResults[i].bestCount = bestCount;
      edgeFlipResults[i].bestI = 0;
      edgeFlipResults[i].bestJ = 0;
    }
    int k;
    // cilk_for over all processors
    cilk_for (k = 0; k < p; k++)
    {
      // only look at the rows that this processor is responsible for
      for (i = graphChunks[k].offset; i < graphChunks[k].offset+graphChunks[k].size; i++)
  		{
  			for (j = i+1; j < graphSize; j++)
  			{
  				/*
  				 * flip it
  				 */
  				graphs[k][i*graphSize+j] = 1 - graphs[k][i*graphSize+j];
          edgeFlipResults[k].count = CliqueCount(graphs[k], graphSize);
  				/*
  				 * is it better and the i,j,count not taboo?
  				 */
  				if ((edgeFlipResults[k].count < edgeFlipResults[k].bestCount) && !FIFOFindEdgeCount(tabooList, i, j, edgeFlipResults[k].count))
  //					!FIFOFindEdge(taboo_list,i,j))
  				{
  					edgeFlipResults[k].bestCount = edgeFlipResults[k].count;
  					edgeFlipResults[k].bestI = i;
  					edgeFlipResults[k].bestI = j;
  				}

  				/*
  				 * flip it back
  				 */
  				graphs[k][i*graphSize+j] = 1 - graphs[k][i*graphSize+j];
  			}
  		}
    }
    // Free memory we dont need anymore
    free(graphs);
    free(graphChunks);
    for (i = 0; i < p; i++)
    {
      if (edgeFlipResults[i].bestCount < bestCount)
      {
        bestCount = edgeFlipResults[i].bestCount;
        bestI = edgeFlipResults[i].bestI;
        bestJ = edgeFlipResults[i].bestJ;
      }
    }
    free(edgeFlipResults);
    // Did our move make an improvement?
		if (bestCount == BIG_COUNT) {
			printf("no best edge found, terminating\n");
			exit(1);
		}
		/*
		 * keep the best flip we saw
		 */
		graph[bestI*graphSize+bestJ] = 1 - graph[bestI*graphSize+bestJ];
		/*
		 * taboo this graph configuration so that we don't visit
		 * it again
		 */
		count = CliqueCount(graph, graphSize);
    // FIFOInsertEdge(tabooList, bestI, bestJ);
    FIFOInsertEdgeCount(tabooList, bestI, bestJ, count);

		printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
			graphSize,
			bestCount,
			bestI,
			bestJ,
			graph[bestI*graphSize+bestJ]
    );
		/*
		 * rinse and repeat
		 */
	}
  // Clean up
	FIFODeleteGraph(tabooList);
  
  return 0;
}