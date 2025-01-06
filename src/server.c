#include <assert.h>
#include <stdio.h>

#include "server.h"

#define INITIAL_TABLE_SIZE 8

void
init_table (hashmap_t *service_time_table)
{

  *service_time_table
      = hashmap_new (sizeof (service_t), INITIAL_TABLE_SIZE, 0, 0, service_hash, service_compare, NULL, NULL);
  hashmap_t table = *service_time_table;
  hashmap_set (table, &(service_t){ .disability_type = NO_DISABILITY, .dest = DEST_1, .service_time = 4 });
  hashmap_set (table, &(service_t){ .disability_type = NO_DISABILITY, .dest = DEST_2, .service_time = 6 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_1, .dest = DEST_1, .service_time = 8 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_1, .dest = DEST_2, .service_time = 10 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_2, .dest = DEST_1, .service_time = 10 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_2, .dest = DEST_2, .service_time = 14 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_3, .dest = DEST_1, .service_time = 18 });
  hashmap_set (table, &(service_t){ .disability_type = TYPE_3, .dest = DEST_2, .service_time = 16 });
}

int
service_compare (const void *a, const void *b, void *udata)
{
  const service_t *sa = a;
  const service_t *sb = b;
  return sa->disability_type != sb->disability_type || sa->dest != sb->dest;
}

bool
service_iter (const void *item, void *udata)
{
  const service_t *service = item;
  printf ("SERVICE FOR DISABILITY TYPE : %d WITH DEST : %d", service->disability_type, service->dest);
  return true;
}

uint64_t
service_hash (const void *item, uint64_t seed0, uint64_t seed1)
{
  const service_t *service = item;
  return hashmap_sip (&service->disability_type, sizeof (service->disability_type), seed0, seed1)
         ^ hashmap_sip (&service->dest, sizeof (service->dest), seed0, seed1);
}

standard_server_t *
standard_server_new (hashmap_t service_time_table, prob_t error_prob)
{
  standard_server_t *item = (standard_server_t *)malloc (sizeof (standard_server_t));
  item->base.service_time_table = service_time_table;
  item->base.error_prob = error_prob;
  item->base.current_client = NULL;

  item->client_queue = queue_init (128);
  return item;
}

void
assign_client_to_standard_server (standard_server_t *server, client_t *client)
{
  queue_push (server->client_queue, client);
}

void
assign_client_to_robotic_server (robotic_server_t *server, client_t *client)
{
  if (client->disability_type == NO_DISABILITY)
  {
    queue_push (server->normal_client_queue, client);
  }
  else
  {
    queue_push (server->disable_client_queue, client);
  }
}

int
get_service_time (const base_server_t *server, client_t *client)
{
  assert (client);
  assert (server);

  service_t key = { .disability_type = client->disability_type, .dest = client->destination };
  service_t *value = ((service_t *)hashmap_get (server->service_time_table, &key));
  if (value)
  {
    assert (value->service_time);
    return value->service_time;
  }
  else
  {
    printf ("INVALID HASHING\n");
    fflush (stdout);
    exit (EXIT_FAILURE);
  }
}

void
log_server_stat (const base_server_t *server, int server_id)
{
  if (!server)
  {
    fprintf (stderr, "Error: Server is NULL.\n");
    return;
  }

  base_server_t base = *server;
  server_stat_t stat = base.stat;

  // Print server stats
  printf ("==== Server ID: %d ====\n", server_id);
  printf ("Active Time: %d\n", stat.active_time);
  printf ("Idle Time: %d\n", stat.idle_time);
  printf ("Clients Served: %d\n", stat.client_served);
  printf ("Failure Count: %d\n", stat.failure_count);
  printf ("Mean Queue Length: %d\n", stat.mean_server_queue_len);

  // Print queue lengths
  printf ("Finished Queue Length: %zu\n", queue_size (base.finished_clients_queue));

  // while (!queue_is_empty (base.finished_clients_queue))
  // {
  //   const client_t *client = queue_top (base.finished_clients_queue);
  //   log_client_t (client);
  //   queue_pop (base.finished_clients_queue);
  // }

  // Print error probability
  printf ("Error Probability: %.2f\n", (double)base.stat.failure_count / (base.stat.client_served + base.stat.failure_count));

  printf ("========================\n");
}

void
serve (base_server_t *server)
{
  // serve current task for standard servers
  if (server->current_client != NULL)
  {
    // update server stat
    assert (server->to_serve > 0);
    server->stat.active_time += 1;

    server->to_serve--;
  }
  else
  {
    // update server stat
    server->stat.idle_time += 1;
  }
}