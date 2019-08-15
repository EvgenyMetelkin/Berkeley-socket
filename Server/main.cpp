#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENTS 101

int setNonblock(int fd) {
	int flags;
#if defined(O_NONBLOCK)
	if(-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

int main(int atgc, char **argv) {

	int main_socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
	int main_socket_udp_in = socket(AF_INET, SOCK_DGRAM, 0);
	int main_socket_udp_out = socket(AF_INET, SOCK_DGRAM, 0);
	if(main_socket_tcp < 0 || main_socket_udp_in < 0 || main_socket_udp_out < 0){
        perror("socket");
        exit(1);
	}

	struct sockaddr_in sock_tcp_addr;
	sock_tcp_addr.sin_family = AF_INET;
	sock_tcp_addr.sin_port = htons(23444);
	sock_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	struct sockaddr_in sock_udp_addr_in;
	sock_udp_addr_in.sin_family = AF_INET;
	sock_udp_addr_in.sin_port = htons(23445);
	sock_udp_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
	struct sockaddr_in sock_udp_addr_out;
	sock_udp_addr_out.sin_family = AF_INET;
	sock_udp_addr_out.sin_port = htons(23447);
	sock_udp_addr_out.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(main_socket_tcp, (struct sockaddr *)&sock_tcp_addr, sizeof(sock_tcp_addr)) < 0 ||
            bind(main_socket_udp_in, (struct sockaddr *)&sock_udp_addr_in, sizeof(sock_udp_addr_in)) < 0) {
            perror("bind");
            exit(2);
    }

	setNonblock(main_socket_tcp);
	listen(main_socket_tcp, SOMAXCONN);


	int epoll = epoll_create1(0);

	struct epoll_event epoll_ev;
	epoll_ev.data.fd = main_socket_tcp;
	epoll_ev.events = EPOLLIN;
	epoll_ctl(epoll, EPOLL_CTL_ADD, main_socket_tcp, &epoll_ev);
	epoll_ev.data.fd = main_socket_udp_in;
	epoll_ev.events = EPOLLIN | EPOLLONESHOT;
	epoll_ctl(epoll, EPOLL_CTL_ADD, main_socket_udp_in, &epoll_ev);

	while(true) {
		char buffer[1024];
		struct epoll_event events[MAX_EVENTS];
		int count_events = epoll_wait(epoll, events, MAX_EVENTS, -1);

		for(int i = 0; i < count_events; ++i) {
			if(events[i].data.fd == main_socket_tcp) {
				int slave_socket = accept(main_socket_tcp, 0, 0);
				setNonblock(slave_socket);
				struct epoll_event epoll_ev;
				epoll_ev.data.fd = slave_socket;
				epoll_ev.events = EPOLLIN;
				epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &epoll_ev);
			}
			else if(events[i].data.fd == main_socket_udp_in) {
                int bytes_read = recvfrom(events[i].data.fd, buffer, 1024, 0, NULL, NULL);
                std::cout << buffer << std::endl;
                sendto(main_socket_udp_out, buffer, bytes_read, 0, (struct sockaddr *)&sock_udp_addr_out, sizeof(sock_udp_addr_out));
			}
			else {
				int bytes_read = recv(events[i].data.fd, buffer, 1024, MSG_NOSIGNAL);

				if((bytes_read == 0)&&(errno != EAGAIN)) {
					shutdown(events[i].data.fd, SHUT_RDWR);
					close(events[i].data.fd);
				}
				else if (bytes_read > 0) {
                    std::cout << buffer << std::endl;
					send(events[i].data.fd,	buffer, bytes_read, MSG_NOSIGNAL);
				}
			}
		}
	}
	return 0;

}



