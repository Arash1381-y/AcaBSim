#include <assert.h>
#include <hashmap.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "queue.h"
#include "server.h"
#include "system.h"
#include "utils.h"

static client_t *
get_client (queue_t *clients, int current_time)
{
  client_t *client = (client_t *)queue_top (clients);
  if (client)
  {
    if (client->service_stat.arrival_time == current_time)
    {
      if (queue_pop (clients))
      {
        erf_log (INFO, "CLIENT ID : %d ENTER SYSTEM {DISABILITY TYPE: %d, DEST: %d}", client->id,
                 client->disability_type, client->destination);
        client->service_stat = (client_service_stat_t){ 0 };
        return client;
      }
      else
      {
        exit (EXIT_FAILURE);
      }
    }
  }

  return NULL;
}

static int
assign_client (system_t *system, client_t *client)
{
  standard_server_t *low_load_standard_server = NULL;
  int min_load_standard = INT32_MAX;
  for (size_t i = 0; i < system->standard_servers_size; i++)
  {
    standard_server_t *cserver = system->standard_servers[i];
    int load = server_get_load ((server_t *)cserver, client->disability_type);
    if (min_load_standard > load)
    {
      low_load_standard_server = cserver;
      min_load_standard = load;
    }
  }

  robotic_server_t *low_load_robotic_server = NULL;
  int min_load_robotic = INT32_MAX;

  for (size_t i = 0; i < system->robotic_servers_size; i++)
  {
    robotic_server_t *cserver = system->robotic_servers[i];
    int load = server_get_load ((server_t *)cserver, client->disability_type);
    if (min_load_robotic > load)
    {
      low_load_robotic_server = cserver;
      min_load_robotic = load;
    }
  }

  assert (low_load_robotic_server != NULL || low_load_standard_server != NULL);

  if (client->disability_type != NO_DISABILITY && low_load_robotic_server != NULL)
  {
    server_assign_client ((server_t)low_load_robotic_server, client);
  }
  else if (low_load_standard_server == NULL)
  {
    server_assign_client ((server_t)low_load_robotic_server, client);
  }
  else if (low_load_robotic_server == NULL)
  {
    server_assign_client ((server_t)low_load_standard_server, client);
  }
  else
  {
    if (min_load_robotic < min_load_standard)
    {
      server_assign_client ((server_t)low_load_robotic_server, client);
    }
    else
    {
      server_assign_client ((server_t)low_load_standard_server, client);
    }
  }

  return 0;
}

static void
servers_update_serve_time (system_t *system)
{
  for (size_t i = 0; i < system->standard_servers_size; i++)
  {
    base_server_t *server = &system->standard_servers[i]->base;
    server_tick (server);
  }
  for (size_t i = 0; i < system->robotic_servers_size; i++)
  {
    base_server_t *server = &system->robotic_servers[i]->base;
    server_tick (server);
  }
}

static void
update_clients (server_t **servers, size_t servers_len, int current_time)
{
  // assign new client to standard servers
  for (size_t i = 0; i < servers_len; i++)
  {
    server_t *server = servers[i];
    // check server status
    if (server_is_active (server))
    {
      // check service status
      if (server_is_service_finished (server))
      {
        // check if service was successful
        if (server_is_service_successful (server))
        {
          // update client status
          client_t *finished = server_finalize_current_client (server);
          client_finalize (finished, current_time);
          server_set_next_client ((server_t *)server, current_time);
        }
        else
        {
          server_retry ((server_t *)server);
        }
      }
    }
    else
    {
      server_set_next_client ((server_t *)server, current_time);
    }

    server_update_stat ((server_t *)server, current_time);
  }
}

static void
servers_run (system_t *system, int current_time)
{
  servers_update_serve_time (system);
  update_clients ((server_t **)system->standard_servers, system->standard_servers_size, current_time);
  update_clients ((server_t **)system->robotic_servers, system->robotic_servers_size, current_time);
}

static void
compute_client_stats (client_t *client, int *total, int *wait_time, int *service_time)
{
  (*total)++;
  *wait_time += client->service_stat.wait_time;
  *service_time += client->service_stat.total_time;
}

