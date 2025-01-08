#pragma once

typedef struct
{
  int total_clients;
  int mean_queue_size;
  int mean_wait_time;
  int mean_serve_time;

  int nd_mean_wait_time;
  int nd_mean_serve_time;

  int t1_mean_wait_time;
  int t1_mean_serve_time;

  int t2_mean_wait_time;
  int t2_mean_serve_time;

  int t3_mean_wait_time;
  int t3_mean_serve_time;

} sim_stat_t;

void update_total_stats (sim_stat_t *total_stats, sim_stat_t *stats);
void finalize_stat (sim_stat_t *total_stats, int test_count);
void print_stats (sim_stat_t *total_stats);