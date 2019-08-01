#ifndef __CFIFO_H__
#define __CFIFO_H__
#include "stdint.h"
#include "string.h"

typedef struct {
  uint16_t read_index;
  uint16_t write_index;
  uint16_t used_count;

  uint16_t queue_width; // width of queue
  uint16_t queue_depth; // depth of queue
  uint32_t queue;       // start address of memery,remind the 4 bytes aligned
#if MEM_TYPE_FLASH_ENABLE
  uint32_t flash_block_table_addr; //flash模式下的块地址表的首地址
  uint16_t flash_page_size;
#endif
} cfifo_struct;

void cfifo_show(cfifo_struct *fifo);
void cfifo_init(cfifo_struct *fifo, uint32_t data_addr, uint16_t width, uint16_t depth, uint32_t flash_block_table_addr,
                uint32_t flash_page_size);
int cfifo_write(cfifo_struct *fifo, uint8_t *data, uint16_t len);

#if MEM_TYPE_FLASH_ENABLE
int cfifo_read(cfifo_struct *fifo, uint8_t *data, uint16_t len);
#else
int cfifo_read(cfifo_struct *fifo, uint8_t **data, uint16_t len);
#endif

void cfifo_rmv_top(cfifo_struct *fifo);
uint16_t cfifo_get_size(cfifo_struct *fifo);
#endif
