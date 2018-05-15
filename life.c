/* DESCRIPTION: Parallel code for simulating a cellular automaton running 
 *            Conway's Game of Life.
 * AUTHOR:    Aaron Weeden, Shodor Education Foundation, Inc.
 * DATE:      January 2012
 */

/***********************
 * Libraries to import *
 ***********************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

#define ALIVE 1
#define DEAD 0

/********************************************
 * Need at least this many rows and columns *
 ********************************************/
const int MINIMUM_ROWS = 1;
const int MINIMUM_COLUMNS = 1;
const int MINIMUM_TIME_STEPS = 1;

/*****************************************************
 * Add an "s" to the end of a value's name if needed *
 *****************************************************/
void pluralize_value_if_needed(int value)
{
  if(value != 1)
    fprintf(stderr, "s");

  return;
}

/*******************************************************************************
 * Make sure a value is >= another value, print error and return -1 if it isn't
 ******************************************************************************/
int assert_minimum_value(char which_value[16], int actual_value,
    int expected_value)
{
  int retval;

  if(actual_value < expected_value)
  {
    fprintf(stderr, "ERROR: %d %s", actual_value, which_value);
    pluralize_value_if_needed(actual_value);
    fprintf(stderr, "; need at least %d %s", expected_value, which_value);
    pluralize_value_if_needed(expected_value);
    fprintf(stderr, "\n");
    retval = -1;
  }
  else
    retval = 0;

  return retval;
}

/******************************************************************************
 * Print a function name and exit if the specified boolean expression is true *
 ******************************************************************************/
void exit_if(int boolean_expression, char function_name[32], int my_rank)
{
  if(boolean_expression)
  {
    fprintf(stderr, "Thread %d ", omp_get_thread_num());

    fprintf(stderr, "ERROR in %s\n", function_name);
    exit(-1);
  }

  return;
}

/****************
 * Main program *
 ****************/
