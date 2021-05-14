// almost all code reused from Beej's tutorial

#include "network.hpp"

#include <string>
#include <iostream>
#include <cstring>
// #include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#ifdef __DEBUG__
#include <arpa/inet.h> // for print the remote address
#endif

// 坑：cannot use std namespace due to bind()
// using namespace std;


int server_setup_socket(std::string port, bool is_udp, int &sockfd) {
    // int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int status;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;    // don't care IPv4 or IPv6
    hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM; // UDP or TCP
    hints.ai_flags = AI_PASSIVE;    // fill in my IP

    if ((status = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "ERROR: " << ": getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("INFO: server_setup_socket: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("INFO: server_setup_socket: setsockpot");
            return -2;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("INFO: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        std::cerr << "ERROR: failed to bind socket" << std::endl;
        return -3;
    }

    if (!is_udp && listen(sockfd, BACKLOG) == -1) {
        std::cerr << "ERROR: failed to listen" << std::endl;
        return -4;
    }

    return 0;
}

int client_tcp_setup_socket(std::string host, std::string port, int &sockfd) {
    struct addrinfo hints, *servinfo, *p;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;    // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "ERROR: client_tcp_setup_socket: getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("INFO: client_tcp_setup_socket: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("INFO: client_tcp_setup_socket: connect");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        std::cerr << "ERROR: client failed to connect" << std::endl;
        return -2;
    }

    return 0;
}

int server_tcp_get_child_sockfd(int sockfd, int &re_sockfd) {
    struct sockaddr_storage remote_addr;
    socklen_t sin_size = sizeof remote_addr;
    int child_sockfd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size);
    if ((child_sockfd) == -1) {
        perror("ERROR: server_tcp_receive: accept");
        return -1;
    }
    re_sockfd = child_sockfd;
    return 0;
}

int tcp_send_msg(int sockfd, std::string msg) {

#ifdef __DEBUG__
    std::cout << "DEBUG: TCP sent [" << msg << "]" << std::endl;
#endif

    if (send(sockfd, msg.c_str(), msg.size(), 0) == -1) {
        perror("WARN: tcp_send_msg: send");
        return -1;
    }
    return 0;
}

int tcp_rec_msg(int sockfd, std::string &msg) {
    char buf[MAXBUFLEN];
    int numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0);
    if (numbytes == -1) {
        perror("WARN: server_tcp_receive: recv");
        return -1;
    }

    buf[numbytes] = '\0';
    msg = buf;

#ifdef __DEBUG__
    std::cout << "DEBUG: TCP received: [" << buf << "]" << std::endl;
#endif

    return 0;
}

#ifdef __DEBUG__
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
#endif

// do not care the remote address
int server_udp_rec_msg(int sockfd, std::string &msg) {
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_storage remote_addr;
    socklen_t addr_len = sizeof remote_addr;

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&remote_addr, &addr_len)) == -1) {
        perror("WARN: server_udp_rec_msg: recvfrom failed");
        return -1;
    }

    buf[numbytes] = '\0';
    msg = buf;

#ifdef __DEBUG__
    std::cout << "DEBUG: UDP received: [" << buf << "]" << std::endl;

    char ipstr[INET6_ADDRSTRLEN];
    printf("DEBUG: got packet from %s", inet_ntop(remote_addr.ss_family, 
            get_in_addr((struct sockaddr *)&remote_addr), ipstr, sizeof ipstr));
    printf(" in port is %d \n", ((struct sockaddr_in *)&remote_addr)->sin_port);
#endif

    return 0;
}

// using the existed udp socket
int client_udp_send_msg_to(int sockdfd, std::string host, std::string port, std::string msg) {
    struct addrinfo hints, *servinfo, *p;
    int status;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;    // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "ERROR: client_udp_send_msg_to: getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {

#ifdef __DEBUG__
        std::cout << "DEBUG: UDP sent [" << msg << "]" << std::endl;
#endif

        if ((numbytes = sendto(sockdfd, msg.c_str(), msg.length(), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("INFO: client_udp_send_msg_to: sendto");
            continue;
        }
        break;
    }

    if (p == NULL) {
        std::cerr << "WARN: client_udp_send_msg_to: failed to send" << std::endl;
        return -2;
    }

    if (numbytes < (int)msg.length()) {
        std::cerr << "WARN: client_udp_send_msg_to: only send " << numbytes << "bytes, expected " << msg.length() << "bytes" << std::endl;
        return -3;
    }

    return 0;
}

