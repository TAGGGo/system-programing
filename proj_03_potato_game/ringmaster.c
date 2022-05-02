#include "potato.h"
#define MAX_CONNECT 100

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    std::cout << "ringmaster <port_num> <num_players> <num_hops>" << std::endl;
    return EXIT_FAILURE;
  }
  const char *port = argv[1];
  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);
  int socket_fd = init_server(port);
  if (socket_fd < 0)
  {
    return EXIT_FAILURE;
  }
  if(num_players <= 1) {
    return EXIT_FAILURE;
  }
  std::cout << "Potato Ringmaster" << std::endl;
  std::cout << "Players = " << num_players << std::endl;
  std::cout << "Hops = " << num_hops << std::endl;

  
  std::vector<client_info_box> address_info(num_players);
  std::vector<int> fds(num_players);
  fd_set rfds;

  // First Loop: Build Connection
  for (int i = 0; i < num_players; ++i)
  {
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1)
    {
      std::cerr << "Error: cannot accept connection on socket" << std::endl;
      return -1;
    }
    auto buffer = read_string(client_connection_fd);
    send_string(client_connection_fd, std::to_string(i));
    send_string(client_connection_fd, std::to_string(num_players));
    fds[i] = client_connection_fd;
    address_info[i] = client_info_box(buffer);
    address_info[i].client_id = i;
    FD_SET(client_connection_fd, &rfds);
    std::cout << "Player " << i << " is ready to play" << std::endl;
  }

  // Second Loop: Send Them Information with potato
  for (int i = 0; i < num_players; ++i)
  {
    std::string address(address_info[(i - 1 + num_players) % num_players].serialize());
    send_string(fds[i], address);
  }
  srand((unsigned int)time(NULL) + num_players);
  int begin_client_id = rand() % num_players;
  hop_circle potato(num_hops);
  std::string potato_str = potato.serialize();
  send_string(fds[begin_client_id], potato_str);
  std::cout << "Ready to start the game, sending potato to player " << begin_client_id << std::endl;
  // Listen to the result
  bool received = false;
  while (!received)
  {
    FD_ZERO(&rfds);
    int max_fd = 0;
    for(int i = 0; i < num_players; ++i) {
      FD_SET(fds[i], &rfds);
      max_fd = std::max(max_fd, fds[i]);
    }

    int retval = select(max_fd + 1, &rfds, NULL, NULL, NULL);
    if (retval == -1)
    {
      perror("SELECT() ERROR");
      return EXIT_FAILURE;
    }
    else if (retval == 0)
    {
      break;
    }
    else
    {
      for (int i = 0; i < fds.size(); ++i)
      {
        if (FD_ISSET(fds[i], &rfds))
        {
          std::string buffer = read_string(fds[i]);
          if (buffer.size() == 0)
          {
            continue;
          }
          hop_circle potato_circle(buffer);
          received = true;
          std::cout << "Trace of potato:" << std::endl;
          for(int i = 0; i < potato_circle.route.size(); ++i) {
            if(i != 0) {
              std::cout << ",";
            }
            std::cout << potato_circle.route[i];
          }
          std::cout << std::endl;
        }
      }
    }
  }

  //send finish
  for(int i = 0; i < num_players; ++i) {
    send_string(fds[i], "DONE");
  }
  return 0;
}
