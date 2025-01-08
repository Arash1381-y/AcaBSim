#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "client.h"
#include "queue.h"
#include "utils.h"

typedef struct
{
  DISABILITY_TYPE disability_type; // for which type of client
  DESTINATION dest;                // which destination serve the clinet
  int service_time;                // amount of time it takes to serve the client
} service_t;

// compare
int service_compare (const void *a, const void *b, void *udata);

// iter
bool service_iter (const void *item, void *udata);

// hash
uint64_t service_hash (const void *item, uint64_t seed0, uint64_t seed1);

// init base server time table services
void init_table (hashmap_t *service_time_table);

typedef struct
{
  int active_time;
  int idle_time;
  int client_served;
  int failure_count;
  int sum_queue_lens;
  int mean_server_queue_len;
} server_stat_t;

typedef enum
{
  INVALID = 0,
  STANDARD_SERVER = 1,
  ROBOTIC_SERVER = 2
} SERVER_TYPE;

typedef struct
{
  SERVER_TYPE type;
  int id;
  hashmap_t service_time_table; // service time table

  client_t *current_client; // current serving client
  int to_serve;             // time to serve current client

  prob_t error_prob;               // server error prob
  server_stat_t stat;              // server_stat
  queue_t *finished_clients_queue; // finished tasks
} base_server_t;

typedef struct
{
  base_server_t base; // base server

  queue_t *client_queue; // clients waiting for the service
} standard_server_t;

typedef struct
{
  base_server_t base; // base server

  double boost_rate; // boost rate of the robotic server

  queue_t *disable_client_queue; // disable clients waiting for the service
  queue_t *normal_client_queue;  // normal clients waiting for the service
} robotic_server_t;

typedef void *server_t;

standard_server_t *standard_server_new (int server_id, hashmap_t service_time_table, prob_t error_prob);

robotic_server_t *robotic_server_new (int server_id, hashmap_t service_time_table, double boost_rate,
                                      prob_t error_prob);

int server_get_load (const server_t *server, DISABILITY_TYPE client_disability_type);

int server_assign_client (server_t *server, client_t *client);

void server_set_next_client (server_t *server, int current_time);

void server_update_stat (server_t *server, int current_time);

bool server_is_active (server_t *server);

bool server_is_service_finished (server_t *server);

bool server_is_service_successful (server_t *server);

void server_retry (server_t *server);

client_t *server_finalize_current_client (server_t *server);

void log_server_stat (const base_server_t *server);

void server_tick (base_server_t *server);

void server_free (server_t* server);