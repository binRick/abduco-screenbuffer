#include "../../../../bash-loadable-wireguard/src/string/ring_buffer.c"
#include "../../../../bash-loadable-wireguard/src/string/ring_buffer.h"
#include "../../../../bash-loadable-wireguard/src/time/timestamp.c"

#define RB_BYTE_SIZE    1024 * 1024 * 16 //   16MB RING BUFFER
#define RBS             128              //   QTY OF ITEMS HELD IN RING BUFFER
#define RBQ             RB_BYTE_SIZE / RBS

ring_buffer rb_int, rb_msg, rb_byte, rb_struct1, rb_stdout;

bool rb_init = false;

void init_ring_buffers();
void ring_buffers_info();
void rb_t();
int rb_push_stdout(char *dat);


void ring_buffers_info(){
  init_ring_buffers();
  buffer_print_info(rb_stdout, stderr);
}


void init_ring_buffers(){
  if (rb_init) {
    return;
  }

  rb_int    = buffer_init((RBQ * sizeof(int) * RBS));
  rb_byte   = buffer_init((RBQ * sizeof(char) * RBS));
  rb_stdout = buffer_init((RBQ * sizeof(char) * RBS));

  rb_init = true;
}


int rb_push_stdout(char *dat){
  init_ring_buffers();
  char b[RBS];

  if (sprintf(b, "%s", dat)) {
    if (b != NULL) {
      if (buffer_push(rb_stdout, b, RBS)) {
        return(strlen(b));
      }
    }
  }
  return(-1);
}


void rb_t(){
  init_ring_buffers();
  char buf_byte[RBS];
  char dat[RBS];


  void dpush(){
    char b[RBS];

    if (sprintf(b, "%zu", timestamp())) {
      if (b != NULL) {
        buffer_push(rb_byte, b, RBS);
      }
    }
    if (sprintf(b, "%d", getpid())) {
      if (b != NULL) {
        buffer_push(rb_byte, b, RBS);
      }
    }
  }
  dpush();
}

enum PacketType {
  MSG_CONTENT = 0,
  MSG_ATTACH  = 1,
  MSG_DETACH  = 2,
  MSG_RESIZE  = 3,
  MSG_EXIT    = 4,
  MSG_PID     = 5,
};

