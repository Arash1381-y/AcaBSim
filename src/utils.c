#include <math.h>
#include <stdlib.h>

#include "cflag.h"
#include "stdio.h"
#include "utils.h"

#define RATE 0.5

int
exponential_sample ()
{
  double lambda = RATE; // rate parameter
  return (int)(-log (1.0 - (double)rand () / RAND_MAX) / lambda) + 1;
}

int
unifrom_sample (int a, int b)
{
  return a + (int)(((double)rand () / RAND_MAX) * (b - a + 1));
}

// Function to print simulation parameters
void
print_simulation_params (const sim_param_t *params)
{
  printf ("Simulation Parameters:\n");
  printf ("  Simulation time: %d\n", params->simulation_time);
  printf ("  Disability probability: %f\n", params->disability_prob);
  printf ("  P1 destination probability: %f\n", params->p1_dest_prob);
  printf ("  Error probability: %f\n", params->error_per_act_prob);
  printf ("  Robot boost coefficient: %f\n", params->robot_boost_coff);
}

// Initialize simulation parameters from command-line arguments
void
initialize_parameters (sim_param_t *params, int argc, char *argv[])
{
  const struct cflag options[]
      = { CFLAG (int, "simulation-time", 'T', &params->simulation_time, "Total simulation time of the system"),
          CFLAG (double, "disability-prob", 'X', &params->disability_prob, "Probability of client having disability"),
          CFLAG (double, "robot-boost", 'A', &params->robot_boost_coff, "Robot server boost over standard system"),
          CFLAG (double, "p1-prob", 'P', &params->p1_dest_prob, "Client P1 destination probability"),
          CFLAG (double, "error-prob", 'E', &params->error_per_act_prob, "Client P1 destination probability"),
          CFLAG_HELP,
          CFLAG_END };

  cflag_apply (options, "[options]", &argc, &argv);
  print_simulation_params (params);
}