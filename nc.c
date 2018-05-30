#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BACKLOG 10
#define SEND_BUF_SIZE 0x1000
#define RECV_BUF_SIZE 0x1000

// TODO: reuse addr by using setsockopt
// TODO: interactively read and write
// TODO: close socket immediately when you end receiving data

void serve(int recv_sock)
{
  char buf[RECV_BUF_SIZE];
  while(1) {
    ssize_t nrecv = read(recv_sock, buf, sizeof(buf));
    if(nrecv < 0) {
      perror("read");
      exit(1);
    }
    if(nrecv == 0) {
      break;
    }
    write(1, buf, nrecv);
  }
}

int open_connection(const char* hostname, const char* port)
{
  int err;
  struct addrinfo* res;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo(hostname, port, &hints, &res);
  if(err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    exit(1);
  }
  for(struct addrinfo* ai = res; ai != NULL; ai = ai->ai_next) {
    int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if(sock < 0) {
      continue;
    }
    if(connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
      close(sock);
      continue;
    }
    // success
    freeaddrinfo(res);
    return sock;
  }
  fputs("open_connection: all candidates failed\n", stderr);
  freeaddrinfo(res);
  exit(1);
}

void send_data(const char* hostname, const char* port)
{
  int sock = open_connection(hostname, port);
  char buf[SEND_BUF_SIZE];
  while(1) {
    ssize_t nread = read(0, buf, sizeof(buf));
    if(nread < 0) {
      perror("read");
      exit(1);
    }
    if(nread == 0) {
      break;
    }
    write(sock, buf, nread);
  }
}

int sock_listen(const char* port)
{
  int err;
  struct addrinfo* res;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  err = getaddrinfo(NULL, port, &hints, &res);
  if(err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    exit(1);
  }
  for(struct addrinfo* ai = res; ai != NULL; ai = ai->ai_next) {
    int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if(sock < 0) {
      continue;
    }
    if(bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
      close(sock);
      continue;
    }
    if(listen(sock, MAX_BACKLOG) < 0) {
      close(sock);
      continue;
    }
    // success
    freeaddrinfo(res);
    return sock;
  }
  fputs("sock_listen: all candidates failed\n", stderr);
  exit(1);
}

void accept_loop(int listen_fd)
{
  while(1) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    int sock = accept(listen_fd, (struct sockaddr*)&addr, &addrlen);
    if(sock < 0) {
      perror("accept");
      exit(1);
    }
    pid_t pid = fork();
    if(pid == 0) { // child
      serve(sock);
      close(sock);
    }
  }
}

int main(int argc, char** argv)
{
  int opt;
  const char* src_port = NULL;
  bool listen_flag = false;
  while((opt = getopt(argc, argv, "lp:")) != -1) {
    switch(opt) {
      case 'l':
        listen_flag = true;
        break;
      case 'p':
        src_port = optarg;
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("Option parser: default case\n", stderr);
        exit(1);
    }
  }

  if(listen_flag) { // server mode
    if(src_port == NULL) {
      fputs("You should use -p option to specify source port\n", stderr);
      exit(1);
    }
    int listen_sock = sock_listen(src_port);
    accept_loop(listen_sock);
  } else { // client mode
    if(argc < optind + 2) {
      fprintf(stderr, "Usage: %s [options..] <host> <port>\n", argv[0]);
      fprintf(stderr, "         or\n");
      fprintf(stderr, "       %s [optoins..] -l -p <port>\n", argv[0]);
      exit(1);
    }
    send_data(argv[optind], argv[optind+1]);
  }

  return 0;
}
