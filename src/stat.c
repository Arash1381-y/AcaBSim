#include "stat.h"
#include "stdio.h"

// Function to update the total statistics
void
update_total_stats (sim_stat_t *total_stats, sim_stat_t *stats)
{
  total_stats->total_clients += stats->total_clients;
  total_stats->mean_queue_size += stats->mean_queue_size;
  total_stats->mean_wait_time += stats->mean_wait_time;
  total_stats->mean_serve_time += stats->mean_serve_time;
  total_stats->nd_mean_wait_time += stats->nd_mean_wait_time;
  total_stats->nd_mean_serve_time += stats->nd_mean_serve_time;
  total_stats->t1_mean_wait_time += stats->t1_mean_wait_time;
  total_stats->t1_mean_serve_time += stats->t1_mean_serve_time;
  total_stats->t2_mean_wait_time += stats->t2_mean_wait_time;
  total_stats->t2_mean_serve_time += stats->t2_mean_serve_time;
  total_stats->t3_mean_wait_time += stats->t3_mean_wait_time;
  total_stats->t3_mean_serve_time += stats->t3_mean_serve_time;
}

// Function to print average results
void
print_stats (sim_stat_t *total_stats)
{
  printf ("\nAverage Results:\n");
  printf ("Mean client size: %d\n", total_stats->total_clients);
  printf ("Mean queue size: %d\n", total_stats->mean_queue_size);
  printf ("Mean wait time: %d\n", total_stats->mean_wait_time);
  printf ("Mean service time: %d\n", total_stats->mean_serve_time);
  printf ("ND mean wait time: %d\n", total_stats->nd_mean_wait_time);
  printf ("ND mean service time: %d\n", total_stats->nd_mean_serve_time);
  printf ("T1 mean wait time: %d\n", total_stats->t1_mean_wait_time);
  printf ("T1 mean service time: %d\n", total_stats->t1_mean_serve_time);
  printf ("T2 mean wait time: %d\n", total_stats->t2_mean_wait_time);
  printf ("T2 mean service time: %d\n", total_stats->t2_mean_serve_time);
  printf ("T3 mean wait time: %d\n", total_stats->t3_mean_wait_time);
  printf ("T3 mean service time: %d\n", total_stats->t3_mean_serve_time);
}

void
finalize_stat (sim_stat_t *total_stats, int test_count)
{

  total_stats->total_clients /= test_count;
  total_stats->mean_queue_size /= test_count;
  total_stats->mean_wait_time /= test_count;
  total_stats->mean_serve_time /= test_count;
  total_stats->nd_mean_wait_time /= test_count;
  total_stats->nd_mean_serve_time /= test_count;
  total_stats->t1_mean_wait_time /= test_count;
  total_stats->t1_mean_serve_time /= test_count;
  total_stats->t2_mean_wait_time /= test_count;
  total_stats->t2_mean_serve_time /= test_count;
  total_stats->t3_mean_wait_time /= test_count;
  total_stats->t3_mean_serve_time /= test_count;
}