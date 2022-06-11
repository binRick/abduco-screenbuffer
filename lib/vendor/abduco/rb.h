//#include "../../../../bash-loadable-wireguard/src/string/ring_buffer.c"
////#include "../../../../bash-loadable-wireguard/src/string/ring_buffer.h"
//#include "../../../../bash-loadable-wireguard/src/time/timestamp.c"
//#include "../../../../bash-loadable-wireguard/src/string/stringbuffer.c"
//#include "../../../../bash-loadable-wireguard/src/fs/fsio.c"

#define RB_BYTE_SIZE    1024 * 1024 * 16 //   16MB RING BUFFER
#define RBS             128              //   QTY OF ITEMS HELD IN RING BUFFER
#define RBQ             RB_BYTE_SIZE / RBS

//ring_buffer rb_int, rb_msg, rb_byte, rb_struct1, rb_stdout;

void init_ring_buffers();
void ring_buffers_info();
int rb_push_stdout(char *dat);
