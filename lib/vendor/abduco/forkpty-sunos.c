/*
 * Copyright (c) 2008 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef TTY_NAME_MAX
#define TTY_NAME_MAX    TTYNAME_MAX
#endif


pid_t forkpty(int *master, char *name, struct termios *tio, struct winsize *ws){
  int   slave;
  char  *path;
  pid_t pid;

  if ((*master = open("/dev/ptmx", O_RDWR | O_NOCTTY)) == -1) {
    return(-1);
  }
  if (grantpt(*master) != 0) {
    goto out;
  }
  if (unlockpt(*master) != 0) {
    goto out;
  }

  if ((path = ptsname(*master)) == NULL) {
    goto out;
  }
  if (name != NULL) {
    strlcpy(name, path, TTY_NAME_MAX);
  }
  if ((slave = open(path, O_RDWR | O_NOCTTY)) == -1) {
    goto out;
  }

  switch (pid = fork()) {
  case -1:
    goto out;
  case 0:
    close(*master);

    setsid();
#ifdef TIOCSCTTY
    if (ioctl(slave, TIOCSCTTY, NULL) == -1) {
      return(-1);
    }
#endif

    if (ioctl(slave, I_PUSH, "ptem") == -1) {
      return(-1);
    }
    if (ioctl(slave, I_PUSH, "ldterm") == -1) {
      return(-1);
    }

    if (tio != NULL && tcsetattr(slave, TCSAFLUSH, tio) == -1) {
      return(-1);
    }
    if (ioctl(slave, TIOCSWINSZ, ws) == -1) {
      return(-1);
    }

    dup2(slave, 0);
    dup2(slave, 1);
    dup2(slave, 2);
    if (slave > 2) {
      close(slave);
    }
    return(0);
  }

  close(slave);
  return(pid);

out:
  if (*master != -1) {
    close(*master);
  }
  if (slave != -1) {
    close(slave);
  }
  return(-1);
} /* forkpty */
