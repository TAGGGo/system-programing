#include "potato.h"
#include <iostream>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "player <machine_name> <port_num>" << std::endl;
        return EXIT_FAILURE;
    }
    const char *hostname = argv[1];
    const char *port = argv[2];
    int client_socket_fd = init_client(hostname, port);
    if(client_socket_fd < 0) {
        return EXIT_FAILURE;
    }
    int server1_socket_fd = init_server("0");
    if(server1_socket_fd < 0) {
        return EXIT_FAILURE;
    }
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if(getsockname(server1_socket_fd, (struct sockaddr *)&sa, &sa_len) < 0) {
        std::cout << "Error: get sock name failed" << std::endl;
        return EXIT_FAILURE;
    }
    std::string ip_addr = get_ip();
    const char * local_ip = &ip_addr.c_str()[0];
    int local_port = ntohs(sa.sin_port);

    client_info_box info(0, local_port, std::string(local_ip));
    std::string str = info.serialize();
    send_string(client_socket_fd, str);
    info.client_id = std::stoi(read_string(client_socket_fd));
    int num_players = std::stoi(read_string(client_socket_fd));
    std::cout << "Connected as player " << info.client_id << " out of " << num_players << " total players" << std::endl;
    std::string recv_buffer = read_string(client_socket_fd);
    client_info_box server2(recv_buffer);
    int server2_socket_fd = init_client(&server2.ip.data()[0], std::to_string(server2.port).c_str());
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd = accept(server1_socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1)
    {
      std::cerr << "Error: cannot accept connection on socket" << std::endl;
      return -1;
    }
    send_string(server2_socket_fd, std::to_string(info.client_id));
    int id_from_server2s_client = std::stoi(read_string(client_connection_fd));

    
    //Now we have three sockets in ready
    std::vector<int> fds = {server2_socket_fd, client_connection_fd, client_socket_fd};
    std::vector<int> ids = {server2.client_id, id_from_server2s_client};
    bool done = false;
    srand((unsigned int)time(NULL) + 2);
    while(!done) {
        //Initialize fdset
        fd_set rfds;
        FD_ZERO(&rfds);
        int max_fd = 0;
        for(int i = 0; i < 3; ++i) {
            FD_SET(fds[i], &rfds);
            max_fd = std::max(fds[i], max_fd);
        }
        
        //Begin Select
        int retval = select(max_fd+1, &rfds, NULL, NULL, NULL);
        if(retval == -1) {
            perror("SELECT() ERROR");
            return EXIT_FAILURE;
        }
        else if(retval == 0) {
            break;
        }
        else {
            for(int i = 0; i < fds.size(); ++i) {
                if(FD_ISSET(fds[i], &rfds)) {
                    std::string buffer = read_string(fds[i]);
                    if(buffer.size() == 0) {
                        continue;
                    }
                    if(buffer == "DONE") {
                        done = true;
                        break;
                    }
                    int next_client_id = rand() % 2;
                    hop_circle potato_circle(buffer);
                    potato_circle.hops--;
                    potato_circle.route.push_back(info.client_id);
                    if(potato_circle.hops == 0) {
                        send_string(client_socket_fd, potato_circle.serialize());
                        std::cout << "I'm it" << std::endl;
                    }
                    else if(potato_circle.hops > 0){
                        std::cout << "Sending potato to " << ids[next_client_id] << std::endl;
                        send_string(fds[next_client_id], potato_circle.serialize());
                    }
                    else {
                        done = true;
                        break;
                    }
                }
            }
        }
    }

    close(client_connection_fd);
    close(server1_socket_fd);
    close(server2_socket_fd);
    close(client_socket_fd);
    return EXIT_SUCCESS;
}
