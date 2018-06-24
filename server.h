typedef struct
{
  int fd_client;
  int fd_slave;
  int task;
}msg;

typedef struct
{
  unsigned int id;
  msg content;
}data;

typedef struct
{
  unsigned int id;
  struct pollfd fd_server_slave;
}data_slave;

typedef struct
{
  unsigned int id;
  struct pollfd fd_server_client;
}data_client;


