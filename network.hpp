#ifndef NETWORK_H
#define NETWORK_H

#include <string>

#define PORT_HOSPITAL_A "30420"
#define PORT_HOSPITAL_B "31420"
#define PORT_HOSPITAL_C "32420"
#define PORT_SCHEDULER_UDP "33420"
#define PORT_SCHEDULER_TCP "34420"

// #define __DEBUG__

#define BACKLOG 10 // how many pending connections queue will hold
#define MAXBUFLEN 100
#define NUM_HOSPITAL 3


int server_setup_socket(std::string port, bool is_udp, int &sockfd);
int client_tcp_setup_socket(std::string host, std::string port, int &sockfd);

int server_tcp_get_child_sockfd(int sockfd, int &re_sockfd);
int tcp_send_msg(int child_sockfd, std::string msg);
int tcp_rec_msg(int child_sockfd, std::string &msg);

int client_udp_send_msg_to(int sockdfd, std::string host, std::string port, std::string msg);
int server_udp_rec_msg(int sockfd, std::string &msg);

#endif