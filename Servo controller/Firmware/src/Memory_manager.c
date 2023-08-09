// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.12
// 18:13:44
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


uint8_t         *ram_memory_pool;
uint32_t         ram_memory_pool_size;

TX_BYTE_POOL    g_app_pool;


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t RAM_pool_create(void *first_unused_memory)
{
  ram_memory_pool = first_unused_memory;
  ram_memory_pool_size   = RAM_END - (uint32_t)ram_memory_pool + 1;

  return tx_byte_pool_create(&g_app_pool,"app_pool_mem", ram_memory_pool,ram_memory_pool_size);
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызываема сразу после создания первого потока.
  Вызов функции генерируется конфигуратором SSP
  Используем для инициализации прикладного пула динамической памяти

  The entry point for user code in a ThreadX application.

  \param first_unused_memory
-----------------------------------------------------------------------------------------------------*/
void App_memory_pools_creation(void *first_unused_memory)
{
  RAM_pool_create(first_unused_memory);

  json_set_alloc_funcs(App_malloc, App_free); // Назначаем парсеру JSON функции работы с динамической памятью
}


/*-----------------------------------------------------------------------------------------------------


  \param memory_size
  \param wait_option

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_malloc_pending(ULONG size, ULONG wait_option)
{
  void *ptr;
  if (tx_byte_allocate(&g_app_pool,&ptr, size,wait_option) != TX_SUCCESS) return NULL;
  memset(ptr, 0, size);
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
  Выделение блока памяти
  Блоки выделяются с выравниванием равным ALIGN_TYPE. Макрос ALIGN_TYPE определен в tx_api.h и равен ULONG

  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_malloc(size_t size)
{
  return App_malloc_pending(size, TX_NO_WAIT);
}


/*-----------------------------------------------------------------------------------------------------

  \param block_ptr  - Pointer to the previously allocated memory block.

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void App_free(VOID *block_ptr)
{
  tx_byte_release(block_ptr);
}

/*-----------------------------------------------------------------------------------------------------
    Функция используется в WICED

  \param num
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_calloc(size_t num, size_t size)
{
  void    *ptr;
  uint32_t res;
  res =  tx_byte_allocate(&g_app_pool,&ptr, num * size, TX_NO_WAIT);
  if (res != TX_SUCCESS) return NULL;
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
  Функция используется в WICED

  \param name
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void*  App_malloc_named(char const *name, size_t size)
{
  return App_malloc(size);
}

/*-----------------------------------------------------------------------------------------------------
  Функция используется в WICED

  \param name
  \param num
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void*  App_calloc_named(char const *name, size_t num, size_t size)
{
  return App_calloc(num,size);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t  App_get_pool_size(void)
{
  return ram_memory_pool_size;
}


/*-----------------------------------------------------------------------------------------------------


  \param avail_bytes
  \param fragments
-----------------------------------------------------------------------------------------------------*/
void App_get_RAM_pool_statistic(uint32_t *avail_bytes, uint32_t *fragments)
{
  CHAR           *pool_name;
  TX_THREAD      *first_suspended;
  ULONG           suspended_count;
  TX_BYTE_POOL   *next_pool;

  tx_byte_pool_info_get(&g_app_pool,&pool_name,(ULONG *)avail_bytes,(ULONG *)fragments,&first_suspended,&suspended_count,&next_pool);
}

