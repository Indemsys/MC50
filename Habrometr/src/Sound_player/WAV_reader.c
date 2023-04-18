#include "MC50.h"

uint8_t               g_hdr_indx = 0;
uint8_t               af_header[AF_HEADER_SIZE];
WAVE_FormatTypeDef    wave_struct;

static  uint32_t      g_sample_rate; // Частота сэмлирования в герцах

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
int Read_wav_header_from_file(FX_FILE *fp)
{
   ULONG    actual_size;
   fx_file_read(fp, af_header, AF_HEADER_SIZE, &actual_size);
   if (actual_size != AF_HEADER_SIZE)
   {
      return 0;
   }
   return 1;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
unsigned int Player_Get_wave_file_sample_rate(void)
{
   return g_sample_rate;
}
/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
uint32_t Read_from_header(uint8_t num, T_endianness bfmt)
{
   uint32_t i = 0;
   uint32_t tmp = 0;

   if (bfmt == LITTLE_ENDIAN)
   {
      for (i = 0; i < num; i++)
      {
         tmp |= af_header[g_hdr_indx++] << (i * 8);
      }
   }
   else
   {
      for (i = num; i != 0; i--)
      {
         tmp |= af_header[g_hdr_indx++] << ((i - 1) * 8);
      }
   }
   return tmp;
}


/*******************************************************************************
* Function Name  : DAC_CODEC_WaveParsing
* Description    : Checks the format of the .WAV file and gets information about
*                  the audio format. This is done by reading the value of a
*                  number of parameters stored in the file header and comparing
*                  these to the values expected authenticates the format of a
*                  standard .WAV  file (44 bytes will be read). If  it is a valid
*                  .WAV file format, it continues reading the header to determine
*                  the  audio format such as the sample rate and the sampled data
*                  size. If the audio format is supported by this application,
*                  it retrieves the audio format in WAVE_Format structure and
*                  returns a zero value. Otherwise the function fails and the
*                  return value is nonzero.In this case, the return value specifies
*                  the cause of  the function fails. The error codes that can be
*                  returned by this function are declared in the header file.
* Input          : None
* Output         : None
* Return         : Zero value if the function succeed, otherwise it return
*                  a nonzero value which specifies the error code.
*******************************************************************************/
T_WAV_pars_err Wavefile_header_parsing(void)
{
   uint32_t tmp = 0x00;
   uint32_t extr_byte = 0;

   /* Initialize the HeaderTabIndex variable */
   g_hdr_indx = 0;

   /* Read chunkID, must be 'RIFF'      ----------------------------------------------*/
   tmp = Read_from_header(4, BIG_ENDIAN);
   if (tmp != ChunkID)
   {
      return (Unvalid_RIFF_ID);
   }
   /* Read the file length ----------------------------------------------------*/
   wave_struct.RIFFchunksize = Read_from_header(4, LITTLE_ENDIAN);

   /* Read the file format, must be 'WAVE' ------------------------------------*/
   tmp = Read_from_header(4, BIG_ENDIAN);
   if (tmp != FileFormat)
   {
      return (Unvalid_WAVE_Format);
   }
   /* Read the format chunk, must be 'fmt ' -----------------------------------*/
   tmp = Read_from_header(4, BIG_ENDIAN);
   if (tmp != FormatID)
   {
      return (Unvalid_FormatChunk_ID);
   }
   /* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
   tmp = Read_from_header(4, LITTLE_ENDIAN);
   if (tmp != 0x10)
   {
      extr_byte = 1;
   }
   /* Read the audio format, must be 0x01 (PCM) -------------------------------*/
   wave_struct.FormatTag = Read_from_header(2, LITTLE_ENDIAN);
   if (wave_struct.FormatTag != WAVE_FORMAT_PCM)
   {
      return (Unsupporetd_FormatTag);
   }
   /* Read the number of channels: 0x02->Stereo 0x01->Mono --------------------*/
   wave_struct.NumChannels = Read_from_header(2, LITTLE_ENDIAN);

   /* Read the Sample Rate ----------------------------------------------------*/
   wave_struct.SampleRate = Read_from_header(4, LITTLE_ENDIAN);

   /* Update the OCA value according to the .WAV file Sample Rate */
   g_sample_rate = wave_struct.SampleRate;

   /* Read the Byte Rate ------------------------------------------------------*/
   wave_struct.ByteRate = Read_from_header(4, LITTLE_ENDIAN);

   /* Read the block alignment ------------------------------------------------*/
   wave_struct.BlockAlign = Read_from_header(2, LITTLE_ENDIAN);

   /* Read the number of bits per sample --------------------------------------*/
   wave_struct.BitsPerSample = Read_from_header(2, LITTLE_ENDIAN);
   if (wave_struct.BitsPerSample != Bits_Per_Sample_16)
   {
      return (Unsupporetd_Bits_Per_Sample);
   }
   /* If there are Extra format bytes, these bytes will be defined in "Fact Chunk" */
   if (extr_byte == 1)
   {
      /* Read th Extra format bytes, must be 0x00 ------------------------------*/
      tmp = Read_from_header(2, LITTLE_ENDIAN);
      if (tmp != 0x00)
      {
         return (Unsupporetd_ExtraFormatBytes);
      }
      /* Read the Fact chunk, must be 'fact' -----------------------------------*/
      tmp = Read_from_header(4, BIG_ENDIAN);
      if (tmp != FactID)
      {
         return (Unvalid_FactChunk_ID);
      }
      /* Read Fact chunk data Size ---------------------------------------------*/
      tmp = Read_from_header(4, LITTLE_ENDIAN);

      /* Set the index to start reading just after the header end */
      g_hdr_indx += tmp;
   }
   /* Read the Data chunk, must be 'data' -------------------------------------*/
   tmp = Read_from_header(4, BIG_ENDIAN);
   if (tmp == LISTID)
   {
     tmp = Read_from_header(4, LITTLE_ENDIAN);
     g_hdr_indx += tmp;
   }
   else if (tmp != DataID)
   {

      return (Unvalid_DataChunk_ID);
   }
   /* Read the number of sample data ------------------------------------------*/
   wave_struct.DataSize = Read_from_header(4, LITTLE_ENDIAN);

   return (Valid_WAVE_File);
}