int main(int argc, char **argv)
{
  int NUM_ROWS = 5, NUM_COLS = 5, NUM_STEPS = 5, 
      row, col, neighbor_row, neighbor_column,
      num_alive_neighbors, c, return_value; 
  int **current_grid, **next_grid;
  int step;

  /* Parse command line arguments */ 
  while((c = getopt(argc, argv, "r:c:t:")) != -1)
  {
    switch(c)
    {
      case 'r':
        NUM_ROWS = atoi(optarg);
        break;
      case 'c':
        NUM_COLS = atoi(optarg);
        break;
      case 't':
        NUM_STEPS = atoi(optarg);
        break;
      case '?':
      default:
        fprintf(stderr, "Usage: %s [-r NUM_ROWS] [-c NUM_COLS] [-t NUM_STEPS]\n", argv[0]);
        exit(-1);
    }
  }
  argc -= optind;
  argv += optind;

  /* Make sure we have enough rows, columns, and time steps */
  return_value = assert_minimum_value("row", NUM_ROWS, MINIMUM_ROWS);
  return_value += assert_minimum_value("column", NUM_COLS,
      MINIMUM_COLUMNS);
  return_value += assert_minimum_value("time step", NUM_STEPS,
      MINIMUM_TIME_STEPS);

  /* Exit if we don't */
  if(return_value != 0)
  {
    exit(-1);
  }

  /* Allocate enough space in the current grid and next grid for the
   *  number of rows and the number of columns, plus the ghost rows
   *  and columns */
  exit_if(((current_grid = (int**)malloc((NUM_ROWS + 2) 
            * (NUM_COLS + 2) * sizeof(int))) == NULL),
      "malloc(current_grid)", 0);
  exit_if(((next_grid = (int**)malloc((NUM_ROWS + 2) 
            * (NUM_COLS + 2) * sizeof(int))) == NULL),
      "malloc(next_grid)", 0);
  for(row = 0; row <= NUM_ROWS + 1; row++)
  {
    exit_if(((current_grid[row]
            = (int*)malloc((NUM_COLS + 2) * sizeof(int))) 
          == NULL), "malloc(current_grid[some_row])", 0);
    exit_if(((next_grid[row]
            = (int*)malloc((NUM_COLS + 2) * sizeof(int)))
          == NULL), "malloc(next_grid[some_row])", 0);
  }

  srandom(time(NULL));

  /* Initialize the grid (each cell gets a random state) */
  for(row = 1; row <= NUM_ROWS; row++)
  {
    for(col = 1; col <= NUM_COLS; col++)
    {
      current_grid[row][col] = random() % (ALIVE + 1);
    }
  }

  /* Run the simulation for the specified number of time steps */
  for(step = 0; step <= NUM_STEPS - 1; step++)
  {
    for(col = 0; col <= NUM_COLS + 1; col++)
    {
      /* Set the top row to be the same as the second-to-last row */
      current_grid[0][col] = current_grid[NUM_ROWS][col];

      /* Set the bottom row to be the same as the second-to-top row */
      current_grid[NUM_ROWS + 1][col] = current_grid[1][col];
    }

    /* Set up the ghost columns */
    for(row = 0; row <= NUM_ROWS + 1; row++)
    {
      /* The left ghost column is the same as the farthest-right, non-ghost
         column */
      current_grid[row][0] = current_grid[row][NUM_COLS];

      /* The right ghost column is the same as the farthest-left, non-ghost
         column */
      current_grid[row][NUM_COLS + 1] = current_grid[row][1];
    }

    /* Display the current grid */
    printf("Time Step %d:\n", step);
    for(row = 0; row <= NUM_ROWS + 1; row++)
    {
      if(row == 1)
      {
        for(col = 0; col <= NUM_COLS + 1 + 2; col++)
        {
          printf("- ");
        }
        printf("\n");
      }

      for(col = 0; col <= NUM_COLS + 1; col++)
      {
        if(col == 1)
        {
          printf("| ");
        }

        printf("%d ", current_grid[row][col]);

        if(col == NUM_COLS)
        {
          printf("| ");
        }
      }
      printf("\n");

      if(row == NUM_ROWS)
      {
        for(col = 0; col <= NUM_COLS + 1 + 2; col++)
        {
          printf("- ");
        }
        printf("\n");
      }
    }

    /* Determine the next grid -- for each row, do the following: */
    for(row = 1; row <= NUM_ROWS; row++)
    {
      /* For each column, do the following: */
      for(col = 1; col <= NUM_COLS; col++)
      {
        /* Initialize the count of ALIVE neighbors to 0 */
        num_alive_neighbors = 0;

        /* For each row of the cell's neighbors, do the following: */
        for(neighbor_row = row - 1; neighbor_row <= row + 1; neighbor_row++)
        {
          /* For each column of the cell's neighbors, do the following: */
          for(neighbor_column = col - 1; neighbor_column <= col + 1;
              neighbor_column++)
          {
            /* If the neighbor is not the cell itself, and the neighbor is
               ALIVE, do the following: */
            if((neighbor_row != row || neighbor_column != col) &&
               (current_grid[neighbor_row][neighbor_column] == ALIVE))
            {
              /* Add 1 to the count of the number of ALIVE neighbors */
              num_alive_neighbors++;
            }
          }
        }

        /* Apply Rule 1 of Conway's Game of Life */
        if(num_alive_neighbors < 2)
        {
          next_grid[row][col] = DEAD;
        }

        /* Apply Rule 2 of Conway's Game of Life */
        else if(current_grid[row][col] == ALIVE &&
           (num_alive_neighbors == 2 || num_alive_neighbors == 3))
        {
          next_grid[row][col] = ALIVE;
        }

        /* Apply Rule 3 of Conway's Game of Life */
        else if(num_alive_neighbors > 3)
        {
          next_grid[row][col] = DEAD;
        }

        /* Apply Rule 4 of Conway's Game of Life */
        else if(current_grid[row][col] == DEAD && num_alive_neighbors == 3)
        {
          next_grid[row][col] = ALIVE;
        }

        /* No rule applies; keep the same state */
        else
        {
          next_grid[row][col] = current_grid[row][col];
        }
      }
    }

    /* Copy the next grid into the current grid */
    for(row = 1; row <= NUM_ROWS; row++)
    {
      for(col = 1; col <= NUM_COLS; col++)
      {
        current_grid[row][col] = next_grid[row][col];
      }
    }
  }

  /* Deallocate data structures */
  for(row = NUM_ROWS + 1; row >= 0; row--)
  {
    free(next_grid[row]);
    free(current_grid[row]);
  }
  free(next_grid);
  free(current_grid);

  return 0;
}
