// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-02
// 15:05:57
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------
  Найти  делители для тактовой частоты SCI обеспечивающие наилучшее совпадение с заданной битовой скоростью baud в битах в  секунду

  \param baud
  \param N
  \param M

  \return uint32_t возвращает RES_ERROR если подходящих делителей найти не удалось
-----------------------------------------------------------------------------------------------------*/
uint32_t SCI_Find_best_divider(uint32_t  baud, uint8_t *cks, uint8_t *N, uint8_t *M)
{
  float    NDIV = 0;
  float    f_baud;
  uint32_t vN;
  uint32_t vM;
  float    err;

  float    min_err = 100;
  uint32_t best_N;
  uint32_t best_M;

  // Подбор подходящей частоты тактирования SCI с учетом что PCLKA = 120 МГц
  if (baud < 120)
  {
    return RES_ERROR;
  }
  if (baud < 7500)
  {
    *cks = 4;
    NDIV = 2048.0f;
  }
  else
  {
    *cks = 0;
    NDIV = 32.0f;
  }

  for (vN = 0; vN < 256; vN++)
  {
    for (vM=128; vM < 256; vM++)
    {
      f_baud          = PCLKA_FREQ / (NDIV * (256.0f / vM) * (vN+1));
      err             = fabsf((f_baud / (float)baud)-1.0f);
      if (err < min_err)
      {
        min_err = err;
        best_N = vN;
        best_M = vM;
      }
    }
  }

  if (best_N == 255) return RES_ERROR;

  *N = (uint8_t)best_N;
  *M = (uint8_t)best_M;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Длина

  \param suid
-----------------------------------------------------------------------------------------------------*/
void Get_CPU_UID(char *suid, uint8_t *uid, uint32_t suid_len)
{
  fmi_unique_id_t unique_id;
  g_fmi.p_api->uniqueIdGet(&unique_id);
  if (uid != 0)
  {
    memcpy(uid, &unique_id, CPU_ID_LEN);
  }

  if (suid != 0)
  {
    snprintf(suid, suid_len, "%02X%02X%02X%02X", unique_id.unique_id[0], unique_id.unique_id[1], unique_id.unique_id[2], unique_id.unique_id[3]);
  }
}


/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
void Reset_SoC(void)
{
  __disable_interrupt();
  // System Control Block -> Application Interrupt and Reset Control Register
  *(uint32_t*)(0xE000ED0C) = 0x05FA0000 // Обязательный шаблон при записи в  регистр SCB_AIRCR
              | BIT(2);  // Установка бита SYSRESETREQ
  for (;;)
  {
    __no_operation();
  }
}

