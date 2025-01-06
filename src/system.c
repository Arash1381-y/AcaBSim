#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "hashmap.h"
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
        client->service_stat.failure_count = 0;
        client->service_stat.finish_time = 0;
        client->service_stat.wait_time = 0;
        client->service_stat.total_time = 0;

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
assign_disable_client (system_t *system, client_t *client)
{
  if (system->robotic_servers_size == 0)
  {
    standard_server_t *low_load_standard_server = NULL;
    size_t min_queue_len = UINT32_MAX;

    // find the standard server with min queue size
    for (size_t i = 0; i < system->standard_servers_size; i++)
    {
      queue_t *queue = system->standard_servers[i]->client_queue;
      if (queue_size (queue) < min_queue_len)
      {
        min_queue_len = queue_size (queue);
        low_load_standard_server = system->standard_servers[i];
      }
    }

    assign_client_to_standard_server (low_load_standard_server, client);
  }
  else
  {
    robotic_server_t *low_load_robotic_server = NULL;
    size_t min_queue_len = UINT32_MAX;

    // find the standard server with min queue size
    for (size_t i = 0; i < system->robotic_servers_size; i++)
    {
      queue_t *queue = system->robotic_servers[i]->disable_client_queue;
      if (queue_size (queue) < min_queue_len)
      {
        min_queue_len = queue_size (queue);
        low_load_robotic_server = system->robotic_servers[i];
      }
    }

    assign_client_to_robotic_server (low_load_robotic_server, client);
  }

  return 0;
}

static int
assign_normal_client (system_t *system, client_t *client)
{
  standard_server_t *low_load_standard_server = NULL;
  robotic_server_t *low_load_robotic_server = NULL;
  size_t min_queue_len = UINT32_MAX;

  // find the standard server with min queue size
  for (size_t i = 0; i < system->standard_servers_size; i++)
  {
    queue_t *queue = system->standard_servers[i]->client_queue;
    if (queue_size (queue) < min_queue_len)
    {
      min_queue_len = queue_size (queue);
      low_load_standard_server = system->standard_servers[i];
    }
  }

  for (size_t i = 0; i < system->robotic_servers_size; i++)
  {
    queue_t *disable_queue = system->robotic_servers[i]->disable_client_queue;
    queue_t *normal_queue = system->robotic_servers[i]->normal_client_queue;
    size_t total_queues_size = queue_size (normal_queue) + queue_size (disable_queue);
    if (total_queues_size < min_queue_len)
    {
      min_queue_len = total_queues_size;
      low_load_robotic_server = system->robotic_servers[i];
    }
  }

  if (low_load_robotic_server)
  {
    assign_client_to_robotic_server (low_load_robotic_server, client);
  }
  else if (low_load_standard_server)
  {
    assign_client_to_standard_server (low_load_standard_server, client);
  }

  return 0;
}

static int
assign_client (system_t *system, client_t *client)
{
  // check if client is disable
  if (client->disability_type == NO_DISABILITY)
  {
    return assign_normal_client (system, client);
  }
  else
  {
    return assign_disable_client (system, client);
  }
}

static void
servers_update_serve_time (system_t *system)
{
  for (size_t i = 0; i < system->standard_servers_size; i++)
  {
    base_server_t *server = &system->standard_servers[i]->base;
    serve (server);
  }
  for (size_t i = 0; i < system->robotic_servers_size; i++)
  {
    base_server_t *server = &system->robotic_servers[i]->base;
    serve (server);
  }
}

static void
standard_server_set_next_client (standard_server_t *server, int current_time)
{

  queue_t *client_queue = server->client_queue;
  if (queue_size (client_queue))
  {
    // assign new clinet
    client_t *next = (client_t *)queue_top (client_queue);
    if (!queue_pop (client_queue))
    {
      exit (EXIT_FAILURE);
    }

    assert (next != NULL);
    server->base.to_serve = get_service_time (&server->base, next);
    server->base.current_client = next;

    // update client stat
    set_client_start_service_stat (next, current_time);
    set_client_wait_stat (next, current_time - next->service_stat.arrival_time);
  }
  else
  {
    server->base.current_client = NULL;
    server->base.to_serve = 0;
  }
}

