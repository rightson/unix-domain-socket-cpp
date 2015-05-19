#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "server.hpp"

SocketThread* SocketThread::instance_ = NULL;

SocketThread* SocketThread::Instance() {
  if (!instance_) {
    instance_ = new SocketThread;
  }
  return instance_;
}

SocketThread::SocketThread()
  : pid_(0),
    socket_path_(".socket"),
    sockfd_(0),
    curr_sock_fd_(0) {
  fprintf(stdout, "Init mutex...\n");
  pthread_mutex_init(&mutex_, NULL);
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_sigaction = &thread_stopper;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGTERM, &act, NULL) < 0) {
    fprintf(stderr, "Failed to register SIGTERM\n");
  }
  if (sigaction(SIGINT, &act, NULL) < 0) {
    fprintf(stderr, "Failed to register SIGTINT\n");
  }
}

SocketThread::~SocketThread() {
  stop();
}

bool SocketThread::start() {
  fprintf(stdout, "Creating thread...\n");
  if (pthread_create(&pid_, NULL, &(SocketThread::thread_starter), this) != 0) {
    fprintf(stderr, "Error: Failed to start thread\n");
    return false;
  }
  fprintf(stdout, "Thread created\n");
  return true;
}

void SocketThread::stop() {
  fprintf(stdout, "Destroying mutex...\n");
  pthread_mutex_destroy(&mutex_);
  if (pid_) {
    pthread_cancel(pid_);
    pid_ = 0;
  }
  if (sockfd_) {
    close(sockfd_);
    sockfd_ = 0;
  }

  if (access(socket_path_.c_str(), F_OK) != -1) {
    fprintf(stdout, "Cleanup socket\n");
    unlink(socket_path_.c_str());
  }
}

void *SocketThread::thread_starter(void *obj) {
  fprintf(stdout, "Starting thread starter...\n");
  return reinterpret_cast<SocketThread *>(obj)->run_server();
}

void SocketThread::thread_stopper(int sig, siginfo_t *siginfo, void *context) {
  fprintf(stdout, "Starting thread stopper...\n");
  Instance()->stop();
  exit(0);
}

void *SocketThread::run_server() {
  fprintf(stdout, "Starting thread server...\n");
  if (pthread_mutex_trylock(&mutex_) != 0) {
    fprintf(stderr, "Error: Failed to lock mutex thread\n");
    return NULL;
  }

  fprintf(stdout, "Creating socket...\n");
  if ((sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Error: Failed to create socket\n");
    return NULL;
  }

  fprintf(stdout, "Initializing socket...\n");
  struct sockaddr_un serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, socket_path_.c_str());

  fprintf(stdout, "Binding socket...\n");
  if (bind(sockfd_, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    fprintf(stderr, "Error: Failed to bind socket\n");
    return NULL;
  }

  fprintf(stdout, "Listening socket...\n");
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

int main() {
  SocketThread::Instance()->start();
  char buffer[SocketThread::BUFFER_SIZE];
  fprintf(stdout, "Fake loop...");
  while (true) {
    scanf("%s", buffer);
  }
  SocketThread::Instance()->stop();
}
