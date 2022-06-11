//#include "../../../../bash-loadable-wireguard/src/fs/fsio.h"
//#include "../../../../bash-loadable-wireguard/src/string/ring_buffer.h"
#include "rb.h"

#define RB_LOG                  "/tmp/t.log"
#define RB_FLUSH_INTERVAL_MS    10

unsigned long last_flush_ms = 0;


void ring_buffers_info(){
  init_ring_buffers();
  fprintf(stderr, AC_CLS "");
  fprintf(stderr, AC_RESETALL "");
  fprintf(stderr, FG_I_YELLOW  "");
  fprintf(stderr, FG_FAINT  "");
  buffer_print_info(rb_stdout, stderr);
  fprintf(stderr, "" AC_RESETALL);
  flush_ring_buffers();
}


void flush_ring_buffers(){
  unsigned long ms_since_last_flush = timestamp() - last_flush_ms;

//  log_info("ms_since_last_flush:%zums", ms_since_last_flush);
  if (ms_since_last_flush < RB_FLUSH_INTERVAL_MS) {
    return;
  }
  last_flush_ms = timestamp();

  char buf[RBS];

  while (buffer_data_size(rb_stdout) > 0) {
    enum BUFFER_STATUS buff_stat = buffer_pop(rb_stdout, buf, 128);
    if (buff_stat == BS_OK) {
//      log_info("Read from stdout buffer!");
//      fsio_append_text_file(RB_LOG, buf);
    }else{
      //    log_error("Failed to read stdout buffer!, %d", buff_stat);
    }
    //log_info("Read %d/%d bytes into temp buffer to %s", strlen(buf), buffer_data_size(rb_stdout), RB_LOG);
  }
}


void init_ring_buffers(){
  if (rb_init) {
    return;
  }

//  log_info("initizialing ring buffers........");
//  fsio_write_text_file(RB_LOG, "");
  rb_stdout = buffer_init((RBQ * sizeof(char) * RBS));

  rb_init = true;
}


int rb_push_stdout(char *dat){
  init_ring_buffers();
  return(buffer_push(rb_stdout, dat, RBS));
}

