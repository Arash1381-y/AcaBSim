#include "client.h"
#include "queue.h"
#include "utils.h"

#include <malloc.h>

client_t *
client_new ()
{
  client_t *client = (client_t *)malloc (sizeof (client_t));
  client->service_stat.failure_count = 0;
  client->service_stat.arrival_time = 0;
  client->service_stat.wait_time = 0;
  client->service_stat.start_service_time = 0;
  client->service_stat.finish_time = 0;
  return client;
}

void
clients_init (queue_t **clients, prob_t disability_prob, prob_t destination_prob, int finish_time)
{
  *clients = queue_init (QUEUE_BASE_SIZE);
  int id = 0;
  int next = 0;
  while (1)
  {
    next += exponential_sample ();
    if (next >= finish_time)
      break;

    client_t *client = client_new ();
    client->id = id;
    id++;
    client->service_stat.arrival_time = next;

    // check if has disability
    if ((double)rand () / RAND_MAX < disability_prob)
    {
      // determine disability type
      switch (unifrom_sample (0, sizeof (DISABILITY_TYPE) - 2))
      {
      case 0:
        client->disability_type = TYPE_1;
        break;
      case 1:
        client->disability_type = TYPE_2;
        break;
      case 2:
        client->disability_type = TYPE_3;
        break;

      default:
        break;
      }
    }
    else
    {
      client->disability_type = NO_DISABILITY;
    }

    // check if destination is p1
    if ((double)rand () / RAND_MAX < destination_prob)
    {
      client->destination = DEST_2;
    }
    else
    {
      client->destination = DEST_1;
    }
    queue_push (*clients, (void *)client);
  }
}

void
client_set_arrival (client_t *client, int time)
{
  client->service_stat.arrival_time = time;
}
void
client_set_wait_time (client_t *client, int time)
{
  client->service_stat.wait_time = time;
}

void
client_set_start_service_time (client_t *client, int time)
{
  client->service_stat.start_service_time = time;
}

void
client_set_finish_time (client_t *client, int time)
{
  client->service_stat.finish_time = time;
}

void
client_set_total_service_time (client_t *client, int time)
{
  client->service_stat.total_time = time;
}

void
log_client_t (const client_t *client)
{
  if (!client)
  {
    fprintf (stderr, "Error: Client is NULL.\n");
    return;
  }

  printf ("Client ID: %d\n", client->id);
  printf ("  Arrival Time: %d\n", client->service_stat.arrival_time);
  printf ("  Wait Time: %d\n", client->service_stat.wait_time);
  printf ("  Start Service Time: %d\n", client->service_stat.start_service_time);
  printf ("  Service Time: %d\n", client->service_stat.total_time);
  printf ("  Finish Time: %d\n", client->service_stat.finish_time);
  printf ("  Disability Type: %d\n", client->disability_type); // Enum as int
  printf ("  Destination: %d\n", client->destination);         // Enum as int
}

void
client_finalize (client_t *client, int time)
{
  client_set_finish_time (client, time);
  client_set_total_service_time (client, time - client->service_stat.arrival_time);
}