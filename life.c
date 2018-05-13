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
void exit_if(int boolean_expression, char function_name[32], int OUR_RANK)
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
  int NUMBER_OF_ROWS = 5, NUMBER_OF_COLUMNS = 5, NUMBER_OF_TIME_STEPS = 5, 
      current_row, current_column, neighbor_row, neighbor_column,
      number_of_alive_neighbors, c, return_value; 
  int **current_grid, **next_grid;
  int current_time_step;

  /* Parse command line arguments */ 
  while((c = getopt(argc, argv, "r:c:t:")) != -1)
  {
    switch(c)
    {
      case 'r':
        NUMBER_OF_ROWS = atoi(optarg);
        break;
      case 'c':
        NUMBER_OF_COLUMNS = atoi(optarg);
        break;
      case 't':
        NUMBER_OF_TIME_STEPS = atoi(optarg);
        break;
      case '?':
      default:
        fprintf(stderr, "Usage: %s [-r NUMBER_OF_ROWS] [-c NUMBER_OF_COLUMNS] [-t NUMBER_OF_TIME_STEPS]\n", argv[0]);
        exit(-1);
    }
  }
  argc -= optind;
  argv += optind;

  /* Make sure we have enough rows, columns, and time steps */
  return_value = assert_minimum_value("row", NUMBER_OF_ROWS, MINIMUM_ROWS);
  return_value += assert_minimum_value("column", NUMBER_OF_COLUMNS,
      MINIMUM_COLUMNS);
  return_value += assert_minimum_value("time step", NUMBER_OF_TIME_STEPS,
      MINIMUM_TIME_STEPS);

  /* Exit if we don't */
  if(return_value != 0)
    exit(-1);

  /* Allocate enough space in the current grid and next grid for the
   *  number of rows and the number of columns, plus the ghost rows
   *  and columns */
  exit_if(((current_grid = (int**)malloc((NUMBER_OF_ROWS + 2) 
            * (NUMBER_OF_COLUMNS + 2) * sizeof(int))) == NULL),
      "malloc(current_grid)", 0);
  exit_if(((next_grid = (int**)malloc((NUMBER_OF_ROWS + 2) 
            * (NUMBER_OF_COLUMNS + 2) * sizeof(int))) == NULL),
      "malloc(next_grid)", 0);
  for(current_row = 0; current_row <= NUMBER_OF_ROWS + 1;
      current_row++)
  {
    exit_if(((current_grid[current_row]
            = (int*)malloc((NUMBER_OF_COLUMNS + 2) * sizeof(int))) 
          == NULL), "malloc(current_grid[some_row])", 0);
    exit_if(((next_grid[current_row]
            = (int*)malloc((NUMBER_OF_COLUMNS + 2) * sizeof(int)))
          == NULL), "malloc(next_grid[some_row])", 0);
  }

  srandom(time(NULL));

  /* Initialize the grid (each cell gets a random state) */
  for(current_row = 1; current_row <= NUMBER_OF_ROWS;
      current_row++)
  {
    for(current_column = 1; current_column <= NUMBER_OF_COLUMNS;
        current_column++)
    {
      current_grid[current_row][current_column] =
        random() % (ALIVE + 1);
    }
  }

  /* Determine the process with the next-lowest rank */

  /* Determine the process with the next-highest rank */

  /* Run the simulation for the specified number of time steps */
  for(current_time_step = 0; current_time_step <= NUMBER_OF_TIME_STEPS - 1;
      current_time_step++)
  {
    for(current_column = 0;
        current_column <= NUMBER_OF_COLUMNS + 1;
        current_column++)
    {
      /* Set the top row to be the same as the second-to-last row */
      current_grid[0][current_column]
        = current_grid[NUMBER_OF_ROWS][current_column];

      /* Set the bottom row to be the same as the second-to-top row */
      current_grid[NUMBER_OF_ROWS + 1][current_column]
        = current_grid[1][current_column];
    }

    /* Set up the ghost columns */
    for(current_row = 0; current_row <= NUMBER_OF_ROWS + 1;
        current_row++)
    {
      /* The left ghost column is the same as the farthest-right, non-ghost
         column */
      current_grid[current_row][0] =
        current_grid[current_row][NUMBER_OF_COLUMNS];

      /* The right ghost column is the same as the farthest-left, non-ghost
         column */
      current_grid[current_row][NUMBER_OF_COLUMNS + 1] =
        current_grid[current_row][1];
    }

    /* Display the current grid */
    printf("Time Step %d:\n", current_time_step);
    for(current_row = 0; current_row <= NUMBER_OF_ROWS + 1;
        current_row++)
    {
      if(current_row == 1)
      {
        for(current_column = 0;
            current_column <= NUMBER_OF_COLUMNS + 1 + 2;
            current_column++)
        {
          printf("- ");
        }
        printf("\n");
      }

      for(current_column = 0;
          current_column <= NUMBER_OF_COLUMNS + 1; 
          current_column++)
      {
        if(current_column == 1)
        {
          printf("| ");
        }

        printf("%d ", current_grid[current_row]
            [current_column]);

        if(current_column == NUMBER_OF_COLUMNS)
        {
          printf("| ");
        }
      }
      printf("\n");

      if(current_row == NUMBER_OF_ROWS)
      {
        for(current_column = 0;
            current_column <= NUMBER_OF_COLUMNS + 1 + 2;
            current_column++)
        {
          printf("- ");
        }
        printf("\n");
      }
    }

    /* Determine the next grid -- for each row, do the following: */
    for(current_row = 1; current_row <= NUMBER_OF_ROWS;
        current_row++)
    {
      /* For each column, do the following: */
      for(current_column = 1; current_column <= NUMBER_OF_COLUMNS;
          current_column++)
      {
        /* Initialize the count of ALIVE neighbors to 0 */
        number_of_alive_neighbors = 0;

        /* For each row of the cell's neighbors, do the following: */
        for(neighbor_row = current_row - 1;
            neighbor_row <= current_row + 1;
            neighbor_row++)
        {
          /* For each column of the cell's neighbors, do the following: */
          for(neighbor_column = current_column - 1;
              neighbor_column <= current_column + 1;
              neighbor_column++)
          {
            /* If the neighbor is not the cell itself, and the neighbor is
               ALIVE, do the following: */
            if((neighbor_row != current_row
                  || neighbor_column != current_column)
                && (current_grid[neighbor_row]
                  [neighbor_column] == ALIVE))
            {
              /* Add 1 to the count of the number of ALIVE neighbors */
              number_of_alive_neighbors++;
            }
          }
        }

        /* Apply Rule 1 of Conway's Game of Life */
        if(number_of_alive_neighbors < 2)
        {
          next_grid[current_row][current_column] = DEAD;
        }

        /* Apply Rule 2 of Conway's Game of Life */
        if(current_grid[current_row][current_column] == ALIVE
            && (number_of_alive_neighbors == 2
              || number_of_alive_neighbors == 3))
        {
          next_grid[current_row][current_column] = ALIVE;
        }

        /* Apply Rule 3 of Conway's Game of Life */
        if(number_of_alive_neighbors > 3)
        {
          next_grid[current_row][current_column] = DEAD;
        }

        /* Apply Rule 4 of Conway's Game of Life */
        if(current_grid[current_row][current_column] == DEAD
            && number_of_alive_neighbors == 3)
        {
          next_grid[current_row][current_column] = ALIVE;
        }
      }
    }

    /* Copy the next grid into the current grid */
    for(current_row = 1; current_row <= NUMBER_OF_ROWS;
        current_row++)
    {
      for(current_column = 1; current_column <= NUMBER_OF_COLUMNS;
          current_column++)
      {
        current_grid[current_row][current_column] =
          next_grid[current_row][current_column];
      }
    }
  }

  /* Deallocate data structures */
  for(current_row = NUMBER_OF_ROWS + 1; current_row >= 0;
      current_row--)
  {
    free(next_grid[current_row]);
    free(current_grid[current_row]);
  }
  free(next_grid);
  free(current_grid);

  return 0;
}
