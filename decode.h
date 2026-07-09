#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    
    /*Stego Image info*/
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret file info*/
    uint extn_size;
    char secret_file_extn[MAX_FILE_SUFFIX];

    uint secret_file_size;

    /*Output File Info*/
    char output_fname[20];
    FILE *fptr_output_file;


} DecodeInfo;


/* Decoding function prototype */

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_decode(DecodeInfo *decInfo);

/* Skip bmp image header */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Store Magic String */
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo);

Status decode_output_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_output_file_extn( DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_output_file_size( DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_output_file_data(DecodeInfo *decInfo);

/* Decode function, which does the real encoding */
//Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Decode a byte into LSB of image data array */
Status decode_lsb_to_byte(char *ch, char *image_buffer);

Status decode_lsb_to_int (int *size, char *image_buffer);


#endif
