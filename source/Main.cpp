extern "C" void app_main();

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#include "Hardware.h"
#include "ticker_dal.h"

namespace {
pthread_cond_t interrupt_signal;
pthread_mutex_t interrupt_lock;
}

void
__wait_for_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_wait(&interrupt_signal, &interrupt_lock);
  pthread_mutex_unlock(&interrupt_lock);
}

namespace {
void
signal_interrupt() {
  pthread_mutex_lock(&interrupt_lock);
  pthread_cond_signal(&interrupt_signal);
  pthread_mutex_unlock(&interrupt_lock);
}

void*
code_thread_start(void*) {
  app_main();
}

void*
serial_thread_start(void*) {
  const int MAX_EVENTS = 10;
  int epoll_fd = epoll_create1(0);
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];

  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);

  ev.events = EPOLLIN;
  ev.data.fd = STDIN_FILENO;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

  while (true) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("epoll wait\n");
      exit(1);
    }

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == STDIN_FILENO) {
        uint8_t buf[64];
        ssize_t nbytes = read(STDIN_FILENO, &buf, sizeof(buf));
        for (ssize_t i = 0; i < nbytes; ++i) {
          serial_add_byte(buf[i]);
        }
        signal_interrupt();
      }
    }
  }
  return NULL;
}

void*
timer_thread_start(void*) {
  while (true) {
    for (int i = 0; i < 375; ++i) {
      usleep(16);
      ticker_on_tick();
      signal_interrupt();
    }
    ticker_on_macrotick();
  }
}

void
unbuffered_terminal(bool enable) {
  static struct termios old_tio;

  if (enable) {
    tcgetattr(STDIN_FILENO, &old_tio);
    struct termios new_tio = old_tio;

    // disable canonical mode (buffered i/o) and local echo
    new_tio.c_lflag &= (~ICANON & ~ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
  }
}
}

int
main(int, char**) {
  unbuffered_terminal(true);

  pthread_t code_thread, serial_thread, timer_thread;

  pthread_cond_init(&interrupt_signal, NULL);
  pthread_mutex_init(&interrupt_lock, NULL);

  pthread_create(&code_thread, NULL, &code_thread_start, NULL);
  pthread_create(&serial_thread, NULL, &serial_thread_start, NULL);
  pthread_create(&timer_thread, NULL, &timer_thread_start, NULL);

  fprintf(stdout, "Threads running\n");

  void* result;
  pthread_join(code_thread, &result);
  pthread_join(serial_thread, &result);
  pthread_join(timer_thread, &result);

  pthread_cond_destroy(&interrupt_signal);
  pthread_mutex_destroy(&interrupt_lock);

  unbuffered_terminal(false);
  return 0;
}