static void
compute_stats (system_t *system, sim_stat_t *stat)
{
  int total_queue_len = 0;
  int nd_total = 0, nd_wait_time = 0, nd_service_time = 0;
  int t1_total = 0, t1_wait_time = 0, t1_service_time = 0;
  int t2_total = 0, t2_wait_time = 0, t2_service_time = 0;
  int t3_total = 0, t3_wait_time = 0, t3_service_time = 0;
  int total_wait_time = 0, total_serve_time = 0, total_served_clients = 0;

  for (size_t i = 0; i < system->standard_servers_size + system->robotic_servers_size; i++)
  {
    base_server_t server = (i < system->standard_servers_size)
                               ? system->standard_servers[i]->base
                               : system->robotic_servers[i - system->standard_servers_size]->base;

    total_queue_len += server.stat.mean_server_queue_len;
    queue_t *finished = server.finished_clients_queue;
    queue_iterator_t *iterator = queue_iterator_create (finished);

    while (queue_iterator_has_next (iterator))
    {
      client_t *client = (client_t *)queue_iterator_next (iterator);
      switch (client->disability_type)
      {
      case NO_DISABILITY:
        compute_client_stats (client, &nd_total, &nd_wait_time, &nd_service_time);
        break;
      case TYPE_1:
        compute_client_stats (client, &t1_total, &t1_wait_time, &t1_service_time);
        break;
      case TYPE_2:
        compute_client_stats (client, &t2_total, &t2_wait_time, &t2_service_time);
        break;
      case TYPE_3:
        compute_client_stats (client, &t3_total, &t3_wait_time, &t3_service_time);
        break;
      default:
        break;
      }
      total_wait_time += client->service_stat.wait_time;
      total_serve_time += client->service_stat.total_time;
      total_served_clients++;
    }
    queue_iterator_destroy (iterator);
  }

  stat->nd_mean_wait_time = nd_total > 0 ? nd_wait_time / nd_total : 0;
  stat->nd_mean_serve_time = nd_total > 0 ? nd_service_time / nd_total : 0;
  stat->t1_mean_wait_time = t1_total > 0 ? t1_wait_time / t1_total : 0;
  stat->t1_mean_serve_time = t1_total > 0 ? t1_service_time / t1_total : 0;
  stat->t2_mean_wait_time = t2_total > 0 ? t2_wait_time / t2_total : 0;
  stat->t2_mean_serve_time = t2_total > 0 ? t2_service_time / t2_total : 0;
  stat->t3_mean_wait_time = t3_total > 0 ? t3_wait_time / t3_total : 0;
  stat->t3_mean_serve_time = t3_total > 0 ? t3_service_time / t3_total : 0;

  stat->mean_queue_size = (system->standard_servers_size + system->robotic_servers_size) > 0
                              ? total_queue_len / (system->standard_servers_size + system->robotic_servers_size)
                              : 0;
  stat->mean_serve_time = total_served_clients > 0 ? total_serve_time / total_served_clients : 0;
  stat->mean_wait_time = total_served_clients > 0 ? total_wait_time / total_served_clients : 0;
}

static int
system_tick (system_t *system, int current_time, sim_stat_t *stat)
{
  // check if there is any new client and assign it
  client_t *new_client = get_client (system->system_clients, current_time);
  if (new_client)
  {
    assign_client (system, new_client);
  }

  servers_run (system, current_time);

  return 0;
}

int
simulate (system_t *system, int simulation_time, sim_stat_t *stat)
{
  service_t key = { .disability_type = NO_DISABILITY, .dest = DEST_1 };
  hashmap_get (system->standard_servers[0]->base.service_time_table, &key);

  if (system == NULL || system->system_clients == NULL
      || (system->robotic_servers_size + system->standard_servers_size == 0))
  {
    fprintf (stderr, "Error: system is not initialized correctly");
    exit (EXIT_FAILURE);
  }

  if (stat == NULL)
  {
    fprintf (stderr, "Error: stat is undefined");
    exit (EXIT_FAILURE);
  }

  stat->total_clients = queue_size (system->system_clients);

  for (int t = 0; t < simulation_time; t++)
  {
    erf_log (INFO, "EVENT AT TIME : %d", t);
    system_tick (system, t, stat);
  }
  compute_stats (system, stat);

  return 0;
}
