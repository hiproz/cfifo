#include "cfifo.h"

#define CFIFO_ENABLE_LOG 1
#define CLOG             1

#if CFIFO_ENABLE_LOG
#ifdef CLOG
#include "clog.h"
#else
#define clog(level, fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#else  //CFIFO_ENABLE_LOG
#define clog(level, fmt, ...)
#endif
#if MEM_TYPE_FLASH_ENABLE
uint16_t fstorage_read(uint32_t addr, uint8_t* buf, uint16_t len);
#else
uint16_t fstorage_read(uint32_t addr, uint8_t** buf, uint16_t len);
#endif

uint16_t fstorage_write(uint32_t addr, uint8_t* buf, uint16_t len);
void     fstorage_erase_one_page(uint32_t addr);

// show
void cfifo_show(cfifo_struct* fifo) {
  clog_war("fifo info: width %u,depth %u, read index %u,write index %u,used %u\r\n", fifo->queue_width, fifo->queue_depth, fifo->read_index, fifo->write_index, fifo->used_count);
}

// init
void cfifo_init(cfifo_struct* fifo, uint32_t data_addr, uint16_t width, uint16_t depth, uint32_t flash_block_table_addr, uint32_t flash_page_size) {
// 读取本地记录信息
#if MEM_TYPE_FLASH_ENABLE
  fstorage_read(flash_block_table_addr, (uint8_t*)fifo, sizeof(cfifo_struct));
#endif
  if (fifo->init_ind != 0x55) {
    fifo->init_ind    = 0x55;
    fifo->queue       = data_addr;
    fifo->queue_width = width;
    fifo->queue_depth = depth;
    fifo->read_index  = 0;
    fifo->write_index = 0;
    fifo->used_count  = 0;
#if MEM_TYPE_FLASH_ENABLE
    fifo->flash_block_table_addr = flash_block_table_addr;
    fifo->flash_page_size        = flash_page_size;

    fstorage_erase_one_page(fifo->flash_block_table_addr);
    fstorage_write(fifo->flash_block_table_addr, (uint8_t*)fifo, sizeof(cfifo_struct));
#endif
    clog_run("init cfifo success\r\n");
  } else {
    cfifo_show(fifo);
  }
}

// read
/**
 * 
 * return :0 success, 1 failed
*/
#if MEM_TYPE_FLASH_ENABLE
int cfifo_read(cfifo_struct* fifo, uint8_t* data, uint16_t len) {
#else
int      cfifo_read(cfifo_struct* fifo, uint8_t** data, uint16_t len) {
#endif
  clog_run("fifo used count:%u\r\n", fifo->used_count);
  clog_run("fifo start:%x, read index:%d, width:%d\r\n", fifo->queue, fifo->read_index, fifo->queue_width);
  if (fifo->used_count > 0) {
#if MEM_TYPE_FLASH_ENABLE
    fstorage_read((uint32_t)(fifo->queue + fifo->read_index * fifo->queue_width), data, len);
#else
    *data = fifo->queue + fifo->read_index * fifo->queue_width;
#endif

    return 0;
  } else {
    *data = NULL;
    return 1;
  }
}

/**
 * 
 * return :0 success, 1 failed
*/
int cfifo_write(cfifo_struct* fifo, uint8_t* data, uint16_t len) {

  if (len <= fifo->queue_width) {
#if MEM_TYPE_FLASH_ENABLE
    // todo 遍历地址范围，擦除必要的page,目前只考虑页面对齐，且内容小于页面的情况，也就是
    // 擦除只能出现在block的首地址。如果要实现通用场景，请自行根据实际需求修改
    if (((uint32_t)fifo->queue + fifo->write_index * fifo->queue_width) % fifo->flash_page_size == 0) {
      fstorage_erase_one_page((uint32_t)(fifo->queue + fifo->write_index * fifo->queue_width));
    }

    fstorage_write((uint32_t)(fifo->queue + fifo->write_index * fifo->queue_width), data, len);
#else
    memcpy(fifo->queue + fifo->write_index * fifo->queue_width, data, len);
#endif
    fifo->write_index = (fifo->write_index + 1) % fifo->queue_depth;
    fifo->used_count++;
    if (fifo->used_count > fifo->queue_depth) {
      fifo->used_count = fifo->queue_depth;
      fifo->read_index = fifo->write_index;
      clog_war("*** fifo is full\r\n");
    }
#if MEM_TYPE_FLASH_ENABLE
    fstorage_erase_one_page(fifo->flash_block_table_addr);
    fstorage_write(fifo->flash_block_table_addr, (uint8_t*)fifo, sizeof(cfifo_struct));
#endif
    return len;
  } else {
    clog_err("the data len %d is invalid than %d, write error\r\n", len, fifo->queue_width);
    return 0;
  }
}

uint16_t cfifo_get_size(cfifo_struct* fifo) { return fifo->used_count; }

void cfifo_rmv_top(cfifo_struct* fifo) {
  if (fifo->used_count > 0) {
    clog_run("remove index:%u\r\n", fifo->read_index);
    fifo->read_index = (fifo->read_index + 1) % fifo->queue_depth;
    fifo->used_count--;
#if MEM_TYPE_FLASH_ENABLE
    fstorage_erase_one_page(fifo->flash_block_table_addr);
    fstorage_write(fifo->flash_block_table_addr, (uint8_t*)fifo, sizeof(cfifo_struct));
#endif
  } else {
    clog_err("the fifo is empty.\r\n");
  }
}

/*
清除fifo的block table
*/
void cfifo_clear(cfifo_struct* fifo) {
  fifo->init_ind    = 0;
  fifo->read_index  = 0;
  fifo->write_index = 0;
  fifo->used_count  = 0;
#if MEM_TYPE_FLASH_ENABLE
  clog_run("erase one page:%x\r\n", fifo->flash_block_table_addr);
  fstorage_erase_one_page(fifo->flash_block_table_addr);
  clog_run("write:%x\r\n", fifo->flash_block_table_addr);
  fstorage_write(fifo->flash_block_table_addr, (uint8_t*)fifo, sizeof(cfifo_struct));
#endif
  clog_run("cfifo clean complete\r\n");
}
