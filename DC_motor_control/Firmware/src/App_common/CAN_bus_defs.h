#ifndef CAN_BUS_DEFS_H
  #define CAN_BUS_DEFS_H


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой сервоконтролера
// Номер платы записывается в биты 20...23 идентификатора
//******************************************************************************************************************************************************
  #define DMC01_ONBUS_MSG                     0x1A00FFFF  // Асинхронная посылка из платы сервоконтролера
  #define DMC01_REQ                           0x1A01FFFF  // Запрос данных
     // data[0] - Идетификатор команды
     #define DMC01_REQ_STATUS                          0x01 // Команда запроса статуса
     #define DMC01_REQ_ZERO                            0x02 // Команда фиксирующая пребывание в точке закрытия
     #define DMC01_REQ_OPEN                            0x03 // Команда открыть
     #define DMC01_REQ_CLOSE                           0x04 // Команда закрыть
     #define DMC01_REQ_BRAKE                           0x05 // Команда затормозить
     #define DMC01_REQ_SET0GRAD                        0x06 // Команда установки позиции 0 градусов
     #define DMC01_REQ_SET90GRAD                       0x07 // Команда установки позиции 90 градусов
     #define DMC01_REQ_CLOS_PRESS_PULSE                0x08 // Команда сформирорвать импульс на закрытие
     #define DMC01_REQ_OPEN_PRESS_PULSE                0x09 // Команда сформирорвать импульс на открытие
     #define DMC01_REQ_FREE_RUN                        0x0A // Команда свободного хода
     #define DMC01_REQ_RESET                           0x0B // Команда сброса контроллера
     #define DMC01_REQ_SETOPENANGLE                    0x0C // Команда установки угла открытия
     #define DMC01_REQ_SETCLOSEANGLE                   0x0D // Команда установки угла закрытия

  #define DMC01_ANS                           0x1A02FFFF  // Ответ с данными


#endif



