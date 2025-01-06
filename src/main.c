#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "cflag.h"
#include "client.h"
#include "hashmap.h"
#include "queue.h"
#include "server.h"
#include "system.h"
#include "utils.h"

#define TEST_COUNT 10
// #define SETUP0

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

  printf ("the simulation time is : %d\n", params->simulation_time);
  printf ("the disability prob is : %f\n", params->disability_prob);
  printf ("the p1 dest    prob is : %f\n", params->p1_dest_prob);
  printf ("the error prob      is : %f\n", params->error_per_act_prob);
  printf ("the boost coff      is : %f\n", params->robot_boost_coff);
}

sim_stat_t
setup0 (hashmap_t table, queue_t *clients, sim_param_t params)
{
  standard_server_t server_1 = {
    .base = { .service_time_table = table,
              .error_prob = params.error_per_act_prob,
              .current_client = NULL,
              .stat = { 0 },
              .finished_clients_queue = queue_init (QUEUE_BASE_SIZE) },
    .client_queue = queue_init (QUEUE_BASE_SIZE),
  };

  standard_server_t server_2 = {
    .base = { .service_time_table = table,
              .error_prob = params.error_per_act_prob,
              .current_client = NULL,
              .stat = { 0 },
              .finished_clients_queue = queue_init (QUEUE_BASE_SIZE) },
    .client_queue = queue_init (QUEUE_BASE_SIZE),
  };

  standard_server_t *servers[] = { &server_1, &server_2 };
  system_t system = { .system_clients = clients,
                      .standard_servers = servers,
                      .standard_servers_size = sizeof (servers) / sizeof (servers[0]),
                      .robotic_servers = NULL,
                      .robotic_servers_size = 0 };

  sim_stat_t stats;
  simulate (&system, params.simulation_time, &stats);

  log_server_stat (&server_1.base, 1);
  log_server_stat (&server_2.base, 2);
  return stats;
}

sim_stat_t
setup1 (hashmap_t table, queue_t *clients, sim_param_t params)
{
  standard_server_t server_1 = {
    .base = { .service_time_table = table,
              .error_prob = params.error_per_act_prob,
              .current_client = NULL,
              .stat = { 0 },
              .finished_clients_queue = queue_init (QUEUE_BASE_SIZE) },
    .client_queue = queue_init (QUEUE_BASE_SIZE),
  };

  robotic_server_t server_2 = { .base = { .service_time_table = table,
                                          .error_prob = params.error_per_act_prob,
                                          .current_client = NULL,
                                          .stat = { 0 },
                                          .finished_clients_queue = queue_init (QUEUE_BASE_SIZE) },
                                .normal_client_queue = queue_init (QUEUE_BASE_SIZE),
                                .disable_client_queue = queue_init (QUEUE_BASE_SIZE),
                                .boost_rate = params.robot_boost_coff };

  standard_server_t *servers[] = { &server_1 };
  robotic_server_t *robotics[] = { &server_2 };
  system_t system = {
    .system_clients = clients,
    .standard_servers = servers,
    .standard_servers_size = sizeof (servers) / sizeof (servers[0]),
    .robotic_servers = robotics,
    .robotic_servers_size = sizeof (servers) / sizeof (servers[0]),
  };

  sim_stat_t stats;
  simulate (&system, params.simulation_time, &stats);
  log_server_stat (&server_1.base, 1);
  log_server_stat (&server_2.base, 2);
  return stats;
}

int
main (int argc, char *argv[])
{
  srand (time (NULL)); // randomize seed
  sim_param_t params;
  initialize_parameters (&params, argc, argv);

  sim_stat_t total_stats = { 0 };
  int test_count = 10;

  for (int i = 0; i < test_count; i++)
  {
    hashmap_t table = NULL;
    init_table (&table);

    queue_t *clients = NULL;
    clients_init (&clients, params.disability_prob, params.p1_dest_prob, params.simulation_time);
    if (table == NULL || clients == NULL)
    {
      fflush (stdout);
      return EXIT_FAILURE;
    }

#ifdef SETUP0
    sim_stat_t stats = setup0 (table, clients, params);
#else
    sim_stat_t stats = setup1 (table, clients, params);
#endif

    total_stats.mean_queue_size += stats.mean_queue_size;
    total_stats.mean_wait_time += stats.mean_wait_time;
    total_stats.mean_serve_time += stats.mean_serve_time;

    total_stats.nd_mean_wait_time += stats.nd_mean_wait_time;
    total_stats.nd_mean_serve_time += stats.nd_mean_serve_time;

    total_stats.t1_mean_wait_time += stats.t1_mean_wait_time;
    total_stats.t1_mean_serve_time += stats.t1_mean_serve_time;

    total_stats.t2_mean_wait_time += stats.t2_mean_wait_time;
    total_stats.t2_mean_serve_time += stats.t2_mean_serve_time;

    total_stats.t3_mean_wait_time += stats.t3_mean_wait_time;
    total_stats.t3_mean_serve_time += stats.t3_mean_serve_time;

    queue_free (clients);
    hashmap_free (table);
  }

  total_stats.mean_queue_size /= test_count;
  total_stats.mean_wait_time /= test_count;
  total_stats.mean_serve_time /= test_count;

  total_stats.nd_mean_wait_time /= test_count;
  total_stats.nd_mean_serve_time /= test_count;

  total_stats.t1_mean_wait_time /= test_count;
  total_stats.t1_mean_serve_time /= test_count;

  total_stats.t2_mean_wait_time /= test_count;
  total_stats.t2_mean_serve_time /= test_count;

  total_stats.t3_mean_wait_time /= test_count;
  total_stats.t3_mean_serve_time /= test_count;

  // Print the average results
  printf ("Average Results:\n");
  printf ("Mean queue size: %d\n", total_stats.mean_queue_size);
  printf ("Mean wait time: %d\n", total_stats.mean_wait_time);
  printf ("Mean service time: %d\n", total_stats.mean_serve_time);

  printf ("ND mean wait time: %d\n", total_stats.nd_mean_wait_time);
  printf ("ND mean service time: %d\n", total_stats.nd_mean_serve_time);

  printf ("T1 mean wait time: %d\n", total_stats.t1_mean_wait_time);
  printf ("T1 mean service time: %d\n", total_stats.t1_mean_serve_time);

  printf ("T2 mean wait time: %d\n", total_stats.t2_mean_serve_time);
  printf ("T2 mean service time: %d\n", total_stats.t2_mean_serve_time);
}
