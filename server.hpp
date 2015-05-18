class SocketThread {
public:
  SocketThread();
  ~SocketThread();
  bool start();
  void stop();
  enum {
    MAX_BACKLOG = 20,
    BUFFER_SIZE = 8192
  };

private:
  static void* thread_helper(void *obj);
  void * thread_server();
  pthread_t pid_;
  pthread_mutex_t mutex_;
  int sockfd_;
  int curr_sock_fd_;
};
