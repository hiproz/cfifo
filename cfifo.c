#include "cfifo.h"

// show
void cfifo_show(cfifo_struct *fifo) {
  log(INF, "fifo info: width %u,depth %u, read index %u,write index %u,used %u\r\n",
      fifo->queue_width, fifo->queue_depth, fifo->read_index, fifo->write_index,
      fifo->used_count);
}

// init
void cfifo_init(cfifo_struct *fifo, uint8_t *queue, uint16_t width, uint16_t depth) {
  fifo->queue = queue;
  fifo->queue_depth = depth;
  fifo->queue_width = width;
  fifo->read_index = 0;
  fifo->write_index = 0;
  fifo->used_count = 0;
}

// read
int cfifo_read(cfifo_struct *fifo, uint16_t *index, uint8_t **data, uint16_t *len) {
  if (fifo->used_count > 0) {
    *index = fifo->read_index;
    *data = fifo->queue + fifo->read_index * fifo->queue_width;
    *len = fifo->queue_width;
    return 0;
  } else {
    *data = NULL;
    *len = 0;
    return 1;
  }
}

// write
int cfifo_write(cfifo_struct *fifo, uint8_t *data, uint16_t len) {

  if (len <= fifo->queue_width) {
    memcpy(fifo->queue + fifo->write_index * fifo->queue_width, data, len);
    fifo->write_index = (fifo->write_index + 1) % fifo->queue_depth;
    fifo->used_count++;
    if (fifo->used_count > fifo->queue_depth) {
      fifo->used_count = fifo->queue_depth;
      fifo->read_index = fifo->write_index;
      log(WAR, "*** fifo is full\r\n");
    }
  } else {
    log(ERR, "the data len is invalid, write error\r\n");
    return 1;
  }

  return 0;
}

uint16_t cfifo_get_size(cfifo_struct *fifo) {
  return fifo->used_count;
}

void cfifo_rmv_top(cfifo_struct *fifo) {
  if (fifo->used_count > 0) {
    // Çå³ýÊý¾Ý
    memset(fifo->queue + fifo->read_index * fifo->queue_width, 0, fifo->queue_width);
    fifo->read_index = (fifo->read_index + 1) % fifo->queue_depth;
    fifo->used_count--;
  } else {
    //log(ERR, "the fifo is empty\r\n");
  }
}

void cfifo_clear(void) {
}
