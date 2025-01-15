# Introduction

This repository contains code related to computer simulation project. It simulates a queue system using different strategies to enhance the service ratio in an academic environment. 

# Code Structure

The simulation model consists of 2 main modules:

1. `clients`
2. `servers`

`servers` provide `service`s to clients. in the next sections we will take a look at the general structure of these modules.

## Client

The client module is modeled by `client_t` struct in `client.c/h` file.

```C
typedef struct
{
  int id;
  client_service_stat_t service_stat;
  DISABILITY_TYPE disability_type;
  DESTINATION destination;
} client_t;
```

where first argument specifies the status of the client in the system and second and third argument specify the type of disability and client destination.

`service_stat` is defined as below

```C
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
```

`service_stat` will be updated during the simulation. it tracks the status of the client in the process of simulating.

## Servers

There are 2 types of servers in this application.

1. standard
2. robotics

to optimally define these two servers, a `base_server` class is provided to capture the similar member of these 2 servers. This struct is defined as below:

```C
typedef struct
{
  hashmap_t service_time_table; // service time table

  client_t *current_client; // current serving client
  int to_serve;             // time to serve current client

  prob_t error_prob;               // server error prob
  server_stat_t stat;              // server_stat
  queue_t *finished_clients_queue; // finished tasks
} base_server_t;
```

the robotic and standard server are defined based on this:

```C
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
```

the robotic_server has two queue. this modeling is based on the fact that robotic server prioritize the clients with disabilities. Using 2 separate queue capture the prioritization process perfectly.

## System

System is a collection of servers and clients. it is defined as below:

```C
typedef struct
{
  queue_t *system_clients;

  standard_server_t **standard_servers;
  size_t standard_servers_size;

  robotic_server_t **robotic_servers;
  size_t robotic_servers_size;
} system_t;
```

each server may have multiple standard or robotic servers. although the simulation setup is pretty simple, this modeling help us in capturing more complex scenarios.

# Setup

This project has 2 main setups (`setup0` & `setup1`) where `setup0` consists of 2 standard server and `setup1` consists of 1 standard and on robotic server.
