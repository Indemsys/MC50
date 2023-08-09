#ifndef WAV_READER_H
  #define WAV_READER_H

/* .WAV file format :

 Endian      Offset      Length      Contents
  big         0           4 bytes     'RIFF'             // 0x52494646
  little      4           4 bytes     <file length - 8>
  big         8           4 bytes     'WAVE'             // 0x57415645

Next, the fmt chunk describes the sample format:

  big         12          4 bytes     'fmt '          // 0x666D7420
  little      16          4 bytes     0x00000010      // Length of the fmt data (16 bytes)
  little      20          2 bytes     0x0001          // Format tag: 1 = PCM
  little      22          2 bytes     <channels>      // Channels: 1 = mono, 2 = stereo
  little      24          4 bytes     <sample rate>   // Samples per second: e.g., 22050
  little      28          4 bytes     <bytes/second>  // sample rate * block align
  little      32          2 bytes     <block align>   // channels * bits/sample / 8
  little      34          2 bytes     <bits/sample>   // 8 or 16

  // дополнение после анализа сторонних WAV файлов
  big         36          4 bytes     'LIST'          // 0x4C495354
  little      40          4 bytes     <length of the LIST block>
  little      44          *           <LIST data>

Finally, the data chunk contains the sample data:

  big         36          4 bytes     'data'        // 0x64617461
  little      40          4 bytes     <length of the data block>
  little      44          *           <sample data>

*/

typedef enum
{
  LITTLE_ENDIAN,
  BIG_ENDIAN
}T_endianness;


/* Audio file information structure */
typedef struct
{
    uint32_t  RIFFchunksize;
    uint16_t  FormatTag;
    uint16_t  NumChannels;
    uint32_t  SampleRate;
    uint32_t  ByteRate;
    uint16_t  BlockAlign;
    uint16_t  BitsPerSample;
    uint32_t  DataSize;
} WAVE_FormatTypeDef;


/* Error Identification structure */
typedef enum
{
  Valid_WAVE_File = 0,
  Unvalid_RIFF_ID,
  Unvalid_WAVE_Format,
  Unvalid_FormatChunk_ID,
  Unsupporetd_FormatTag,
  Unsupporetd_Number_Of_Channel,
  Unsupporetd_Sample_Rate,
  Unsupporetd_Bits_Per_Sample,
  Unvalid_DataChunk_ID,
  Unsupporetd_ExtraFormatBytes,
  Unvalid_FactChunk_ID
} T_WAV_pars_err;


/* Audio file header size */
  #define AF_HEADER_SIZE          100


/* Audio Parsing Constants */
  #define  ChunkID             0x52494646  /* correspond to the letters 'RIFF' */
  #define  FileFormat          0x57415645  /* correspond to the letters 'WAVE' */
  #define  FormatID            0x666D7420  /* correspond to the letters 'fmt ' */
  #define  DataID              0x64617461  /* correspond to the letters 'data' */
  #define  FactID              0x66616374  /* correspond to the letters 'fact' */
  #define  LISTID              0x4C495354  /* correspond to the letters 'LIST' */

  #define  WAVE_FORMAT_PCM     0x01
  #define  FormatChunkSize     0x10
  #define  Channel_MONO        0x01
  #define  Channel_STEREO      0x02

  #define  SampleRate_8000     8000
  #define  SampleRate_11025    11025
  #define  SampleRate_16000    16000
  #define  SampleRate_22050    22050
  #define  SampleRate_44100    44100
  #define  SampleRate_48000    48000
  #define  Bits_Per_Sample_8   8
  #define  Bits_Per_Sample_16  16


int              Read_wav_header_from_file(FX_FILE *fp);
uint32_t         Read_from_header(uint8_t num, T_endianness bfmt);
T_WAV_pars_err   Wavefile_header_parsing(void);

#endif
