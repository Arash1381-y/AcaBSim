#pragma once

typedef double prob_t;
typedef struct hashmap *hashmap_t;

typedef struct
{
  int simulation_time;
  prob_t disability_prob;
  double robot_boost_coff;
  prob_t error_per_act_prob;
  prob_t p1_dest_prob;
} sim_param_t;

int exponential_sample ();

int unifrom_sample (int a, int b);

void print_simulation_params (const sim_param_t *params);

void initialize_parameters (sim_param_t *params, int argc, char *argv[]);