#ifndef APP_MEM_MAN_H
  #define APP_MEM_MAN_H


void*          App_malloc_pending(ULONG size, ULONG wait_option);
void*          App_malloc(size_t size);
void*          App_calloc(size_t num, size_t size);
void*          App_malloc_named(char const *name, size_t size);
void*          App_calloc_named(char const *name, size_t num, size_t size);

void           App_free(VOID *block_ptr);
void           App_memory_pools_creation(void *first_unused_memory);

void           App_get_RAM_pool_statistic(uint32_t *avail_bytes, uint32_t *fragments);
uint32_t       App_get_pool_size(void);
#endif // APP_MEM_MAN_H



