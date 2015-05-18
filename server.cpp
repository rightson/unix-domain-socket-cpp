#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "server.hpp"

SocketThread::SocketThread()
  : pid_(0),
    sockfd_(0),
    curr_sock_fd_(0) {
  printf("Init mutex...\n");
  pthread_mutex_init(&mutex_, NULL);
}

SocketThread::~SocketThread() {
  stop();
}

bool SocketThread::start() {
  printf("Creating thread...\n");
  if (pthread_create(&pid_, NULL, &(SocketThread::thread_helper), this) != 0) {
    fprintf(stderr, "Error: Failed to start thread\n");
    return false;
  }
  printf("Thread created\n");
  return true;
}

void SocketThread::stop() {
  printf("Destroying mutex...\n");
  pthread_mutex_destroy(&mutex_);
  if (pid_) {
    pthread_cancel(pid_);
    pid_ = 0;
  }
  if (sockfd_) {
    close(sockfd_);
    sockfd_ = 0;
  }
}

void *SocketThread::thread_helper(void *obj) {
  printf("Starting thread helper...\n");
  return reinterpret_cast<SocketThread *>(obj)->thread_server();
}

void *SocketThread::thread_server() {
  printf("Starting thread server...\n");
  if (pthread_mutex_trylock(&mutex_) != 0) {
    fprintf(stderr, "Error: Failed to lock mutex thread\n");
    return NULL;
  }

  printf("Creating socket...\n");
  if ((sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Error: Failed to create socket\n");
    return NULL;
  }

  printf("Initializing socket...\n");
  struct sockaddr_un serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, ".socket");

  printf("Binding socket...\n");
  if (bind(sockfd_, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    fprintf(stderr, "Error: Failed to bind socket\n");
    return NULL;
  }

  printf("Listening socket...\n");
  if (listen(sockfd_, MAX_BACKLOG) == -1) {
    fprintf(stderr, "Error: Failed to listen to socket\n");
    return NULL;
  }

  struct sockaddr_un cli_addr;
  char buffer[BUFFER_SIZE + 1];
  while (true) {
    socklen_t cli_len = sizeof(cli_addr);
    if ((curr_sock_fd_ = accept(sockfd_, (struct sockaddr *)&cli_addr, &cli_len)) == -1) {
      fprintf(stderr, "Error: Failed to accept socket\n");
      continue;
    }
    bzero(buffer, BUFFER_SIZE + 1);
    if (read(curr_sock_fd_, buffer, BUFFER_SIZE) == -1) {
      fprintf(stderr, "Error: Failed to read socket\n");
      continue;
    }
    fprintf(stdout, "=> read buffer [%s]\n", buffer);
    close(curr_sock_fd_);
  }
}

void terminate(SocketThread &) {

}

int main() {
  SocketThread socket;
  socket.start();
  char buffer[SocketThread::BUFFER_SIZE];
  printf("Fake loop...");
  while (true) {
    scanf("%s", buffer);
  }
  socket.stop();
}
