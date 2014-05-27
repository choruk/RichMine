#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cilk/cilk.h>
#include "fifo.h"
#include "ce_constructor.h"

#define INITIAL_GRAPH_SIZE 8
#define SYMMETRIC_RAMSEY_NUMBER 6
#define TABOO_SIZE 500
#define BIG_COUNT 9999999

double getSeconds() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (double) (tp.tv_sec + ((1e-6)*tp.tv_usec));
}

void printUsageAndExit()
{
  fprintf(stderr, "Unexpected usage!\n");
  fprintf(stderr, "usage: ./ce_constructor [number of proccesors] [initial graph size]");
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
  fflush(stdout);

  for(i=0; i < gsize; i++)
  {
    for(j=0; j < gsize; j++)
    {
      fprintf(stdout,"%d ",g[i*gsize+j]);
      fflush(stdout);
    }
    fprintf(stdout,"\n");
    fflush(stdout);
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
void CopyGraph(int *oldGraph, int oldGraphSize, int *newGraph, int newGraphSize)
{
  int i;
  int j;
  /*
  * new g must be bigger or equal
  */
  if (newGraphSize < oldGraphSize)
    return;

  for (i=0; i < oldGraphSize; i++)
  {
    for (j=0; j < oldGraphSize; j++)
    {
      newGraph[i*newGraphSize+j] = oldGraph[i*oldGraphSize+j];
    }
  }
  // If we are just increasing size by 1...
  if (newGraphSize == oldGraphSize + 1)
  {
    // Lets balance out the edge colorings of this new node randomly
    int totalZeros = oldGraphSize / 2;
    int totalOnes = oldGraphSize - totalZeros;
    // Only care about upper triangle, which means we only need
    // to worry about the last column of entries and we dont
    // care about last row, so just use oldGraphSize for i.
    for (i = 0; i < oldGraphSize; i++)
    {
      j = newGraphSize-1;
      int color = random() % 2;
      // Keep it balanced
      if (totalZeros <= 0)
      {
        color = 1;
      }
      else if (totalOnes <= 0)
      {
        color = 0;
      }
      // Update counts
      if (color == 0)
      {
        totalZeros--;
      }
      else // color == 1
      {
        totalOnes--;
      }
      // Set color
      newGraph[i*newGraphSize+j] = color;
    }
  }
  return;
}

void CreateGraph(int *graph, int graphSize)
{
  int i,j;
  int matrixSize = graphSize * graphSize;
  // How many nodes in the upper diag region?
  int halfMatrix = matrixSize / 2;
  int halfMatrixRemainder = matrixSize % 2;
  int halfSize = graphSize / 2;
  int halfSizeRemainder = graphSize % 2;
  // (halfMatrixRemainder+halfSizeRemainder) guaranteed to be even
  int upperDiagonalSize = halfMatrix - halfSize + ((halfMatrixRemainder+halfSizeRemainder) / 2);
  int numZeros = upperDiagonalSize / 2;
  int numOnes = upperDiagonalSize - numZeros;
  for (i = 0; i < graphSize; i++)
  {
    for (j = i+1; j < graphSize; j++)
    {
      int color = random() % 2;
      // Keep it balanced
      if (numZeros <= 0)
      {
        color = 1;
      }
      else if (numOnes <= 0)
      {
        color = 0;
      }
      // Update counts
      if (color == 0)
      {
        numZeros--;
      }
      else // color == 1
      {
        numOnes--;
      }
      // Set the actual color
      graph[i*graphSize+j] = color;
    }
  }
}

void writeGraphToFile(int *graph, int graphSize, char *filename)
{
  FILE *graphFile = fopen(filename, "w");
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
  double executionStartTime = getSeconds();
  char *numProc = "4";
  int initialGraphSize = INITIAL_GRAPH_SIZE;
  if (argc > 1)
  {
    numProc = argv[1];
  }
  if (argc > 2)
  {
    initialGraphSize = atoi(argv[2]);
  }
  __cilkrts_set_param("nworkers", numProc);
  fprintf(stdout, "# of workers: %d\n", __cilkrts_get_nworkers());
  fflush(stdout);
  int p = atoi(numProc);
  int count, bestCount;
  int i, j, bestI, bestJ;
  int *newGraph;
  // Seed the PRNG
  struct timeval tm;
  gettimeofday(&tm, NULL);
  srandom(tm.tv_sec + tm.tv_usec * 1000000ul);
  /*
  * start with graph of size INITIAL_GRAPH_SIZE
  */
  int graphSize = initialGraphSize;
  int *graph = (int *)malloc(graphSize*graphSize*sizeof(int));
  if (graph == NULL) {
    exit(1);
  }
  // And evenly randomize the edge colorings
  CreateGraph(graph, graphSize);
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
  struct EdgeFlip *edgeFlipResults = (struct EdgeFlip *)malloc(p * sizeof(struct EdgeFlip));
  /*
  * make a fifo to use as the taboo list
  */
  void *tabooList = FIFOInitEdgeCount(TABOO_SIZE);
  if (tabooList == NULL) {
    exit(1);
  }
  /*
   * we will use this large random number to append to filenames as a
   * simple approach to avoiding filename collisions, but keeping all
   * files associated with a single run recognizable
  */
  double randomNum = random() % 1000000000;
  /*
  * while we do not have a publishable result
  */
  fprintf(stdout, "Initialization took %f seconds.\n", getSeconds() - executionStartTime);
  fflush(stdout);
  while(graphSize < 102)
  {
    double loopIterationStartTime = getSeconds();
    /*
    * find out how we are doing
    */
    count = CliqueCount(graph, graphSize);
    /*
    * if we have a counter example
    */
    if (count == 0)
    {
      fprintf(stdout, "Eureka!  Counter-example found!\n");
      fflush(stdout);
      PrintGraph(graph, graphSize);
      /* 
       * Use a separate file for the graphs that are actually counter-examples
      */
      char ceFileName[50];
      sprintf(ceFileName, "ce_constructor-solution-graph-%f.out", randomNum);
      writeGraphToFile(graph, graphSize, ceFileName);
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
      // Need to throw away old copies and make a new one for each processor
      for (i = 0; i < p; i++)
        free(graphs[i]);
      //      free(graphs);
      //      graphs = (int **)malloc(p * sizeof(int*));
      cilk_for (i = 0; i < p; i++)
      {
        graphs[i] = (int *)malloc(graphSize*graphSize*sizeof(int));
        CopyGraph(graph, graphSize, graphs[i], graphSize);
      }
      // Now we need to redivide up the rows of the matrix
      free(graphChunks);
      graphChunks = (struct GraphChunk *)malloc(p * sizeof(struct GraphChunk));
      // graphSize == # of rows
      chunkSize = graphSize / p;
      leftOver = graphSize % p;
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
    bestCount = BIG_COUNT;
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
      int myI, myJ;
      // only look at the rows that this processor is responsible for
      for (myI = graphChunks[k].offset; myI < graphChunks[k].offset+graphChunks[k].size; myI++)
      {
        for (myJ = myI+1; myJ < graphSize; myJ++)
        {
          /*
          * flip it
          */
          graphs[k][myI*graphSize+myJ] = 1 - graphs[k][myI*graphSize+myJ];
          edgeFlipResults[k].count = CliqueCount(graphs[k], graphSize);
          /*
          * is it better and the i,j,count not taboo?
          */
          if ((edgeFlipResults[k].count < edgeFlipResults[k].bestCount) &&
              !FIFOFindEdgeCount(tabooList, myI, myJ, edgeFlipResults[k].count))
            //					!FIFOFindEdge(taboo_list,i,j))
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
    // Did our move make an improvement?
    if (bestCount == BIG_COUNT) {
      fprintf(stderr, "no best edge found, terminating\n");
      fflush(stderr);
      exit(1);
    }
    /*
    * keep the best flip we saw
    */
    int newColor = 1 - graph[bestI*graphSize+bestJ];
    graph[bestI*graphSize+bestJ] = newColor;
    // We also need to flip this edge in all of the copied graphs
    cilk_for (i = 0; i < p; i++)
    {
      graphs[i][bestI*graphSize+bestJ] = newColor;
    }
    /*
    * taboo this graph configuration so that we don't visit
    * it again
    */
    count = CliqueCount(graph, graphSize);
    // FIFOInsertEdge(tabooList, bestI, bestJ);
    FIFOInsertEdgeCount(tabooList, bestI, bestJ, count);

    fprintf(stdout, "ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
      graphSize,
      bestCount,
      bestI,
      bestJ,
      graph[bestI*graphSize+bestJ]
    );
    fflush(stdout);
    /*
    * rinse and repeat
    */
    double loopIterationEndTime = getSeconds();
    fprintf(
      stdout,
      "That loop iteration took %f seconds.\nIn total, %f seconds have passed so far.\n",
      loopIterationEndTime-loopIterationStartTime,
      loopIterationEndTime-executionStartTime
    );
    fflush(stdout);
    /*
     * write the current graph to a file as well, as a checkpoint
     * we might use again later
    */
    char ckFileName[50];
    sprintf(ckFileName, "ce_constructor-checkpoint-graph-%f.out", randomNum);
    writeGraphToFile(graph, graphSize, ckFileName);
  }
  // Clean up
  FIFODeleteGraph(tabooList);
  // Free memory when we are done with it
  free(edgeFlipResults);
  free(graphChunks);
  for (i = 0; i < p; i++)
    free(graphs[i]);
  free(graphs);
  free(graph);
  return 0;
}