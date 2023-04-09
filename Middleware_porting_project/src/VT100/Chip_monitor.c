// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-05
// 13:58:37
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



const T_VT100_Menu_item MENU_MC50_ITEMS[] =
{
  { '1', Do_MC50_System_info            , 0                        },
  { '2', Do_Flash_control               , 0                        },
  { '3', Do_SD_card_control             , 0                        },
  { '4', Do_SD_card_FS_test             , 0                        },
  { 'R', 0                              , 0                        },
  { 'M', 0                              , (void *)&MENU_MAIN       },
  { 0                                                 }
};

const T_VT100_Menu      MENU_MC50          =
{
  "MC50 MENU",
  "\033[5C MAIN MENU \r\n"
  "\033[5C <1> - System info\r\n"
  "\033[5C <2> - Flash memory guard control\r\n"
  "\033[5C <3> - SD card guard control\r\n"
  "\033[5C <4> - SD card file system test\r\n",
  MENU_MC50_ITEMS,
};

