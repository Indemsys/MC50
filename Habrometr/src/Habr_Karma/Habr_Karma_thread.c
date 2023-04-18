// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-03-30
// 18:17:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

#include "nx_web_http_client.h"
#include "nx_web_http_server.h"
#include "nx_crypto.h"
#include "nx_secure_tls_api.h"
#include "nx_secure_x509.h"
#include "Habr_Karma_thread.h"
#include "Habr_CA_certificate.h"

#define ENABLE_HT_DEBUG_LOG


// Перечень маркеров сопровождающих искомые числовые значения для Хаброметра
#define KARMA_MARK       "tm-karma__votes tm-karma__votes_positive\">"
#define RATING_MARK      "tm-votes-lever__score-counter_rating\">"
#define SUBSCRIPT_MARK   "tm-navigation-dropdown__option-count\">"
#define VOTES_MARK       "tm-votes-meter__value_appearance-article tm-votes-meter__value_rating\">"
#define VIEWS_MARK       "<span class=\"tm-icon-counter__value\">"
#define BOOKMARKS_MARK   "bookmarks-button__counter\">"
#define LEFT_MARK        "<"

// Стек задачи организуем статически
#define HABR_KARMA_TASK_STACK_SIZE (1024*2)
static uint8_t habr_karma_stacks[HABR_KARMA_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.habr_karma_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       habr_karma_thread;

// Идентификация строки хидера по которой определяем размер HTML страницы 
#define  HEADER_CONTENT_LENGTH        "Content-Length"
#define  HEADER_CONTENT_LENGTH_SZ     (sizeof(HEADER_CONTENT_LENGTH)-1)

// Имя файла в кторый будет сохраняться HTML страница 
#define  HTML_FILE_NAME               "file.html"


extern   const NX_SECURE_TLS_CRYPTO   nx_crypto_tls_ciphers_synergys7;

// Размеры служебных структур для расшифровки страницы во время TLS сессии и размеры служебных строк
// Размеры взяты минимально необходимые с учетом размера сертификата сервера Habr
#define HT_TLS_PACKET_BUFFER_SZ       5000
#define HT_REMOTE_CERT_BUFFER_SZ      2000
#define HT_REMOTE_ISSUER_BUFFER_SZ    2000
#define HT_CRYPTO_METADATA_CLIENT_SZ  10000
#define HT_RECEIVE_BUFFER_SZ          8196
#define HT_HEADER_NAME_SZ             128
#define HT_HEADER_VAL_SZ              256
#define CSV_FILE_NAME_MAX_SZ          64
#define CSV_STR_MAX_SZ                128
#define HT_URL_MAX_SZ                 128

// Структура с автоматами для поиска числовые значений Хаброметра
typedef struct
{
    T_marked_num_finder  karma;
    T_marked_num_finder  rating;
    T_marked_num_finder  number_of_subscribers;
    T_marked_num_finder  number_of_votes;
    T_marked_num_finder  number_of_views;
    T_marked_num_finder  number_of_bookmarks;

} T_habrometer_data;

// Управляющая структура задачи Хаброметра 
typedef struct
{
    UCHAR                        tls_packet_buffer[HT_TLS_PACKET_BUFFER_SZ];
    UCHAR                        remote_cert_buffer[HT_REMOTE_CERT_BUFFER_SZ];
    UCHAR                        remote_issuer_buffer[HT_REMOTE_ISSUER_BUFFER_SZ];
    CHAR                         crypto_metadata_client[HT_CRYPTO_METADATA_CLIENT_SZ];
    NX_SECURE_X509_CERT          remote_certificate;
    NX_SECURE_X509_CERT          remote_issuer;
    NX_SECURE_X509_CERT          trusted_certificate;
    NX_WEB_HTTP_CLIENT           http_client;
    NXD_ADDRESS                  server_ip_address;
    UCHAR                        receive_buffer[HT_RECEIVE_BUFFER_SZ];
    int32_t                      html_body_size; // Размер тела HTML страницы получаемый из объвления в HTTP хидере ответа от WEB сервера
    FX_FILE                      html_file;
    uint8_t                      disable_saving_to_html_file;

    uint32_t                     extracted_bytes;
    uint32_t                     offs;
    uint32_t                     received_bytes_total;

    char                         csv_file_name[CSV_FILE_NAME_MAX_SZ+1];
    FX_FILE                      csv_file;
    uint8_t                      disable_saving_to_csv_file;
    char                         csv_str[CSV_STR_MAX_SZ+1];

    char                         url[HT_URL_MAX_SZ+1];
    char                         *host_name;
    uint32_t                     task_repetition_interval_sec;
    uint32_t                     page_receiving_duration_ms;

    T_habrometer_data            habrometer_data;
#ifdef ENABLE_DEBUG_LOG
    char                         header_name[HT_HEADER_NAME_SZ];
    char                         header_val[HT_HEADER_VAL_SZ];
#endif

} T_habr_karma_task_cbl;


T_hamrometer_results  habr_results;

static T_habr_karma_task_cbl ht;

#ifdef ENABLE_HT_DEBUG_LOG
  #define DEBUG_LOG(...)  RTT_printf_str(0, ##__VA_ARGS__ )
#endif

/*-----------------------------------------------------------------------------------------------------


  \param field_name
  \param field_name_length
  \param field_value
  \param field_value_length
-----------------------------------------------------------------------------------------------------*/
#ifdef ENABLE_DEBUG_LOG
static void _Debug_printf_header_string(CHAR *field_name, UINT field_name_length, CHAR *field_value, UINT field_value_length)
{
  if (field_name_length >= HT_HEADER_NAME_SZ) field_name_length = HT_HEADER_NAME_SZ-1;
  if (field_value_length >= HT_HEADER_VAL_SZ) field_value_length = HT_HEADER_VAL_SZ-1;
  memcpy(ht.header_name, field_name, field_name_length);
  ht.header_name[field_name_length] = 0;
  memcpy(ht.header_val, field_value, field_value_length);
  ht.header_val[field_value_length] = 0;
  DEBUG_LOG("Header> %s : %s", ht.header_name, ht.header_val);

}
#endif

/*-----------------------------------------------------------------------------------------------------
  Инициализация автоматов по поиску нужных фрагментов на HTML странице 

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Reinit_karma_counters_search(void)
{
  ht.habrometer_data.karma.step                         = 0;
  ht.habrometer_data.karma.left_mark                    = KARMA_MARK;
  ht.habrometer_data.karma.left_mark_sz                 = strlen(KARMA_MARK);
  ht.habrometer_data.karma.right_mark                   = LEFT_MARK;
  ht.habrometer_data.karma.right_mark_sz                = strlen(LEFT_MARK);

  ht.habrometer_data.rating.step                         = 0;
  ht.habrometer_data.rating.left_mark                    = RATING_MARK;
  ht.habrometer_data.rating.left_mark_sz                 = strlen(RATING_MARK);
  ht.habrometer_data.rating.right_mark                   = LEFT_MARK;
  ht.habrometer_data.rating.right_mark_sz                = strlen(LEFT_MARK);

  ht.habrometer_data.number_of_subscribers.step          = 0;
  ht.habrometer_data.number_of_subscribers.left_mark     = SUBSCRIPT_MARK;
  ht.habrometer_data.number_of_subscribers.left_mark_sz  = strlen(SUBSCRIPT_MARK);
  ht.habrometer_data.number_of_subscribers.right_mark    = LEFT_MARK;
  ht.habrometer_data.number_of_subscribers.right_mark_sz = strlen(LEFT_MARK);

  ht.habrometer_data.number_of_votes.step                = 0;
  ht.habrometer_data.number_of_votes.left_mark           = VOTES_MARK;
  ht.habrometer_data.number_of_votes.left_mark_sz        = strlen(VOTES_MARK);
  ht.habrometer_data.number_of_votes.right_mark          = LEFT_MARK;
  ht.habrometer_data.number_of_votes.right_mark_sz       = strlen(LEFT_MARK);

  ht.habrometer_data.number_of_views.step                = 0;
  ht.habrometer_data.number_of_views.left_mark           = VIEWS_MARK;
  ht.habrometer_data.number_of_views.left_mark_sz        = strlen(VIEWS_MARK);
  ht.habrometer_data.number_of_views.right_mark          = LEFT_MARK;
  ht.habrometer_data.number_of_views.right_mark_sz       = strlen(LEFT_MARK);

  ht.habrometer_data.number_of_bookmarks.step            = 0;
  ht.habrometer_data.number_of_bookmarks.left_mark       = BOOKMARKS_MARK;
  ht.habrometer_data.number_of_bookmarks.left_mark_sz    = strlen(BOOKMARKS_MARK);
  ht.habrometer_data.number_of_bookmarks.right_mark      = LEFT_MARK;
  ht.habrometer_data.number_of_bookmarks.right_mark_sz   = strlen(LEFT_MARK);
}

/*-----------------------------------------------------------------------------------------------------
  

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Initializing_task_settings(void)
{
  snprintf(ht.url,HT_URL_MAX_SZ, "https://habr.com/ru/users/%s/posts/", wvar.habr_user_name);
  ht.host_name                    = "habr.com";
  ht.task_repetition_interval_sec = 30;
  snprintf(ht.csv_file_name, CSV_FILE_NAME_MAX_SZ, "%s.csv", wvar.habr_user_name);
}

/*-----------------------------------------------------------------------------------------------------
  Начало процесса записи страницы в файл

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static void Begin_page_file_saving(void)
{
  uint32_t res;

  if (ht.disable_saving_to_html_file == 0)
  {
    res = Recreate_file_for_write(&ht.html_file, HTML_FILE_NAME);
    if (res != FX_SUCCESS)
    {
      DEBUG_LOG("E: Recreate_file_for_write error %04X (%s %n)", res,__FUNCTION__, __LINE__);
      ht.disable_saving_to_html_file = 1;
    }
    else
    {
      DEBUG_LOG("HTML page will be saved to the file %s", HTML_FILE_NAME);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
   Функция завершения записи страницы в файл 
 
-----------------------------------------------------------------------------------------------------*/
static void End_page_file_saving(void)
{
  uint32_t res;
  if (ht.disable_saving_to_html_file == 0)
  {
    res = fx_file_close(&ht.html_file);
    if (res != FX_SUCCESS)
    {
      DEBUG_LOG("E: fx_file_close error %04X", res);
    }
    else
    {
      DEBUG_LOG("Page file %s was succesfully saved.", HTML_FILE_NAME);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Записываем HTML поток старницы в файл

-----------------------------------------------------------------------------------------------------*/
static void Write_to_page_file(void)
{
  uint32_t res;
  if (ht.disable_saving_to_html_file == 0)
  {
    res = fx_file_write(&ht.html_file, ht.receive_buffer, ht.extracted_bytes);
    if (res != FX_SUCCESS) DEBUG_LOG("E: fx_file_write error %04X (%s %n)", res,__FUNCTION__, __LINE__);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Отправляем в очередь команду воспроизведения голосового сообщения о карме
  
  \param void  
-----------------------------------------------------------------------------------------------------*/
static void Play_Karma_message(float karma)
{
  uint32_t v = (uint32_t)karma;
  T_player_msg msg;
  msg.data[0] = VOAN_CMD_PLAY;
  msg.data[1] = VOAN_MSG_KARMA;
  memcpy(&msg.data[2],&v, 4);
  Player_Send_msg_to_queue(&msg);
}

/*-----------------------------------------------------------------------------------------------------
  Извлекаем нужные данные Хаброметра из HTML потока страницы 

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Extract_harb_counters(void)
{

  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.karma);
  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.rating);
  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.number_of_subscribers);
  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.number_of_votes);
  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.number_of_views);
  Find_habr_marked_number(ht.receive_buffer, ht.extracted_bytes,&ht.habrometer_data.number_of_bookmarks);

  // Если карма изменилась, то выдать голосовое сообщение о текущей величине карт=мы
  if (habr_results.karma != ht.habrometer_data.karma.number)
  {
    habr_results.karma = ht.habrometer_data.karma.number;
    Play_Karma_message(habr_results.karma);
  }
  habr_results.rating                = ht.habrometer_data.rating.number;
  habr_results.number_of_subscribers = ht.habrometer_data.number_of_subscribers.number;
  habr_results.number_of_votes       = ht.habrometer_data.number_of_votes.number;
  habr_results.number_of_views       = ht.habrometer_data.number_of_views.number;
  habr_results.number_of_bookmarks   = ht.habrometer_data.number_of_bookmarks.number;
}

/*-----------------------------------------------------------------------------------------------------
  Сохранения данных Хаброметра в .csv  файл 

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Save_karma_counters(void)
{
  uint32_t    res;
  rtc_time_t  curr_time;

  if (ht.csv_file_name == 0) return;
  if (ht.disable_saving_to_csv_file != 0) return;


  res = fx_file_open(&fat_fs_media,&ht.csv_file, ht.csv_file_name,  FX_OPEN_FOR_WRITE);
  if (res != FX_SUCCESS)
  {
    DEBUG_LOG("E: fx_file_open error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    res = fx_file_create(&fat_fs_media, ht.csv_file_name);
    if (res != FX_SUCCESS)
    {
      DEBUG_LOG("E: fx_file_create error %04X (%s %n)", res,__FUNCTION__, __LINE__);
      ht.disable_saving_to_csv_file = 1;
      return;
    }
    res = fx_file_open(&fat_fs_media,&ht.csv_file, ht.csv_file_name,  FX_OPEN_FOR_WRITE);
    if (res != FX_SUCCESS)
    {
      DEBUG_LOG("E: fx_file_open error %04X (%s %n)", res,__FUNCTION__, __LINE__);
      ht.disable_saving_to_csv_file = 1;
      return;
    }
  }

  res = fx_file_relative_seek(&ht.csv_file,0, FX_SEEK_END);
  if (res != FX_SUCCESS)
  {
    DEBUG_LOG("E: fx_file_relative_seek error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    fx_file_close(&ht.csv_file);
    ht.disable_saving_to_csv_file = 1;
    return;
  }


  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&curr_time);
  curr_time.tm_mon++;
  snprintf(ht.csv_str,CSV_STR_MAX_SZ,"%04d.%02d.%02d  %02d:%02d:%02d; ", curr_time.tm_year+1900, curr_time.tm_mon, curr_time.tm_mday, curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec);
  res += fx_file_write(&ht.csv_file, ht.csv_str, strlen(ht.csv_str));

  snprintf(ht.csv_str,CSV_STR_MAX_SZ,"%d; %d; ", ht.page_receiving_duration_ms, ht.received_bytes_total);
  res += fx_file_write(&ht.csv_file, ht.csv_str, strlen(ht.csv_str));


  snprintf(ht.csv_str,CSV_STR_MAX_SZ,"%0.0f; %0.1f; %0.0f; ", (double)ht.habrometer_data.karma.number,(double) ht.habrometer_data.rating.number, (double)ht.habrometer_data.number_of_subscribers.number);
  res += fx_file_write(&ht.csv_file, ht.csv_str, strlen(ht.csv_str));

  snprintf(ht.csv_str,CSV_STR_MAX_SZ,"%0.0f; %0.0f; %0.0f \r\n", (double)ht.habrometer_data.number_of_votes.number, (double)ht.habrometer_data.number_of_views.number, (double)ht.habrometer_data.number_of_bookmarks.number);
  res += fx_file_write(&ht.csv_file, ht.csv_str, strlen(ht.csv_str));

  fx_file_close(&ht.csv_file);

  if (res != 0)
  {
    DEBUG_LOG("E: fx_file_write error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    ht.disable_saving_to_csv_file = 1;
    return;
  }
  else
  {
    DEBUG_LOG("Results written to file %s successfully",ht.csv_file_name);
  }

}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается из обработчика заголовка HTTP ответа для каждой строки заголовка
  Здесь можно принимать такие данные как : Content-Type, Content-Length, Date

  Например при обработке заголовочной строки "Content-Length : 239374"
  field_name будет содержать строку "Content-Length", а field_value будет содержать строку "239374"


  \param client_ptr
  \param field_name
  \param field_name_length
  \param field_value
  \param field_value_length
-----------------------------------------------------------------------------------------------------*/
VOID http_response_callback(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT field_name_length, CHAR *field_value, UINT field_value_length)
{
  if (field_name_length == HEADER_CONTENT_LENGTH_SZ)
  {
    if (memcmp(field_name, HEADER_CONTENT_LENGTH, HEADER_CONTENT_LENGTH_SZ) == 0)
    {
      if (field_value_length < 8)
      {
        char str[8];
        memcpy(str, field_value, field_value_length);
        str[field_value_length] = 0;
        ht.html_body_size = atoi(field_value);
      }
      else
      {
        ht.html_body_size = -1;
      }
    }
  }
#ifdef ENABLE_DEBUG_LOG
  _Debug_printf_header_string(field_name, field_name_length, field_value, field_value_length);
#endif
}


/*-----------------------------------------------------------------------------------------------------
  Функция настройки TLS сессии
  Вызывается каждый раз при установке защищенного соединения
  Если не запрещен keep alive, то вызывается один раз

  \param client_ptr
  \param tls_session

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT tls_setup_callback(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session)
{
  UINT res;

  res = nx_secure_tls_session_create(tls_session,&nx_crypto_tls_ciphers_synergys7, ht.crypto_metadata_client, sizeof(ht.crypto_metadata_client));
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_secure_tls_session_create error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    return (res);
  }

  res = nx_secure_tls_session_packet_buffer_set(tls_session, ht.tls_packet_buffer, sizeof(ht.tls_packet_buffer));
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_secure_tls_session_packet_buffer_set error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    return (res);
  }

  // Добавляем доверенный сертификат для проверки сертификатов сервера от которого будем получать HTML страницы
  nx_secure_x509_certificate_initialize(&ht.trusted_certificate, USERTrust_CA, sizeof(USERTrust_CA), NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE);
  nx_secure_tls_trusted_certificate_add(tls_session,&ht.trusted_certificate);

  // Выделяем место для сертификатов принимаемых от WEB сервера
  nx_secure_tls_remote_certificate_allocate(tls_session,&ht.remote_certificate, ht.remote_cert_buffer, sizeof(ht.remote_cert_buffer));
  nx_secure_tls_remote_certificate_allocate(tls_session,&ht.remote_issuer, ht.remote_issuer_buffer, sizeof(ht.remote_issuer_buffer));
  return (NX_SUCCESS);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t Get_html_page(void)
{
  UINT            res;
  UINT            get_status = NX_SUCCESS;
  NX_PACKET      *receive_packet;

  Reinit_karma_counters_search();

  DEBUG_LOG("Secure connecting.");
  res = nx_web_http_client_secure_connect(&ht.http_client,&ht.server_ip_address, NX_WEB_HTTPS_SERVER_PORT,tls_setup_callback, NX_WAIT_FOREVER);
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_web_http_client_secure_connect error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    return RES_ERROR;
  }

  res = nx_web_http_client_request_initialize(&ht.http_client,
       NX_WEB_HTTP_METHOD_GET, // GET, PUT, DELETE, POST, HEAD
       ht.url,
       ht.host_name,
       0,                      // Used by PUT and POST
       NX_FALSE,               // If true, input_size is ignored.
       NX_NULL,                // "name",
       NX_NULL,                // "password",
       NX_WAIT_FOREVER);
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_web_http_client_request_initialize error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    return RES_ERROR;
  }

  DEBUG_LOG("Request sending.");
  res = nx_web_http_client_request_send(&ht.http_client, NX_WAIT_FOREVER);
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_web_http_client_request_send error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    return RES_ERROR;
  }


  Begin_page_file_saving();

  ht.received_bytes_total = 0;

  DEBUG_LOG("File %s was created.", HTML_FILE_NAME);
  while (get_status != NX_WEB_HTTP_GET_DONE)
  {
    get_status = nx_web_http_client_response_body_get(&ht.http_client,&receive_packet, NX_WAIT_FOREVER);
    if ((get_status != NX_SUCCESS) && (get_status != NX_WEB_HTTP_GET_DONE))
    {
      DEBUG_LOG("E: nx_web_http_client_response_body_get error %04X (%s %n)", res,__FUNCTION__, __LINE__);
      break;
    }
    else
    {
      ht.extracted_bytes  = 0;
      ht.offs   = 0;
      DEBUG_LOG("Packet processing");
      do
      {
        res = nx_packet_data_extract_offset(receive_packet, ht.offs, ht.receive_buffer, HT_RECEIVE_BUFFER_SZ ,(ULONG *)&ht.extracted_bytes);
        if (res)
        {
          if (res == NX_PACKET_OFFSET_ERROR) DEBUG_LOG("Packet exhausted");
          else  DEBUG_LOG("E: nx_packet_data_extract_offset error %04X (%s %n)", res,__FUNCTION__, __LINE__);
          break;
        }
        else
        {
          DEBUG_LOG("Received packet length: %d", ht.extracted_bytes);

          Write_to_page_file();
          Extract_harb_counters();

          ht.offs += ht.extracted_bytes;
          ht.received_bytes_total += ht.extracted_bytes;
        }
      } while (1);
      res = nx_packet_release(receive_packet);
      if (res != NX_SUCCESS) DEBUG_LOG("E: nx_packet_release error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    }
  }

  End_page_file_saving();
  Save_karma_counters();

  DEBUG_LOG("Received body length: %d. Declared body length: %d ", ht.received_bytes_total, ht.html_body_size);

  if ((ht.received_bytes_total != ht.html_body_size) || (ht.received_bytes_total == 0)) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Habr_Karma_task(ULONG arg)
{
  UINT              res;
  ULONG             web_server_ip_address;
  T_sys_timestump   t1;
  T_sys_timestump   t2;
  T_sys_timestump   t3;

  Initializing_task_settings();

  DEBUG_LOG("Habr Karma task has started and is waiting for the internet.");
  // Ожидаем появления соединения с интернетом
  while ((Is_RNDIS_network_active() == 0) && (Is_ECM_Host_network_active() == 0))
  {
    Wait_ms(10);
  }
  DEBUG_LOG("Habr Karma task got internet access.");

  res =  DNS_get_host_address((UCHAR *)ht.host_name,&web_server_ip_address, MS_TO_TICKS(10000));
  if (res == NX_SUCCESS)
  {
    DEBUG_LOG("WEB server IP of <%s> = %d.%d.%d.%d",ht.url, IPADDR(web_server_ip_address));
  }
  else
  {
    DEBUG_LOG("WEB server IP of <%s> failed to get. Status = %04X",ht.url, res);
    goto err;
  }

  ht.html_body_size = 0;

  DEBUG_LOG("HTTP client creating.");
  if (wvar.usb_mode == USB_MODE_RNDIS)
  {
    res = nx_web_http_client_create(&ht.http_client, "HTTP Client",rndis_ip_ptr,&net_packet_pool, 1536);
  }
  else if (wvar.usb_mode == USB_MODE_HOST_ECM)
  {
    res = nx_web_http_client_create(&ht.http_client, "HTTP Client",ecm_host_ip_ptr,&net_packet_pool, 1536);
  }
  else
  {
    goto err;
  }
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_web_http_client_create error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    goto err;
  }

  ht.server_ip_address.nxd_ip_address.v4 = web_server_ip_address;
  ht.server_ip_address.nxd_ip_version    = NX_IP_VERSION_V4;

  nx_web_http_client_response_header_callback_set(&ht.http_client, http_response_callback);

  Get_hw_timestump(&t1);
  do
  {
    Get_html_page();

    Get_hw_timestump(&t2);
    ht.page_receiving_duration_ms = Timestump_diff_to_msec(&t1,&t2);
    DEBUG_LOG("Page receiving duration = %d msec", ht.page_receiving_duration_ms);

    do
    {
      Get_hw_timestump(&t3);
      if (Timestump_diff_to_sec(&t1,&t3) >= ht.task_repetition_interval_sec)
      {
        memcpy(&t1,&t3, sizeof(t1));
        break;
      }
      Wait_ms(10);

    } while (1);

  } while (1);

  res = nx_web_http_client_delete(&ht.http_client);
  if (res != NX_SUCCESS)
  {
    DEBUG_LOG("E: nx_web_http_client_delete error %04X (%s %n)", res,__FUNCTION__, __LINE__);
  }

err:
  DEBUG_LOG("Habr Karma task finished.");

}



/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_Habr_Karma_create(void)
{
  return tx_thread_create(
    &habr_karma_thread,
    (CHAR *)"Habr Karma",
    Habr_Karma_task,
    0,
    habr_karma_stacks,
    HABR_KARMA_TASK_STACK_SIZE,
    HABROMETER_TASK_PRIO,
    HABROMETER_TASK_PRIO,
    1,
    TX_AUTO_START
    );
}

