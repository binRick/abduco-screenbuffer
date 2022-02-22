#include "/root/bash-loadable-wireguard/src/log/log.h"


static void client_sigwinch_handler(int sig) {
  client.need_resize = true;
}


static bool client_send_packet(Packet *pkt) {
  print_packet("client-send:", pkt);
  if (send_packet(server.socket, pkt)) {
    return(true);
  }
  debug("FAILED\n");
  server.running = false;
  return(false);
}


static bool client_recv_packet(Packet *pkt) {
  if (recv_packet(server.socket, pkt)) {
    print_packet("client-recv:", pkt);
    return(true);
  }
  debug("client-recv: FAILED\n");
  server.running = false;
  return(false);
}


static void client_restore_terminal(void) {
  log_info("restore term..........");
  if (!has_term) {
    return;
  }
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
  if (alternate_buffer) {
    log_info("alt buffer.....");
    printf("\033[?25h\033[?1049l");
    fflush(stdout);
    alternate_buffer = false;
  }else{
    log_info("main buffer.");
  }
}


static void client_setup_terminal(void) {
  if (!has_term) {
    return;
  }
  atexit(client_restore_terminal);

  cur_term          = orig_term;
  cur_term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF);
  cur_term.c_oflag &= ~(OPOST);
  cur_term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  cur_term.c_cflag &= ~(CSIZE | PARENB);
  cur_term.c_cflag |= CS8;
#if  __linux__
  cur_term.c_cc[VLNEXT] = _POSIX_VDISABLE;
#endif
  cur_term.c_cc[VMIN]  = 1;
  cur_term.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &cur_term);

  if (!alternate_buffer) {
    printf("\033[?1049h\033[H");
    fflush(stdout);
    alternate_buffer = true;
  }
}


static int client_mainloop(void) {
  sigset_t emptyset, blockset;

  sigemptyset(&emptyset);
  sigemptyset(&blockset);
#if  __linux__
  sigaddset(&blockset, SIGWINCH);
#endif
  sigprocmask(SIG_BLOCK, &blockset, NULL);

  client.need_resize = true;
  Packet pkt = {
    .type = MSG_ATTACH,
    .u.i  = client.flags,
    .len  = sizeof(pkt.u.i),
  };
  client_send_packet(&pkt);

  while (server.running) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    FD_SET(server.socket, &fds);

    if (client.need_resize) {
      struct winsize ws;
      if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
        Packet pkt = {
          .type = MSG_RESIZE,
          .u    = { .ws = { .rows = ws.ws_row, .cols       = ws.ws_col } },
          .len  = sizeof(pkt.u.ws),
        };
        if (client_send_packet(&pkt)) {
          client.need_resize = false;
        }
      }
    }

    if (pselect(server.socket + 1, &fds, NULL, NULL, NULL, &emptyset) == -1) {
      if (errno == EINTR) {
        continue;
      }
      die("client-mainloop");
    }

    if (FD_ISSET(server.socket, &fds)) {
      Packet pkt;
      if (client_recv_packet(&pkt)) {
        log_info("recv packet of %db.......\n", pkt.len);
        switch (pkt.type) {
        case MSG_CONTENT:
          if (!passthrough) {
            write_all(STDOUT_FILENO, pkt.u.msg, pkt.len);
            log_info("writing packet to stdout of %db.......\n", pkt.len);
//            rb_t();
            rb_push_stdout((char *)pkt.u.msg);
            ring_buffers_info();
          }
          break;
        case MSG_RESIZE:
          client.need_resize = true;
          break;
        case MSG_EXIT:
          client_send_packet(&pkt);
          close(server.socket);
          return(pkt.u.i);
        }
      }
    }
    /*
     * char *BUFFER_MODE = getenv("BUFFER_MODE");
     *
     * fprintf(stderr, "BUFFER_MODE:%s\n", BUFFER_MODE);
     * if(strcmp(BUFFER_MODE,"1") == 0){
     * //    fprintf(STDIN_FILENO,"xxxx\n");
     * //      Packet pkt1 = { .type = MSG_DETACH, .len = 0  };
     * //strncpy(pkt1.u.msg, KEY_DETACH, sizeof(KEY_DETACH));
     *            //client_send_packet(&pkt1);
     * fprintf(stderr, "sending..............\n");
     * //			close(server.socket);
     *
     *  //		return -1;
     * //strlen(np->len))
     * sleep(1);
     * }*/
    if (FD_ISSET(STDIN_FILENO, &fds)) {
      Packet  pkt = { .type = MSG_CONTENT };
      ssize_t len = read(STDIN_FILENO, pkt.u.msg, sizeof(pkt.u.msg));
      if (len == -1 && errno != EAGAIN && errno != EINTR) {
        die("client-stdin");
      }
      if (len > 0) {
        debug("client-stdin: %c\n", pkt.u.msg[0]);
        pkt.len = len;
        if (KEY_REDRAW && pkt.u.msg[0] == KEY_REDRAW) {
          client.need_resize = true;
        } else if (pkt.u.msg[0] == KEY_DETACH) {
          pkt.type = MSG_DETACH;
          pkt.len  = 0;
          client_send_packet(&pkt);
          close(server.socket);
          return(-1);
        } else if (!(client.flags & CLIENT_READONLY)) {
          client_send_packet(&pkt);
        }
      } else if (len == 0) {
        debug("client-stdin: EOF\n");
        return(-1);
      }
    }
  }

  return(-EIO);
} /* client_mainloop */
