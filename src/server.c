#include <assert.h>
#include <stdio.h>

#include "erf_io.h"
#include "hashmap.h"
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
standard_server_new (int server_id, hashmap_t service_time_table, prob_t error_prob)
{
  standard_server_t *item = (standard_server_t *)malloc (sizeof (standard_server_t));
  if (!item)
    return NULL;

  item->base.id = server_id;
  item->base.type = STANDARD_SERVER;
  item->base.service_time_table = service_time_table;
  item->base.error_prob = error_prob;
  item->base.current_client = NULL;
  item->base.finished_clients_queue = queue_init (QUEUE_BASE_SIZE);

  item->base.stat = (server_stat_t){ 0 };

  item->client_queue = queue_init (QUEUE_BASE_SIZE);

  if (!item->client_queue || !item->base.finished_clients_queue)
    return NULL;

  return item;
}

robotic_server_t *
robotic_server_new (int server_id, hashmap_t service_time_table, double boost_rate, prob_t error_prob)
{
  robotic_server_t *item = (robotic_server_t *)malloc (sizeof (robotic_server_t));
  if (!item)
    return NULL;

  item->base.id = server_id;
  item->base.type = ROBOTIC_SERVER;
  item->base.service_time_table = service_time_table;
  item->base.error_prob = error_prob;
  item->base.current_client = NULL;
  item->base.finished_clients_queue = queue_init (QUEUE_BASE_SIZE);

  item->base.stat = (server_stat_t){ 0 };

  item->boost_rate = boost_rate;
  item->disable_client_queue = queue_init (QUEUE_BASE_SIZE);
  item->normal_client_queue = queue_init (QUEUE_BASE_SIZE);

  if (!item->disable_client_queue || !item->normal_client_queue || !item->base.finished_clients_queue)
    return NULL;

  return item;
}

static void
assign_client_to_standard_server (standard_server_t *server, client_t *client)
{
  erf_log (INFO, "CLIENT ID : %d enter STANDARD SERVER QUEUE %d", client->id, server->base.id);
  queue_push (server->client_queue, client);
}

static void
assign_client_to_robotic_server (robotic_server_t *server, client_t *client)
{
  if (client->disability_type == NO_DISABILITY)
  {
    erf_log (INFO, "CLIENT ID : %d enter ROBOTIC SERVER NORMAL QUEUE %d", client->id, server->base.id);
    queue_push (server->normal_client_queue, client);
  }
  else
  {
    erf_log (INFO, "CLIENT ID : %d enter ROBOTIC SERVER DISABLE QUEUE %d", client->id, server->base.id);
    queue_push (server->disable_client_queue, client);
  }
}

int
assign_client_to_server (server_t *server, client_t *client)
{
  assert (server != NULL);
  assert (client != NULL);

  // reinterpret server as base_server_t
  base_server_t *server_base = (base_server_t *)server;
  if (server_base->type == STANDARD_SERVER)
  {
    standard_server_t *sserver = (standard_server_t *)server;
    assign_client_to_standard_server (sserver, client);
  }
  else if (server_base->type == ROBOTIC_SERVER)
  {
    robotic_server_t *rserver = (robotic_server_t *)server;
    assign_client_to_robotic_server (rserver, client);
  }
  else
  {
    fprintf (stderr, "Invalid Server pointer");
    return 1;
  }

  return 0;
}

