#pragma once

#include "queue.h"
#include "utils.h"

typedef enum
{
  NO_DISABILITY = 0,
  TYPE_1 = 2,
  TYPE_2 = 3,
  TYPE_3 = 4
} DISABILITY_TYPE;

typedef enum
{
  DEST_1 = 0,
  DEST_2 = 1
} DESTINATION;

typedef struct
{
  int arrival_time;
  int wait_time;
  int start_service_time;
  int finish_time;
  int failure_count;
  int total_time;

} client_service_stat_t;

typedef struct
{
  int id;
  client_service_stat_t service_stat;
  DISABILITY_TYPE disability_type;
  DESTINATION destination;
} client_t;

client_t *client_new ();

void clients_init (queue_t **clients, prob_t disability_prob, prob_t destination_prob, int finish_time);
void set_client_arrival_stat (client_t *client, int time);
void set_client_wait_stat (client_t *client, int time);
void set_client_start_service_stat (client_t *client, int time);
void set_client_finish_stat (client_t *client, int time);
void set_client_total_service_stat(client_t* client, int time);
void log_client_t (const client_t *client);