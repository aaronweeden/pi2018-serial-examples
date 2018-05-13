/* Parallelization: Infectious Disease
 * By Aaron Weeden, Shodor Education Foundation, Inc.
 * November 2011
 */

#include <assert.h> /* for assert */
#include <stdio.h> /* printf */
#include <stdlib.h> /* malloc, free, and various others */
#include <time.h> /* time is used to seed the random number generator */
#include <unistd.h> /* random, getopt, some others */

/* States of people -- all people are one of these 4 states */
/* These are const char because they are displayed as ASCII */
const char INFECTED = 'X';
const char IMMUNE = 'I';
const char SUSCEPTIBLE = 'o';
const char DEAD = ' ';

/* PROGRAM EXECUTION BEGINS HERE */
int main(int argc, char** argv)
{
  /** Declare variables **/
  /* People */
  int num_infected = 0;
  int new_num_infected = 0;
  int num_people = 50;
  int person1 = 0;
  int current_infected_person = 0;
  int num_init_infected = 1;
  int x = 0;
  int y = 0;
  int num_susceptible = 0;
  int num_immune = 0;
  int num_dead = 0;
  int infected_nearby = 0;
  int person2 = 0;

  /* Environment */
  int env_width = 30;
  int env_height = 30;

  /* Disease */
  int infection_radius = 1;
  int disease_duration = 50;
  int contagiousness_factor = 30;
  int deadliness_factor = 30;
  double num_infections = 0.0;
  double infection_attempts = 0.0;
  double num_deaths = 0.0;
  double recovery_attempts = 0.0;

  /* Time */
  int num_days = 250;
  int current_day = 0;
  int microseconds_per_day = 100000;

  /* Movement */
  int x_dir = 0; 
  int y_dir = 0;

  /* getopt */
  int c = 0;

  /* Integer arrays, a.k.a. integer pointers */
  int *xs;
  int *ys;
  int *infected_xs;
  int *infected_ys;
  int *days_infected;

  /* Character arrays, a.k.a. character pointers */
  char *states;

  /* Array of character arrays, a.k.a. array of character pointers, for text
   *  display */
  char **environment;

  /* Loop control */
  int i;

  /* Get command line options -- this follows the idiom presented in the
   *  getopt man page (enter 'man 3 getopt' on the shell for more) */
  while((c = getopt(argc, argv, "n:i:w:h:t:T:c:d:D:m:")) != -1)
  {
    switch(c)
    {
      case 'n':
        num_people = atoi(optarg);
        break;
      case 'i':
        num_init_infected = atoi(optarg);
        break;
      case 'w':
        env_width = atoi(optarg);
        break;
      case 'h':
        env_height = atoi(optarg);
        break;
      case 't':
        num_days = atoi(optarg);
        break;
      case 'T':
        disease_duration = atoi(optarg);
        break;
      case 'c':
        contagiousness_factor = atoi(optarg);
        break;
      case 'd':
        infection_radius = atoi(optarg);
        break;
      case 'D':
        deadliness_factor = atoi(optarg);
        break;
      case 'm':
        microseconds_per_day = atoi(optarg);
        break;
        /* If the user entered "-?" or an unrecognized option, we need 
         *  to print a usage message before exiting. */
      case '?':
      default:
        fprintf(stderr, "Usage: ");
        fprintf(stderr, "%s [-n num_people][-i num_init_infected][-w env_width][-h env_height][-t num_days][-T disease_duration][-c contagiousness_factor][-d infection_radius][-D deadliness_factor][-m microseconds_per_day]\n", argv[0]);
        exit(-1);
    }
  }
  argc -= optind;
  argv += optind;

  /* Make sure that the total number of initially infected people is less than
   * the total number of people */
  if(num_init_infected > num_people)
  {
    fprintf(stderr, "ERROR: initial number of infected (%d) must be less than total number of people (%d)\n", num_init_infected, num_people);
    exit(-1);
  }

  /* Allocate the arrays */
  xs = (int*)malloc(num_people * sizeof(int));
  ys = (int*)malloc(num_people * sizeof(int));
  infected_xs = (int*)malloc(num_people * sizeof(int));
  infected_ys = (int*)malloc(num_people * sizeof(int));
  days_infected = (int*)malloc(num_people * sizeof(int));
  states = (char*)malloc(num_people * sizeof(char));
  environment = (char**)malloc(env_height * env_width * sizeof(char*));
  for(y = 0; y < env_height; y++)
  {
    environment[y] = (char*)malloc(env_width * sizeof(char));
  }

  /* Seed the random number generator based on the current time */
  srandom(time(NULL));

  /* Set the states of the initially infected people and set
   * the count of infected people */
  for(i = 0; i < num_init_infected; i++)
  {
    states[i] = INFECTED;
    num_infected++;
  }

  /* Set the states of the rest of the people and set the
   * count of susceptible people */
  for(i = num_init_infected; i < num_people; i++)
  {
    states[i] = SUSCEPTIBLE;
    num_susceptible++;
  }

  /* Set random x and y locations for each person */
  for(i = 0; i < num_people; i++)
  {
    xs[i] = random() % env_width;
    ys[i] = random() % env_height;
  }

  /* Initialize the number of days infected of each person
   * to 0 */
  for(i = 0; i < num_people; i++)
  {
    days_infected[i] = 0;
  }

  /* Start a loop to run the simulation for the specified number of days */
  for(current_day = 0; current_day < num_days; current_day++)
  {
    /* Determine infected x locations and infected y locations */
    current_infected_person = 0;
    for(person1 = 0; person1 < num_people; person1++)
    {
      if(states[person1] == INFECTED)
      {
        infected_xs[current_infected_person] = xs[person1];
        infected_ys[current_infected_person] = ys[person1];
        current_infected_person++;
      }
    }

    /* Displays a graphic of the current day */
    for(y = 0; y < env_height; y++)
    {
      for(x = 0; x < env_width; x++)
      {
        environment[y][x] = ' ';
      }
    }

    for(i = 0; i < num_people; i++)
    {
      environment[ys[i]][xs[i]] = states[i];
    }

    printf("----------------------\n");
    for(y = 0; y < env_height; y++)
    {
      for(x = 0; x < env_width; x++)
      {
        printf("%c", environment[y][x]);
      }
      printf("\n");
    }

    /* For each person, do the following */
    for(i = 0; i < num_people; i++)
    {
      /* If the person is not dead, then */
      if(states[i] != DEAD)
      {
        /* Randomly pick whether the person moves left or right or does not move
         * in the x dimension */
        x_dir = (random() % 3) - 1;

        /* Randomly pick whether the person moves up or down or does not move
         * in the y dimension */
        y_dir = (random() % 3) - 1;

        /* If the person will remain in the bounds of the environment after 
         * moving, then */
        if((xs[i] + x_dir >= 0) &&
           (xs[i] + x_dir < env_width) &&
           (ys[i] + y_dir >= 0) &&
           (ys[i] + y_dir < env_height))
        {
          /* Move the person */
          xs[i] += x_dir;
          ys[i] += y_dir;
        }
      }
    }

    new_num_infected = num_infected;
    /* For each person, do the following */
    for(person1 = 0; person1 < num_people; person1++)
    {
      /* If the person is susceptible, then */
      if(states[person1] == SUSCEPTIBLE)
      {
        /* For each of the infected people or until the number of infected 
         *  people nearby is 1, the thread does the following */
        infected_nearby = 0;
        for(person2 = 0; person2 < num_infected && infected_nearby < 1;
            person2++)
        {
          /* If person 1 is within the infection radius, then */
          if((xs[person1] >= infected_xs[person2] - infection_radius) &&
             (xs[person1] <= infected_xs[person2] + infection_radius) &&
             (ys[person1] >= infected_ys[person2] - infection_radius) &&
             (ys[person1] <= infected_ys[person2] + infection_radius))
          {
            /* Increment the number of infected people nearby */
            infected_nearby++;
          }
        }

        if(infected_nearby >= 1)
        {
          infection_attempts++;
        }

        /* If there is at least one infected person nearby, and a random number
         *  less than 100 is less than or equal to the contagiousness factor,
         *  then */
        if(infected_nearby >= 1 && (random() % 100) <= contagiousness_factor)
        {
          /* Change person1's state to infected */
          states[person1] = INFECTED;

          /* Update the counters */
          new_num_infected++;
          num_susceptible--;
          num_infections++;
        }
      }
    }
    num_infected = new_num_infected;

    /* For each person, do the following */
    for(i = 0; i < num_people; i++)
    {
      /* If the person is infected and has been for the full duration of the
       *  disease, then */
      if(states[i] == INFECTED && days_infected[i] == disease_duration)
      {
        recovery_attempts++;

        /* If a random number less than 100 is less than the deadliness
         *  factor, then */
        if((random() % 100) < deadliness_factor)
        {
          /* Change the person's state to dead */
          states[i] = DEAD;

          /* Update the counters */
          num_dead++;
          num_infected--;
          num_deaths++;
        }
        /* Otherwise, */
        else
        {
          /* Change the person's state to immune */
          states[i] = IMMUNE;

          /* Update the counters */
          num_immune++;
          num_infected--;
        }
      }
    }

    /* For each person, do the following */
    for(i = 0; i < num_people; i++)
    {
      /* If the person is infected, then */
      if(states[i] == INFECTED)
      {
        /* Increment the number of days the person has been infected */
        days_infected[i]++;
      }
    }
  }

  printf("Final counts: %d susceptible, %d infected, %d immune, \
      %d dead\nActual contagiousness: %f\nActual deadliness: \
      %f\n", num_susceptible, num_infected, num_immune, 
      num_dead, 100.0 * (num_infections / 
        (infection_attempts == 0 ? 1 : infection_attempts)),
      100.0 * (num_deaths / (recovery_attempts == 0 ? 1 
          : recovery_attempts)));

  /* Deallocate the arrays -- we have finished using the memory, so now we
   *  "free" it back to the heap */
  for(y = env_height - 1; y >= 0; y--)
  {
    free(environment[y]);
  }
  free(environment);
  free(states);
  free(days_infected);
  free(infected_ys);
  free(infected_xs);
  free(ys);
  free(xs);

  /* The program has finished executing successfully */
  return 0;
}
