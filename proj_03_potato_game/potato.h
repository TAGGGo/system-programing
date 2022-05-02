#define _OE_SOCKETS
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

class client_info_box {
  public:
  int client_id;
  int port;
  std::string ip;
  client_info_box() {}
  client_info_box(int client_id, int port, std::string ip):client_id(client_id),port(port),ip(ip) { }
  client_info_box(std::string init) {;
    std::istringstream ss(init);
    std::string temp;
    std::getline(ss, temp, ':');
    client_id = std::stoi(temp);
    std::getline(ss, temp, ':');
    port = std::stoi(temp);
    std::getline(ss, temp, ':');
    ip = temp;
  }
  std::string serialize() {
    return std::to_string(client_id) + ":" + std::to_string(port) + ":" + ip;
  }
};

class hop_circle {
  public:
  int hops;
  std::vector<int> route;
  hop_circle(){hops = 0; route = std::vector<int>();}
  hop_circle(int hops):hops(hops),route(std::vector<int>()){}
  hop_circle(std::string init) {
    std::istringstream ss(init);
    std::string temp;
    std::getline(ss, temp, '\n');
    this->hops = std::stoi(temp);
    this->route = std::vector<int>();
    while(std::getline(ss, temp, '>')) {
      if(temp.size() > 0 && temp[0] <= '9' && temp[0] >= '0')
        this->route.push_back(std::stoi(temp));
    }
  }
  std::string serialize() {
    std::string ans = std::to_string(hops) + "\n";
    for(int i = 0; i < (int)route.size(); ++i) {
      ans += (std::to_string(route[i]) + ">");
    }
    ans += "\n";
    return ans;
  }
};

int init_server(const char *port)
{
  const char *hostname = NULL;
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0)
  {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    return -1;
  }

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1)
  {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)
  {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } // if

  status = listen(socket_fd, 100);
  if (status == -1)
  {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } // if
  return socket_fd;
}

int init_client(const char *hostname, const char *port)
{
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0)
  {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } // if

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1)
  {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }

  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1)
  {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  return socket_fd;
}


std::string get_ip() {
  char name[256];
  memset(name, 0, sizeof(name));
  gethostname(name,sizeof(name));
  return name;
}

int get_port(std::string & str) {
  client_info_box info(str);
  return info.port;
}

void send_string(int fd, std::string msg) {
  size_t size = strlen(msg.c_str());
  send(fd, (char *)&size, sizeof(size_t), 0);
  send(fd, &msg.c_str()[0], strlen(msg.c_str()), 0);
}

std::string read_string(int fd) {
  size_t size;
  char buffer[1024*1024];
  memset(buffer, 0, sizeof(buffer));
  size_t size1 = recv(fd,(char *)&size, sizeof(size_t), 0);
  size_t size2 = recv(fd, &buffer, size, 0);
  return buffer;
}