/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : filex_stub.h
 * Description  : FileX Stub Support Template header file.
 **********************************************************************************************************************/

/* This is a stub routine for FileX for a user application that do not have FileX to rely on. To use this module,
 * users need to define data types to match their system requirements.
 */

#ifndef FILEX_STUB_H
#define FILEX_STUB_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "tx_api.h"
/* This is a stub routine for FileX, for NetX applications that do not have FileX to rely on.  */

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/* Define your own data structures used by FileX stub functions. */

typedef struct FX_MEDIA_STRUCT
{

    /* This structure must be mapped to match application's file system.  */
    ULONG   tbd;
} FX_MEDIA;

typedef struct FX_FILE_STRUCT
{

    /* This structure must be mapped to match application's file system.  */
    ULONG   tbd;
    ULONG   fx_file_current_file_size;
} FX_FILE;

typedef struct FX_LOCAL_PATH_STRUCT
{

    /* This structure must be mapped to match application's file system.  */
    ULONG   tbd;
} FX_LOCAL_PATH;


/* Define your own basic constants used by FileX stub functions. */

#define FX_OPEN_FOR_READ            0
#define FX_OPEN_FOR_WRITE           1

#define FX_MAX_LONG_NAME_LEN        256

#define FX_SUCCESS                  0x00
#define FX_NULL                     0
#define FX_ACCESS_ERROR             0x06
#define FX_END_OF_FILE              0x09
#define FX_NOT_A_FILE               0x05
#define FX_NO_MORE_ENTRIES          0x0F
#define FX_DIRECTORY                0x10
#define FX_READ_ONLY                0x01

/***********************************************************************************************************************
 * Function prototypes
 **********************************************************************************************************************/

UINT        fx_directory_attributes_read(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes_ptr);
UINT        fx_directory_attributes_set(FX_MEDIA *media_ptr, CHAR *directory_name, UINT attributes);
UINT        fx_directory_create(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT        fx_directory_delete(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT        fx_directory_rename(FX_MEDIA *media_ptr, CHAR *old_directory_name, CHAR *new_directory_name);
UINT        fx_directory_first_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT        fx_directory_first_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, 
                ULONG *size, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT        fx_directory_next_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT        fx_directory_next_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, 
                ULONG *size, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT        fx_directory_name_test(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT        fx_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                        UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT        fx_directory_default_set(FX_MEDIA *media_ptr, CHAR *new_path_name);
UINT        fx_directory_default_get(FX_MEDIA *media_ptr, CHAR **return_path_name);

UINT        fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated);
UINT        fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name);
UINT        fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name);
UINT        fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name);
UINT        fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes);
UINT        fx_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr);
UINT        fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name, 
                UINT open_type);
UINT        fx_file_close(FX_FILE *file_ptr);
UINT        fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size);
UINT        fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size);
UINT        fx_file_allocate(FX_FILE *file_ptr, ULONG size);
UINT        fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from);
UINT        fx_file_seek(FX_FILE *file_ptr, ULONG byte_offset);
UINT        fx_file_truncate(FX_FILE *file_ptr, ULONG size);
UINT        fx_file_truncate_release(FX_FILE *file_ptr, ULONG size);
UINT        fx_directory_local_path_restore(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr);
UINT        fx_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name);
UINT        fx_directory_local_path_get(FX_MEDIA *media_ptr, CHAR **return_path_name);
UINT        fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                            CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors, 
                            ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster, 
                            UINT heads, UINT sectors_per_track);
UINT        fx_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                          VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr, 
                          VOID *memory_ptr, ULONG memory_size);
UINT        fx_media_flush(FX_MEDIA *media_ptr);
#endif

