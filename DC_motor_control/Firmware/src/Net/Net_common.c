// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.11
// 15:56:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "nx_secure_tls_api.h"


T_app_net_props         app_net_props;

NX_PACKET_POOL          net_packet_pool;
uint8_t                 net_packet_pool_buffer[(PACKETS_IN_POOL *(PACKET_MAX_SZ + sizeof(NX_PACKET)))] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".mempools");// 48960 байт. NX_PACKET - имеет размер 60 байт. Целиком пакет имеет размер 1628 байта


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Net_packet_pool_init(void)
{
  UINT err;

  err = nx_packet_pool_create(&net_packet_pool, "net packets", PACKET_MAX_SZ,&net_packet_pool_buffer[0],(PACKETS_IN_POOL * (PACKET_MAX_SZ + sizeof(NX_PACKET))));
  if (NX_SUCCESS != err)
  {
    APPLOG("Failed to create net packet pool. Error %d", err);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Init_Net(void)
{
  nx_system_initialize();
  Net_packet_pool_init();
  nx_secure_tls_initialize(); //Initialises the various control data structures for the TLS component

  return RES_OK;
}