static void
standard_servers_update_client (standard_server_t **servers, size_t servers_len, int current_time)
{
  // assign new client to standard servers
  for (size_t i = 0; i < servers_len; i++)
  {
    standard_server_t *server = servers[i];
    if (server->base.to_serve == 0 && server->base.current_client != NULL)
    {
      // no failure
      if (server->base.error_prob < (double)rand () / RAND_MAX)
      {

        // update client stats
        client_t *current_client = server->base.current_client;
        set_client_finish_stat (current_client, current_time);
        set_client_total_service_stat (current_client,
                                       current_time - server->base.current_client->service_stat.arrival_time);
        // update server stats
        server->base.stat.client_served += 1;
        queue_push (server->base.finished_clients_queue, server->base.current_client);

        server->base.current_client = NULL;
        standard_server_set_next_client (server, current_time);
      }
      // failure
      else
      {
        // update server stats
        server->base.stat.failure_count += 1;

        client_t *client = server->base.current_client;
        client->service_stat.failure_count += 1;

        server->base.to_serve = get_service_time (&server->base, client);
      }
    }
    else if (server->base.to_serve == 0)
    {
      standard_server_set_next_client (server, current_time);
    }

    queue_t *queue = server->client_queue;
    size_t total_queues_size = queue_size (queue);
    server->base.stat.sum_queue_lens += total_queues_size;
    server->base.stat.mean_server_queue_len = server->base.stat.sum_queue_lens / (current_time + 1);
  }
}

static void
robotic_server_set_next_client (robotic_server_t *server, int current_time)
{
  if (queue_size (server->disable_client_queue))
  {
    // assign new clinet
    client_t *next = (client_t *)queue_top (server->disable_client_queue);
    if (!queue_pop (server->disable_client_queue))
    {
      exit (EXIT_FAILURE);
    }

    assert (next != NULL);
    server->base.to_serve = get_service_time (&server->base, next) * server->boost_rate;
    assert (server->base.to_serve > 0);
    server->base.current_client = next;

    // update client stat
    set_client_start_service_stat (next, current_time);
    set_client_wait_stat (next, current_time - next->service_stat.arrival_time);
  }
  else if (queue_size (server->normal_client_queue))
  {
    client_t *next = (client_t *)queue_top (server->normal_client_queue);
    if (!queue_pop (server->normal_client_queue))
    {
      exit (EXIT_FAILURE);
    }

    assert (next != NULL);
    server->base.to_serve = get_service_time (&server->base, next);
    assert (server->base.to_serve > 0);
    server->base.current_client = next;

    // update client stat
    set_client_start_service_stat (next, current_time);
    set_client_wait_stat (next, current_time - next->service_stat.arrival_time);
  }
  else
  {
    server->base.current_client = NULL;
    server->base.to_serve = 0;
  }
}

static void
robotic_servers_update_client (robotic_server_t **servers, size_t servers_len, int current_time)
{
  for (size_t i = 0; i < servers_len; i++)
  {
    robotic_server_t *server = servers[i];
    if (server->base.to_serve == 0 && server->base.current_client != NULL)
    {
      // no failure
      if (server->base.error_prob < (double)rand () / RAND_MAX)
      {

        // update client stats
        set_client_finish_stat (server->base.current_client, current_time);
        set_client_total_service_stat (server->base.current_client,
                                       current_time - server->base.current_client->service_stat.arrival_time);

        // update server stats
        server->base.stat.client_served += 1;
        queue_push (server->base.finished_clients_queue, server->base.current_client);

        server->base.current_client = NULL;
        robotic_server_set_next_client (server, current_time);
      }
      // failure
      else
      {
        // update server stats
        server->base.stat.failure_count += 1;

        // update client stat
        client_t *client = server->base.current_client;
        client->service_stat.failure_count += 1;

        server->base.to_serve = get_service_time (&server->base, client);
        if (client->disability_type != NO_DISABILITY)
        {
          server->base.to_serve *= server->boost_rate;
        }
      }
    }
    else if (server->base.to_serve == 0)
    {
      server->base.current_client = NULL;
      robotic_server_set_next_client (server, current_time);
      assert ((server->base.to_serve > 0 && server->base.current_client != NULL)
              || (server->base.current_client == NULL && server->base.to_serve == 0));
    }

    // update server stat
    queue_t *disable_queue = server->disable_client_queue;
    queue_t *normal_queue = server->normal_client_queue;
    size_t total_queues_size = queue_size (normal_queue) + queue_size (disable_queue);
    server->base.stat.sum_queue_lens += total_queues_size;
    server->base.stat.mean_server_queue_len = server->base.stat.sum_queue_lens / (current_time + 1);
  }
}