int
get_service_time (const base_server_t *server, client_t *client)
{
  assert (client);
  assert (server);
  assert (server->service_time_table);

  service_t key = { .disability_type = client->disability_type, .dest = client->destination };
  service_t *value = ((service_t *)hashmap_get (server->service_time_table, &key));
  if (value)
  {
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
log_server_stat (const base_server_t *server)
{
  if (!server)
  {
    fprintf (stderr, "Error: Server is NULL.\n");
    return;
  }

  base_server_t base = *server;
  server_stat_t stat = base.stat;

  // Print server stats
  printf ("==== Server ID: %d ====\n", base.id);
  printf ("Active Time: %d\n", stat.active_time);
  printf ("Idle Time: %d\n", stat.idle_time);
  printf ("Clients Served: %d\n", stat.client_served);
  printf ("Failure Count: %d\n", stat.failure_count);
  printf ("Mean Queue Length: %d\n", stat.mean_server_queue_len);

  // Print queue lengths
  printf ("Finished Queue Length: %zu\n", queue_size (base.finished_clients_queue));

  printf ("CURRENT CLINET: %p\n", &base.current_client);
  printf ("CURRENT TIME TO SERVE: %d\n", base.to_serve);

  // while (!queue_is_empty (base.finished_clients_queue))
  // {
  //   const client_t *client = queue_top (base.finished_clients_queue);
  //   log_client_t (client);
  //   queue_pop (base.finished_clients_queue);
  // }

  // Print error probability
  printf ("Error Probability: %.2f\n",
          (double)base.stat.failure_count / (base.stat.client_served + base.stat.failure_count));

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

static int
get_standard_server_load (standard_server_t *server)
{
  return queue_size (server->client_queue);
}

static int
get_robotic_server_load (robotic_server_t *server, DISABILITY_TYPE client_disability_type)
{
  if (client_disability_type == NO_DISABILITY)
  {
    return queue_size (server->normal_client_queue) + queue_size (server->disable_client_queue);
  }
  else
  {
    return queue_size (server->disable_client_queue);
  }
}

int
get_server_load (const server_t *server, DISABILITY_TYPE client_disability_type)
{
  base_server_t *server_base = (base_server_t *)server;
  if (server_base->type == STANDARD_SERVER)
  {
    return get_standard_server_load ((standard_server_t *)server);
  }
  else if (server_base->type == ROBOTIC_SERVER)
  {
    return get_robotic_server_load ((robotic_server_t *)server, client_disability_type);
  }
  else
  {
    // TODO: correct error handling
    exit (EXIT_FAILURE);
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
    erf_log (INFO, "SERVICE %d start serving CLIENT %d", server->base.id, next->id);
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
    erf_log (INFO, "SERVICE %d start serving CLIENT %d", server->base.id, next->id);
  }
  else
  {
    server->base.current_client = NULL;
    server->base.to_serve = 0;
    erf_log (INFO, "SERVICE %d no one to serve", server->base.id, server->base.id);
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
    erf_log (INFO, "SERVICE %d start serving CLIENT %d", server->base.id, next->id);
  }
  else
  {
    server->base.current_client = NULL;
    server->base.to_serve = 0;
    erf_log (INFO, "SERVICE %d no one to serve", server->base.id, server->base.id);
  }
}

void
server_set_next_client (server_t *server, int current_time)
{
  base_server_t *server_base = (base_server_t *)server;
  if (server_base->type == STANDARD_SERVER)
  {
    standard_server_set_next_client ((standard_server_t *)server, current_time);
  }
  else if (server_base->type == ROBOTIC_SERVER)
  {
    robotic_server_set_next_client ((robotic_server_t *)server, current_time);
  }
  else
  {
    // TODO: correct error handling
    exit (EXIT_FAILURE);
  }
}

static void
standard_server_update_stat (standard_server_t *server, int current_time)
{

  queue_t *queue = server->client_queue;
  size_t total_queues_size = queue_size (queue);
  server->base.stat.sum_queue_lens += total_queues_size;
  server->base.stat.mean_server_queue_len = server->base.stat.sum_queue_lens / (current_time + 1);
  assert (server->base.stat.mean_server_queue_len >= 0);
}

static void
robotic_server_update_stat (robotic_server_t *server, int current_time)
{
  queue_t *disable_queue = server->disable_client_queue;
  queue_t *normal_queue = server->normal_client_queue;
  size_t total_queues_size = queue_size (normal_queue) + queue_size (disable_queue);
  server->base.stat.sum_queue_lens += total_queues_size;
  server->base.stat.mean_server_queue_len = server->base.stat.sum_queue_lens / (current_time + 1);
}

void
server_update_stat (server_t *server, int current_time)
{
  base_server_t *server_base = (base_server_t *)server;
  if (server_base->type == STANDARD_SERVER)
  {
    standard_server_update_stat ((standard_server_t *)server, current_time);
  }
  else if (server_base->type == ROBOTIC_SERVER)
  {
    robotic_server_update_stat ((robotic_server_t *)server, current_time);
  }
  else
  {
    // TODO: correct error handling
    exit (EXIT_FAILURE);
  }
}

bool
server_is_active (server_t *server)
{
  base_server_t *server_base = (base_server_t *)server;
  return server_base->current_client != NULL;
}

bool
server_is_service_finished (server_t *server)
{
  base_server_t *server_base = (base_server_t *)server;
  return server_base->current_client != NULL && server_base->to_serve == 0;
}

bool
server_is_service_successful (server_t *server)
{
  base_server_t *server_base = (base_server_t *)server;
  return server_base->error_prob < (double)rand () / RAND_MAX;
}

client_t *
server_finalize_current_client (server_t *server)
{

  base_server_t *server_base = (base_server_t *)server;
  server_base->stat.client_served += 1;
  client_t *pre_clinet = server_base->current_client;
  queue_push (server_base->finished_clients_queue, pre_clinet);
  server_base->current_client = NULL;

  erf_log (INFO, "SERVER ID %d FINISHED SERVING CLIENT ID %d", server_base->id, pre_clinet->id);
  return pre_clinet;
}

void
server_retry (server_t *server)
{
  base_server_t *server_base = (base_server_t *)server;
  server_base->stat.failure_count += 1;

  client_t *client = server_base->current_client;
  client->service_stat.failure_count += 1;

  server_base->to_serve = get_service_time (server_base, client);
  erf_log (INFO, "SERVER ID %d REDO SERVING CLIENT ID %d", server_base->id, client->id);
}