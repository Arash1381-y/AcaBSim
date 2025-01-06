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

typedef struct
{

} sim_stat_t;

int exponential_sample ();

int unifrom_sample (int a, int b);