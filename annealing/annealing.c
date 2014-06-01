#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "fifo.h"	/* for taboo list */

#define MAXSIZE (512)
#define TABOOSIZE (500)
#define BIGCOUNT (9999999)

/*
 * PrintGraph
 *
 * prints in the right format for the read routine
 */
void PrintGraph(int *g, int graphSize)
{
	int i;
	int j;

	fprintf(stdout,"%d\n",graphSize);

	for(i=0; i < graphSize; i++)
	{
		for(j=0; j < graphSize; j++)
		{
			fprintf(stdout,"%d ",g[i*graphSize+j]);
		}
		fprintf(stdout,"\n");
	}

	return;
}

int CliqueCount(int *g, int graphSize)
{
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int count=0;
    int sgraphSize = 6;
    
    for(i=0;i < graphSize-sgraphSize+1; i++)
    {
	for(j=i+1;j < graphSize-sgraphSize+2; j++)
        {
	    for(k=j+1;k < graphSize-sgraphSize+3; k++) 
            { 
		if((g[i*graphSize+j] == g[i*graphSize+k]) && 
		   (g[i*graphSize+j] == g[j*graphSize+k]))
		{
		    for(l=k+1;l < graphSize-sgraphSize+4; l++) 
		    { 
			if((g[i*graphSize+j] == g[i*graphSize+l]) && 
			   (g[i*graphSize+j] == g[j*graphSize+l]) && 
			   (g[i*graphSize+j] == g[k*graphSize+l]))
			{
			    for(m=l+1;m < graphSize-sgraphSize+5; m++) 
			    {
				if((g[i*graphSize+j] == g[i*graphSize+m]) && 
				   (g[i*graphSize+j] == g[j*graphSize+m]) &&
				   (g[i*graphSize+j] == g[k*graphSize+m]) && 
				   (g[i*graphSize+j] == g[l*graphSize+m])) {
					for(n=m+1; n < graphSize-sgraphSize+6; n++)
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

void convertGraphToString(int *graph, int graphSize, char *graphString)
{
  int i;
  for (i = 0; i < graphSize*graphSize; i++)
    {
      if (graph[i] == 0)
	{
	  graphString[i] = '0';
	}
      else if (graph[i] == 1)
	{
	  graphString[i] = '1';
	}
    }
  graphString[graphSize*graphSize] = '\0';

#ifdef DEBUG_2
  fprintf(stdout, "Converted graph to string: %s\n", graphString);
  fflush(stdout);
#endif
}

int main(int argc,char *argv[])
{
	int *g;
	int graphSize;
	int count;
	int i;
	int j;
	int r;
	int last_count;
	int best_count;
	int best_i;
	int best_j;
	void *taboo_list;
	FILE *ifp;
	FILE *ofp;

	double startTime = getSeconds();
	double currentTime;
	long elapsed, sElapsed, mElapsed, hElapsed;

	// Read in system_best.txt file
	ifp = fopen("system_best.txt", "r");
	if(ifp == NULL) {
	  fprintf(stdout, "Error: Can't open system_best.txt\n");
	  exit(1);
	} else {
	  fprintf(stdout, "Opening system_best.txt - there is a better graph available\n");
	  
	  // Read the system_best.txt graph size
	  char buf[32];
	  fgets(buf, sizeof buf, ifp);
	  graphSize = atoi(buf);
	  
	  // Read the system_best.txt clique count
	  fgets(buf, sizeof buf, ifp);
	  count = atoi(buf);
	  
	  // Read the system_best.txt graph
	  g = (int *)malloc(graphSize*graphSize*sizeof(int));
	  if(g == NULL) {
	    exit(1);
	  }
	  char* gc = (char*) malloc(graphSize*graphSize*sizeof(char));
	  if(fgets(gc, graphSize*graphSize+1, ifp) == NULL)
	    printf("ERROR\n");
	  int x;
	  for(x = 0; x < graphSize*graphSize; x++) {
	    g[x] = gc[x] - '0';
	  }
	  
	  // Close the system_best.txt file and clean up
	  fclose(ifp);
	  free(gc);
	  
	  // Delete file
	  if(remove("system_best.txt") != 0)
	    printf("Error: system_best.txt could not be removed\n");
	}
	
	// Variables to be used later
	char gs[graphSize*graphSize+1];

	/*
	 * make a fifo to use as the taboo list
	 */
	taboo_list = FIFOInitEdge(TABOOSIZE);
	if(taboo_list == NULL) {
		exit(1);
	}


	/*
	 * while we do not have a publishable result
	 */
	while(1)
	{
		// Read in system_best.txt file
		ifp = fopen("system_best.txt", "r");
		if(ifp == NULL) {
		  fprintf(stdout, "Can't open system_best.txt - we are up to date\n");
		} else {
		  fprintf(stdout, "Opening system_best.txt - there is a better graph available\n");
	
		  // Read the system_best.txt graph size
		  char buf[32];
		  fgets(buf, sizeof buf, ifp);
		  graphSize = atoi(buf);
	
		  // Read the system_best.txt clique count
		  fgets(buf, sizeof buf, ifp);
		  count = atoi(buf);
	
		  // Read the system_best.txt graph
		  char* gc = (char*) malloc(graphSize*graphSize*sizeof(char));
		  if(fgets(gc, graphSize*graphSize+1, ifp) == NULL)
		    printf("ERROR\n");
		  int x;
		  for(x = 0; x < graphSize*graphSize; x++) {
		    g[x] = gc[x] - '0';
		  }
	
		  // Close the system_best.txt file and clean up
		  fclose(ifp);
	
		  // Verify
		  free(gc);

		  // Delete file
		  if(remove("system_best.txt") != 0)
		    printf("Error: system_best.txt could not be removed\n");
		}

		/*
		 * if we have a counter example
		 */
		if(count == 0)
		{
			printf("Eureka!  Counter-example found!\n");
			fflush(stdout);
			PrintGraph(g,graphSize);
			break;
		}
		
		// Flip 5 random edges
		for(r = 0; r < 5; r++)
		  {
		    i = rand() % graphSize;
		    j = rand() % graphSize;
		    if(j >= i)
		      g[i*graphSize+j] = 1 - g[i*graphSize+j];
		    else
		      g[j*graphSize+i] = 1 - g[j*graphSize+i];
		  }
		count = CliqueCount(g,graphSize);
		printf("Progress has slowed, flipping 5 random edges. New count is %d\n", count);
		fflush(stdout);

		// Perform greedy search until progress slows
		last_count = 9999;
		while((last_count - count) > 10)
		  {
		    last_count = count;
		    best_count = BIGCOUNT;
		    for(i=0; i < graphSize; i++)
		      {
			for(j=i+1; j < graphSize; j++)
			  {
			    /*
			     * flip it
			     */
			    g[i*graphSize+j] = 1 - g[i*graphSize+j];
			    count = CliqueCount(g,graphSize);

			    /*
			     * is it better and the i,j,count not taboo?
			     */
			    if((count < best_count) && 
			       !FIFOFindEdge(taboo_list,i,j))
			      //					!FIFOFindEdgeCount(taboo_list,i,j,count))
			      {
				best_count = count;
				best_i = i;
				best_j = j;
			      }

			    /*
			     * flip it back
			     */
			    g[i*graphSize+j] = 1 - g[i*graphSize+j];
			  }
		      }

		    if(best_count == BIGCOUNT) {
		      printf("no best edge found, terminating\n");
		      exit(1);
		    }
		
		    /*
		     * keep the best flip we saw
		     */
		    g[best_i*graphSize+best_j] = 1 - g[best_i*graphSize+best_j];

		    /*
		     * taboo this graph configuration so that we don't visit
		     * it again
		     */
		    count = CliqueCount(g,graphSize);
		    //FIFOInsertEdge(taboo_list,best_i,best_j);
		    FIFOInsertEdgeCount(taboo_list,best_i,best_j,count);

		    // Write current solution to file
		    ofp = fopen("local_best.txt", "w");
		    if(ofp == NULL)
		      printf("Error: could not open local_best.txt, terminating\n");
		    fprintf(ofp, "%d\n", graphSize);
		    fflush(ofp);
		    fprintf(ofp, "%d\n", count);
		    fflush(ofp);
		    convertGraphToString(g, graphSize, gs);
		    fprintf(ofp, "%s", gs);
		    fflush(ofp);
		    fclose(ofp);

		    // Calculate timing
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
			   best_count,
			   best_i,
			   best_j,
			   g[best_i*graphSize+best_j]);
		    fflush(stdout);
		  }

		/*
		 * rinse and repeat
		 */
	}

	FIFODeleteGraph(taboo_list);
	free(g);

	return(0);
}
