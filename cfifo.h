#ifndef __CFIFO_H__
#define __CFIFO_H__
#include "stdint.h"
#include "string.h"

#ifdef CLOG
#include "clog.h"
#else
#define log(level, fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

typedef struct {
  uint16_t read_index;
  uint16_t write_index;
  uint16_t used_count;
  uint16_t flag; // 1:1/2满 2:3/4满 3:全满标志

  uint16_t queue_width; // 队列的宽度
  uint16_t queue_depth; // 队列的深度或者说长度
  uint8_t *queue;       // 二维数组退化后的一维数组
} cfifo_struct;

void cfifo_show(cfifo_struct *fifo);
void cfifo_init(cfifo_struct *fifo, uint8_t *queue, uint16_t width, uint16_t depth);
int cfifo_write(cfifo_struct *fifo, uint8_t *data, uint16_t len);
int cfifo_read(cfifo_struct *fifo, uint16_t *index, uint8_t **data, uint16_t *len);
void cfifo_rmv_top(cfifo_struct *fifo);
uint16_t cfifo_get_size(cfifo_struct *fifo);
#endif
