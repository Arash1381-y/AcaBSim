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
void client_finalize (client_t *client, int time);
void client_set_arrival_time (client_t *client, int time);
void client_set_wait_time (client_t *client, int time);
void client_set_start_service_time (client_t *client, int time);
void client_set_finish_time (client_t *client, int time);
void client_set_total_service_time (client_t *client, int time);
void client_log (const client_t *client);