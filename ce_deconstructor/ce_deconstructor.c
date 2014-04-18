#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fifo.h"
#include "ce_deconstructor.h"

// #define DEBUG_1

#define GRAPH_SIZE 101
#define SYMMETRIC_RAMSEY_NUMBER 6
#define BIGCOUNT 9999999
#define TABOOSIZE 500
#define OUTPUT_GRAPH_SIZE 87

int* loadGraphFromFile(char* filename, int graphSize)
{
  FILE *graphFile = fopen(filename, "r");
  int nodeCount = 0;
	if (graphFile == NULL)
  {
		printf("Error reading file.\n");
		exit(-1);
	}
  // malloc the graph
  int *outGraph = (int *)malloc(graphSize*graphSize*sizeof(int));
  int fScan = 0;
  // read it in from the input file
  while (fScan >= 0)
  {
    fScan = fscanf(graphFile, "%d", &outGraph[nodeCount++]);
  }
  // check the size matches
	if (nodeCount-1 != graphSize*graphSize)
  {
		printf(
      "Incorrect number of values scanned: nodeCount=%d, graphSize=%d.\n",
      nodeCount,
      graphSize
    );
		exit(-1);
	}
  
  return outGraph;
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
    
  for (i=0;i < gsize-sgsize+1; i++)
  {
    for (j=i+1;j < gsize-sgsize+2; j++)
    {
      for (k=j+1;k < gsize-sgsize+3; k++) 
      { 
        if ((g[i*gsize+j] == g[i*gsize+k]) && 
            (g[i*gsize+j] == g[j*gsize+k]))
        {
          for (l=k+1;l < gsize-sgsize+4; l++) 
          { 
            if ((g[i*gsize+j] == g[i*gsize+l]) && 
                (g[i*gsize+j] == g[j*gsize+l]) && 
                (g[i*gsize+j] == g[k*gsize+l]))
            {
              for (m=l+1;m < gsize-sgsize+5; m++) 
              {
                if ((g[i*gsize+j] == g[i*gsize+m]) && 
                    (g[i*gsize+j] == g[j*gsize+m]) &&
                    (g[i*gsize+j] == g[k*gsize+m]) && 
                    (g[i*gsize+j] == g[l*gsize+m]))
                {
                  for(n=m+1; n < gsize-sgsize+6; n++)
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
 * - adapted from Rich's example code
 * Copys the contents of oldGraph to the output graph, removing the node
 *  specified by nodeToRemove.
 * In other words, newGraph becomes oldGraph minus one skipped row and one
 *  skipped column corresponding to the node that should be removed.
 */
int* CopyGraph(int *oldGraph, int oldGraphSize, int nodeToRemove) // , int *new_g, int n_gsize
{
	int i;
	int j;
  int outputGraphSize = oldGraphSize - 1;
  int *outputGraph = (int *)malloc(outputGraphSize*outputGraphSize*sizeof(int));
  int shouldIncrementOldI = 0;
  int shouldIncrementOldJ = 0;
  
	for (i = 0; i < outputGraphSize; i++)
	{
    // Need to differentiate between the two graph indecies
    int iOld = i;
    // We want to skip the i-th row
    if (iOld == nodeToRemove)
    {
      // Next row please
      iOld++;
      shouldIncrementOldI = 1;
    }
    // Make sure we dont include the row after the skipped row twice
    else if (shouldIncrementOldI)
    {
      // Next row please
      iOld++;
    }
    shouldIncrementOldJ = 0;
		for (j = 0; j < outputGraphSize; j++)
		{
      int jOld = j;
      // We also want to skip the j-th column
      if (j == nodeToRemove)
      {
        jOld++;
        shouldIncrementOldJ = 1;
      }
      else if (shouldIncrementOldJ)
      {
        jOld++;
      }
      // Copy the edge
			outputGraph[i*outputGraphSize+j] = oldGraph[iOld*oldGraphSize+jOld];
		}
	}

	return outputGraph;
}

void writeGraphToFile(int *graph, int graphSize, char *filename)
{
  FILE *graphFile = fopen(filename, "w");
  int nodeCount = 0;
	if (graphFile == NULL)
  {
		printf("Error opening output file.\n");
		exit(-1);
	}
  int i;
  int rowCount = 0;
	for (i = 0; i < graphSize*graphSize; i++)
  {
    if (rowCount == graphSize)
    {
      fprintf(graphFile, "\n");
      rowCount = 0;
    }
    if (rowCount == graphSize-1)
    {
      fprintf(graphFile, "%d", graph[i]);
    }
    else
    {
      fprintf(graphFile, "%d ", graph[i]);
    }
    rowCount++;
 	}
  fclose(graphFile);
#ifdef DEBUG_1
  printf("Wrote graph to %s", filename);
#endif
}

int main(int argc, char *argv[])
{
  MPI_Init( &argc, &argv );
  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
/*  
  // All processors should load the same initial graph from the file
  char *graphFileName = "deconstructor.input";
	int *graph = loadGraphFromFile(graphFileName, GRAPH_SIZE);
#ifdef DEBUG_1
  printf("Input graph:\n");
  PrintGraph(graph, GRAPH_SIZE);
#endif
  // Then we want each of them to explore different areas...
  // 0-indexed
  int node = GRAPH_SIZE;
  int *smallerGraph = CopyGraph(graph, GRAPH_SIZE, node);
  printf("Smaller graph:\n");
  PrintGraph(smallerGraph, GRAPH_SIZE-1);
  free(graph);
  free(smallerGraph);
*/
	int desiredOutputSize = OUTPUT_GRAPH_SIZE;
  if (argc > 1)
  {
    desiredOutputSize = atoi(argv[1]);
  }
  char *desiredOutputFile = "deconstructor.output";
  if (argc > 2)
  {
    desiredOutputFile = argv[2];
  }
  int count;
	int i;
	int j;
	int best_count;
	int best_i;
	int best_j;
	void *taboo_list;
  
  if (size == 1)
  {
    printf("Sequential\n");
    // Load the initial graph from the file
    char *graphFileName = "deconstructor.input";
    int startingGraphSize = GRAPH_SIZE;
  	int *startingGraph = loadGraphFromFile(graphFileName, startingGraphSize);
#ifdef DEBUG_1
    printf("Input graph:\n");
    PrintGraph(startingGraph, startingGraphSize);
#endif
    // Remove a node randomly
    srand(time(NULL));
    int node = rand() % startingGraphSize;
    int graphToMineSize = startingGraphSize-1;
    int *graphToMine = CopyGraph(startingGraph, GRAPH_SIZE, node);
    while (graphToMineSize != desiredOutputSize)
    {
      node = rand() % graphToMineSize;
      graphToMine = CopyGraph(graphToMine, graphToMineSize, node);
      graphToMineSize--;
    }
#ifdef DEBUG_1    
    printf("Smaller graph with size=%d:\n", graphToMineSize);
    PrintGraph(graphToMine, graphToMineSize);
#endif
    // Now check for CE
    int count = CliqueCount(graphToMine, graphToMineSize);
    // tabu search until we get a CE
   	/*
   	 * make a fifo to use as the taboo list
   	 */
   	taboo_list = FIFOInitEdge(TABOOSIZE);
   	if (taboo_list == NULL)
    {
   		exit(1);
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
    while (count != 0)
    {
   		best_count = BIGCOUNT;
   		for (i = 0; i < graphToMineSize; i++)
   		{
   			for (j = i+1; j < graphToMineSize; j++)
   			{
   				/*
   				 * flip it
   				 */
   				graphToMine[i*graphToMineSize+j] = 1 - graphToMine[i*graphToMineSize+j];
   				count = CliqueCount(graphToMine,graphToMineSize);

   				/*
   				 * is it better and the i,j,count not taboo?
   				 */
   				if ((count < best_count) && !FIFOFindEdgeCount(taboo_list,i,j,count))
             // !FIFOFindEdge(taboo_list,i,j))
   				{
   					best_count = count;
   					best_i = i;
   					best_j = j;
   				}

   				/*
   				 * flip it back
   				 */
   				graphToMine[i*graphToMineSize+j] = 1 - graphToMine[i*graphToMineSize+j];
   			}
   		}
      // Flip best edge and re-do count
  		if (best_count == BIGCOUNT)
      {
        // Do some random move here
  			printf("no best edge found, terminating\n");
  			exit(1);
  		}
  		/*
  		 * keep the best flip we saw
  		 */
  		graphToMine[best_i*graphToMineSize+best_j] = 1 - graphToMine[best_i*graphToMineSize+best_j];
      count = best_count;
  		printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
  			graphToMineSize,
  			best_count,
  			best_i,
  			best_j,
  			graphToMine[best_i*graphToMineSize+best_j]
      );
    }
    // HURRAY! Write to file
#ifdef DEBUG_1
    printf("Found CE:\n");
    PrintGraph(graphToMine, graphToMineSize);
#endif
    writeGraphToFile(graphToMine, graphToMineSize, desiredOutputFile);
    free(startingGraph);
    free(graphToMine);
  }
  else
  {
    // Master processor
    if (rank == 0)
    {
      printf("Found %d processors\n", size);
    }
  }
  MPI_Finalize();
  return 0;
}
