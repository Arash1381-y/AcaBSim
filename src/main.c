// -X 0.4 -T 100 -P 0.2 -A 0.4 -E 0.7
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

int
main (int argc, char *argv[])
{
  // srand (time (NULL)); // randomize seed

  sim_param_t params = {
    .simulation_time = 40, .disability_prob = 0, .robot_boost_coff = 10, .error_per_act_prob = 0, .p1_dest_prob = 0.3
  };

  // initialize_parameters (&params, argc, argv);

  hashmap_t table = NULL;
  init_table (&table);
  // service_t key = { .disability_type = NO_DISABILITY, .dest = DEST_1 };
  // const service_t *value = hashmap_get (table, &key);
  // if (value)
  // {
  //   printf ("The value is : %d\n", value->service_time);
  //   exit (EXIT_SUCCESS);
  // }

  queue_t *clients = NULL;
  clients_init (&clients, params.disability_prob, params.p1_dest_prob, params.simulation_time);
  printf ("THE NUMBER OF CLIENTS ARE : %d\n", queue_size (clients));
  if (table == NULL || clients == NULL)
  {
    fflush (stdout);
    return EXIT_FAILURE;
  }

  standard_server_t server_1 = {
    .base = { .service_time_table = table,
              .error_prob = params.error_per_act_prob,
              .current_client = NULL,
              .stat = { 0 },
              .finished_clients_queue = queue_init (QUEUE_BASE_SIZE) },
    .client_queue = queue_init (QUEUE_BASE_SIZE),
  };

  // standard_server_t server_2 = { .service_time_table = table,
  //                                .error_prob = params.error_per_act_prob,
  //                                .current_client = NULL,
  //                                .client_queue = queue_init (QUEUE_BASE_SIZE),
  //                                .finished_clients_queue = queue_init (QUEUE_BASE_SIZE),
  //                                .stat = { 0 } };

  // standard_server_t *servers[] = { &server_1, &server_2 };
  standard_server_t *servers[] = { &server_1 };
  system_t system = { .system_clients = clients,
                      .standard_servers = servers,
                      .standard_servers_size = sizeof (servers) / sizeof (servers[0]),
                      .robotic_servers = NULL,
                      .robotic_servers_size = 0 };

  sim_stat_t stats;
  simulate (&system, params.simulation_time, &stats);

  int count = queue_size (clients);
  log_server_stat (&server_1, 1);
  // log_server_stat (&server_2, 2);

  queue_free (clients);
  hashmap_free (table);

  return 0;
}
