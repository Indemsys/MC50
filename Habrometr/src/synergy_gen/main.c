#include "bsp_api.h"
#include "tx_api.h"

#ifdef TX_USER_TRACE_BUFFER_DECLARE
TX_USER_TRACE_BUFFER_DECLARE;
#endif

extern void App_memory_pools_creation(void *first_unused_memory);
extern void Main_thread(ULONG arg);

TX_THREAD main_thread;

static uint8_t main_thread_stack[3072] BSP_PLACE_IN_SECTION_V2(".stack.main_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void main_thread_create(void)
{

  tx_thread_create(
                   &main_thread,
                   (CHAR *)"Main Thread",
                   Main_thread,
                   (ULONG) NULL,
                   &main_thread_stack,
                   2048,
                   1,
                   1,
                   1,
                   TX_AUTO_START
                  );
}
/*-----------------------------------------------------------------------------------------------------


  \param first_unused_memory
-----------------------------------------------------------------------------------------------------*/
void tx_application_define(void *first_unused_memory)
{
  main_thread_create();

#ifdef TX_USER_ENABLE_TRACE
  TX_USER_ENABLE_TRACE;
#endif

  App_memory_pools_creation(first_unused_memory);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int
-----------------------------------------------------------------------------------------------------*/
int main(void)
{
  __disable_irq();
  tx_kernel_enter();

  return 0;
}



