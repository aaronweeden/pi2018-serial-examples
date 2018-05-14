/* Approximate pi using a Left Riemann Sum under a quarter unit circle. */

/* Author: Aaron Weeden, Shodor, 2015 */

#include <float.h>   /* DBL_EPSILON, DBL_DIG */
#include <math.h>    /* sqrt() */
#include <stdbool.h> /* bool type */
#include <stdio.h>   /* fprintf(), printf() */
#include <stdlib.h>  /* atoi(), atof(), exit(), EXIT_FAILURE */
#include <getopt.h>  /* getopt(), optarg */

/* Define default number of rectangles */
#define RECTS_PER_SIM_DEFAULT 10

/* Define description of input parameter for number of rectangles */
#define RECTS_PER_SIM_DESCR \
  "This many rectangles will be used (positive integer)"

/* Define character used on the command line to change the number of
   rectangles */
#define RECTS_PER_SIM_CHAR 'r'

/* Define options string used by getopt() - a colon after the character means
   the parameter's value is specified by the user */
char const GETOPT_STRING[] = {
  RECTS_PER_SIM_CHAR, ':'
};

int main(int argc, char **argv) {
  bool isError = false;

  /* Get user options for number of rectangles */
  unsigned long long int rectsPerSim = RECTS_PER_SIM_DEFAULT;
  char c;
  while ((c = getopt(argc, argv, GETOPT_STRING)) != -1) {
    switch(c) {
      /* The user has chosen to change the number of rectangles */
      case RECTS_PER_SIM_CHAR:
        /* Get integer value */
        rectsPerSim = strtoull(optarg, NULL, 10);
        /* Make sure positive and equal to floating point value */
        if (rectsPerSim < 1 || atof(optarg) != rectsPerSim) {
          fprintf(stderr, "ERROR: value for -%c must be positive integer\n",
              RECTS_PER_SIM_CHAR);
          isError = true;
        }
        break;
        /* The user has chosen an unknown option */
      default:
        isError = true;
    }
  }

  /* If there was an error in input, print a usage message and exit early */
  if (isError) {
    fprintf(stderr, "Usage: ");
    fprintf(stderr, "%s [OPTIONS]\n", argv[0]);
    fprintf(stderr, "Where OPTIONS can be any of the following:\n");
    fprintf(stderr, "-%c : \n\t%s\n\tdefault: %d\n", RECTS_PER_SIM_CHAR,
        RECTS_PER_SIM_DESCR, RECTS_PER_SIM_DEFAULT);
    exit(EXIT_FAILURE);
  }

  /* Calculate the width of each rectangle */
  double const width = (double)1 / rectsPerSim;

  /* Sum areas of all rectangles */
  double areaSum = 0.0;
  for (int i = 0; i < rectsPerSim; i++) {
    /* Calculate the x-coordinate of the rectangle's left side */
    double const x = i * width;

    /* Use the circle equation to calculate the rectangle's height squared */
    double const heightSq = 1.0 - x * x;

    /* If the height squared is so close to zero that the sqrt() function would
       return -inf, do not call the sqrt() function, just set the height to zero
       */
    double const height = (heightSq < DBL_EPSILON) ? 0.0 : sqrt(heightSq);

    /* Calculate the area of the rectangle and add it to the total */
    areaSum += width * height;
  }

  /* Calculate pi and print it */
  printf("%.*f\n", DBL_DIG, 4.0 * areaSum);
  printf("Value of pi from math.h is %.*f\n", DBL_DIG, M_PI);
  return 0;
}

