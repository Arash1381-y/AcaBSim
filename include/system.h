#pragma once

#include "erf_io.h"
#include "server.h"
#include "stat.h"

typedef struct
{
  queue_t *system_clients; // all system clients

  standard_server_t **standard_servers; // list of standard servers
  size_t standard_servers_size;         // number of standard servers

  robotic_server_t **robotic_servers; // list of robotic servers
  size_t robotic_servers_size;        // number of robotic servers
} system_t;

int simulate (system_t *system, const int simulation_time, sim_stat_t *stat);
