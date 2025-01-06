#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "client.h"
#include "hashmap.h"
#include "queue.h"
#include "utils.h"

typedef struct
{
  DISABILITY_TYPE disability_type; // for which type of client
  DESTINATION dest;                // which destination serve the clinet
  int service_time;                // amount of time it takes to serve the client
} service_t;

typedef struct
{
  int active_time;
  int idle_time;
  int client_served;
  int failure_count;
  int sum_queue_lens;
  int mean_server_queue_len;
} server_stat_t;

typedef struct
{
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

  hashmap_t service_time_table; // service time table

  prob_t error_prob;        // server error prob
  client_t *current_client; // current client

  int to_serve;     // time to serve current client
  float boost_rate; // boost rate of the robotic server

  queue_t *disable_client_queue;   // disable clients waiting for the service
  queue_t *normal_client_queue;    // normal clients waiting for the service
  queue_t *finished_clients_queue; // finished tasks

  server_stat_t stat; // server_stat
} robotic_server_t;

int service_compare (const void *a, const void *b, void *udata);

bool service_iter (const void *item, void *udata);

uint64_t service_hash (const void *item, uint64_t seed0, uint64_t seed1);

void init_table (hashmap_t *service_time_table);

standard_server_t *standard_server_new (hashmap_t service_time_table, prob_t error_prob);

int get_robotic_service_time (robotic_server_t *server, client_t *client);

get_service_time (base_server_t *server, client_t *client);

void assign_client_to_standard_server (standard_server_t *server, client_t *client);

void assign_client_to_robotic_server (robotic_server_t *server, client_t *client);

// void update_standard_server_active_stat (standard_server_t *server);
// void update_robotic_server_active_stat (standard_server_t *server);

// void update_standard_server_idle_stat (standard_server_t *server);
// void update_robotic_server_idle_stat (standard_server_t *server);

// system_t system_init (standard_server_t **standard_servers, robotic_server_t **robotic_servers);
void log_server_stat (const standard_server_t *server, int server_id);