static void
servers_run (system_t *system, int current_time)
{
  servers_update_serve_time (system);
  standard_servers_update_client (system->standard_servers, system->standard_servers_size, current_time);
  robotic_servers_update_client (system->robotic_servers, system->robotic_servers_size, current_time);
}

static void
compute_stats (system_t *system, sim_stat_t *stat)
{
  int total_queue_len = 0;

  int nd_total = 0;
  int nd_wait_time = 0;
  int nd_service_time = 0;

  int t1_total = 0;
  int t1_wait_time = 0;
  int t1_service_time = 0;

  int t2_total = 0;
  int t2_wait_time = 0;
  int t2_service_time = 0;

  int t3_total = 0;
  int t3_wait_time = 0;
  int t3_service_time = 0;

  int total_wait_time = 0;
  int total_serve_time = 0;
  int total_served_clients = 0;
  for (size_t i = 0; i < system->standard_servers_size; i++)
  {
    base_server_t server = system->standard_servers[i]->base;
    total_queue_len += server.stat.mean_server_queue_len;

    queue_t *finished = server.finished_clients_queue;
    queue_iterator_t *iterator = queue_iterator_create (finished);
    while (queue_iterator_has_next (iterator))
    {
      const client_t *client = (client_t *)queue_iterator_next (iterator);
      switch (client->disability_type)
      {
      case NO_DISABILITY:
        nd_total += 1;
        nd_wait_time += client->service_stat.wait_time;
        nd_service_time += client->service_stat.total_time;
        break;
      case TYPE_1:
        t1_total++;
        t1_wait_time += client->service_stat.wait_time;
        t1_service_time += client->service_stat.total_time;
        break;
      case TYPE_2:
        t2_total++;
        t2_wait_time += client->service_stat.wait_time;
        t2_service_time += client->service_stat.total_time;
        break;
      case TYPE_3:
        t3_total++;
        t3_wait_time += client->service_stat.wait_time;
        t3_service_time += client->service_stat.total_time;
        break;
      default:
        break;
      }

      total_wait_time += client->service_stat.wait_time;
      total_serve_time += client->service_stat.total_time;
      total_served_clients++;
    }
  }

  for (size_t i = 0; i < system->robotic_servers_size; i++)
  {
    base_server_t server = system->robotic_servers[i]->base;
    total_queue_len += server.stat.mean_server_queue_len;

    queue_t *finished = server.finished_clients_queue;
    queue_iterator_t *iterator = queue_iterator_create (finished);
    while (queue_iterator_has_next (iterator))
    {
      client_t *client = (client_t *)queue_iterator_next (iterator);
      switch (client->disability_type)
      {
      case NO_DISABILITY:
        nd_total += 1;
        nd_wait_time += client->service_stat.wait_time;
        nd_service_time += client->service_stat.total_time;
        break;
      case TYPE_1:
        t1_total++;
        t1_wait_time += client->service_stat.wait_time;
        t1_service_time += client->service_stat.total_time;
        break;
      case TYPE_2:
        t2_total++;
        t2_wait_time += client->service_stat.wait_time;
        t2_service_time += client->service_stat.total_time;
        break;
      case TYPE_3:
        t3_total++;
        t3_wait_time += client->service_stat.wait_time;
        t3_service_time += client->service_stat.total_time;
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

  for (int t = 0; t < simulation_time; t++)
  {
    system_tick (system, t, stat);
  }
  compute_stats (system, stat);

  return 0;
}
