#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ERF_IMPLEMENTATION

#include "client.h"
#include "hashmap.h"
#include "queue.h"
#include "server.h"
#include "stat.h"
#include "system.h"
#include "utils.h"

#define TEST_COUNT 10000

// Function pointer for setup functions
sim_stat_t (*setup) (hashmap_t, queue_t *, sim_param_t);

// Setup 0: Standard servers
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

  server_free ((server_t *)server_1);
  server_free ((server_t *)server_2);
  return stats;
}

// Setup 1: Standard and robotic servers
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

  server_free ((server_t *)server_1);
  server_free ((server_t *)server_2);
  return stats;
}

// Function to initialize and run a single test
sim_stat_t
run_test (sim_param_t params, hashmap_t table)
{
  queue_t *clients = NULL;
  clients_init (&clients, params.disability_prob, params.p1_dest_prob, params.simulation_time);

  if (!table || !clients)
  {
    fprintf (stderr, "Error initializing table or clients\n");
    exit (EXIT_FAILURE);
  }

  sim_stat_t stats = setup (table, clients, params); // Run the selected setup
  queue_free (clients, free);
  return stats;
}

void
print_memory_usage ()
{
  FILE *file = fopen ("/proc/self/statm", "r");
  if (file)
  {
    int size, resident, shared, text, lib, data, dt;
    fscanf (file, "%d %d %d %d %d %d %d", &size, &resident, &shared, &text, &lib, &data, &dt);
    printf ("Memory usage: %d KB\n", resident * getpagesize () / 1024);
    fclose (file);
  }
}

int
main (int argc, char *argv[])
{
  srand (time (NULL) * getpid ()); // Randomize seed
  setup = setup0;                  // Choose setup function

  sim_param_t params;
  initialize_parameters (&params, argc, argv);

  hashmap_t table;
  init_table (&table);

  sim_stat_t total_stats = { 0 };

  for (int i = 0; i < TEST_COUNT; i++)
  {
    sim_stat_t stats = run_test (params, table); // Run the test and get stats
    update_total_stats (&total_stats, &stats);   // Update with stats from each test
  }

  finalize_stat (&total_stats, TEST_COUNT);
  print_stats (&total_stats); // Print the average results after all tests

  hashmap_free (table);

  return EXIT_SUCCESS;
}
