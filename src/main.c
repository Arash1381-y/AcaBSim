#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ERF_IMPLEMENTATION

#include "cflag.h"
#include "client.h"
#include "hashmap.h"
#include "queue.h"
#include "server.h"
#include "system.h"
#include "utils.h"

#define TEST_COUNT 1
#define SETUP1

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

// Setup 0: standard servers
sim_stat_t
setup0 (hashmap_t table, queue_t *clients, sim_param_t params)
{
  printf ("===========================\nRUNNING SETUP 0\n===========================\n");

  standard_server_t *server_1 = standard_server_new (1, table, params.error_per_act_prob);
  standard_server_t *server_2 = standard_server_new (2, table, params.error_per_act_prob);

  if (!server_1 || !server_2)
  {
    fprintf (stderr, "Error creating servers\n");
    exit (EXIT_FAILURE);
  }

  standard_server_t *servers[] = { server_1, server_2 };
  system_t system = { .system_clients = clients,
                      .standard_servers = servers,
                      .standard_servers_size = sizeof (servers) / sizeof (servers[0]),
                      .robotic_servers = NULL,
                      .robotic_servers_size = 0 };

  sim_stat_t stats;
  simulate (&system, params.simulation_time, &stats);

  log_server_stat (&server_1->base);
  log_server_stat (&server_2->base);

  return stats;
}

// Setup 1: standard and robotic servers
sim_stat_t
setup1 (hashmap_t table, queue_t *clients, sim_param_t params)
{
  printf ("===========================\nRUNNING SETUP 1\n===========================\n");

  standard_server_t *server_1 = standard_server_new (0, table, params.error_per_act_prob);
  robotic_server_t *server_2 = robotic_server_new (1, table, params.robot_boost_coff, params.error_per_act_prob);

  if (!server_1 || !server_2)
  {
    fprintf (stderr, "Error creating servers\n");
    exit (EXIT_FAILURE);
  }

  standard_server_t *servers[] = { server_1 };
  robotic_server_t *robotics[] = { server_2 };
  system_t system = { .system_clients = clients,
                      .standard_servers = servers,
                      .standard_servers_size = sizeof (servers) / sizeof (servers[0]),
                      .robotic_servers = robotics,
                      .robotic_servers_size = sizeof (robotics) / sizeof (robotics[0]) };

  sim_stat_t stats;
  simulate (&system, params.simulation_time, &stats);

  log_server_stat (&server_1->base);
  log_server_stat (&server_2->base);

  return stats;
}

// Function to update the total statistics
void
update_total_stats (sim_stat_t *total_stats, sim_stat_t *stats, int test_count)
{
  total_stats->total_clients += stats->total_clients;
  total_stats->mean_queue_size += stats->mean_queue_size;
  total_stats->mean_wait_time += stats->mean_wait_time;
  total_stats->mean_serve_time += stats->mean_serve_time;
  total_stats->nd_mean_wait_time += stats->nd_mean_wait_time;
  total_stats->nd_mean_serve_time += stats->nd_mean_serve_time;
  total_stats->t1_mean_wait_time += stats->t1_mean_wait_time;
  total_stats->t1_mean_serve_time += stats->t1_mean_serve_time;
  total_stats->t2_mean_wait_time += stats->t2_mean_wait_time;
  total_stats->t2_mean_serve_time += stats->t2_mean_serve_time;
  total_stats->t3_mean_wait_time += stats->t3_mean_wait_time;
  total_stats->t3_mean_serve_time += stats->t3_mean_serve_time;

  // Averaging after all tests
  if (test_count == TEST_COUNT)
  {
    total_stats->total_clients /= test_count;
    total_stats->mean_queue_size /= test_count;
    total_stats->mean_wait_time /= test_count;
    total_stats->mean_serve_time /= test_count;
    total_stats->nd_mean_wait_time /= test_count;
    total_stats->nd_mean_serve_time /= test_count;
    total_stats->t1_mean_wait_time /= test_count;
    total_stats->t1_mean_serve_time /= test_count;
    total_stats->t2_mean_wait_time /= test_count;
    total_stats->t2_mean_serve_time /= test_count;
    total_stats->t3_mean_wait_time /= test_count;
    total_stats->t3_mean_serve_time /= test_count;
  }
}

// Function to print average results
void
print_average_results (sim_stat_t *total_stats)
{
  printf ("\nAverage Results:\n");
  printf ("Mean client size: %d\n", total_stats->total_clients);
  printf ("Mean queue size: %d\n", total_stats->mean_queue_size);
  printf ("Mean wait time: %d\n", total_stats->mean_wait_time);
  printf ("Mean service time: %d\n", total_stats->mean_serve_time);
  printf ("ND mean wait time: %d\n", total_stats->nd_mean_wait_time);
  printf ("ND mean service time: %d\n", total_stats->nd_mean_serve_time);
  printf ("T1 mean wait time: %d\n", total_stats->t1_mean_wait_time);
  printf ("T1 mean service time: %d\n", total_stats->t1_mean_serve_time);
  printf ("T2 mean wait time: %d\n", total_stats->t2_mean_wait_time);
  printf ("T2 mean service time: %d\n", total_stats->t2_mean_serve_time);
  printf ("T3 mean wait time: %d\n", total_stats->t2_mean_wait_time);
  printf ("T3 mean service time: %d\n", total_stats->t2_mean_serve_time);
}

int
main (int argc, char *argv[])
{
  srand (time (NULL) * getpid ()); // Randomize seed

  sim_param_t params;
  initialize_parameters (&params, argc, argv);

  sim_stat_t total_stats = { 0 };
  int test_count = TEST_COUNT;

  for (int i = 0; i < test_count; i++)
  {
    hashmap_t table = NULL;
    init_table (&table);

    queue_t *clients = NULL;
    clients_init (&clients, params.disability_prob, params.p1_dest_prob, params.simulation_time);

    if (!table || !clients)
    {
      fprintf (stderr, "Error initializing table or clients\n");
      return EXIT_FAILURE;
    }

    sim_stat_t stats;
#ifdef SETUP0
    stats = setup0 (table, clients, params);
#else
    stats = setup1 (table, clients, params);
#endif

    update_total_stats (&total_stats, &stats, i + 1); // Update with stats from each test

    queue_free (clients);
    hashmap_free (table);
  }

  print_average_results (&total_stats); // Print the average results after all tests
}
