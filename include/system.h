#pragma once

#include "server.h"
#include <utils.h>

typedef struct
{
  queue_t *system_clients;

  standard_server_t **standard_servers;
  size_t standard_servers_size;

  robotic_server_t **robotic_servers;
  size_t robotic_servers_size;
} system_t;


int simulate (system_t *system, const int simulation_time, sim_stat_t *stat);