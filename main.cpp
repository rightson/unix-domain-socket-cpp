#include "server.hpp"

int main() {
  SocketThread::Instance()->start();

  fprintf(stdout, "Starting main process (fake loop)\n");
  char buffer[SocketThread::BUFFER_SIZE];
  while (true) {
    scanf("%s", buffer);
  }

  fprintf(stdout, "You will never go here\n");
  SocketThread::Instance()->stop();
}
