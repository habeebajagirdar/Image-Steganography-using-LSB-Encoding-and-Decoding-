#include <stdio.h>
#include "decode.h"
#include "common.h"
#include <string.h>
#include "types.h"

Status open_files_decode(DecodeInfo *decInfo)
{
    // Src Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// Function to Read and validate Decode args from argv
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // check in argv[2] .bmp is present or not
    if (strstr(argv[2], ".bmp") != NULL)
    {
        // update into structure
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure; // if no return e_failure
    }

    // check argv[3] is NULL or not
    if (argv[3] == NULL)
    {
        // copy user provided file name
        strcpy(decInfo->output_fname, "decoded");
    }
    else
    {
        strcpy(decInfo->output_fname, argv[3]);

        char *extn = strrchr(decInfo->output_fname, '.');

        // check extension is present or not
        if (extn != NULL)
        {
            *extn = '\0'; // Remove existing extension
        }

        // append .txt extension
        strcat(decInfo->output_fname, ".txt");
    }

    return e_success;
}

// Function to Skip bmp image header present in the stego image
Status skip_bmp_header(FILE *fptr_stego_image)
{
    // Move the file pointer to the beginning of the pixel data
    fseek(fptr_stego_image, 54 , SEEK_SET);
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_files_decode(decInfo) == e_failure)
    {
        return e_failure;
    }

    if (skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        return e_failure;
    }
    if (decode_output_file_extn_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf("Extention size = %u\n", decInfo->extn_size);

    if (decode_output_file_extn(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf("Extension = %s\n", decInfo->secret_file_extn);
    if(decode_output_file_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf("File size  = %u\n",decInfo->secret_file_size);

    decInfo->fptr_output_file = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output_file == NULL)
    {
        printf("ERROR: Unable to open output file\n");
        return e_failure;
    }
    if (decode_output_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}

// this function is used to decode and verify the magic string
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo)
{
    char decoded_magic[3];
    char image_buffer[8];

    // decode magic string
    for (int i = 0; i < 2; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decode_lsb_to_byte(&decoded_magic[i], image_buffer);
    }

    decoded_magic[2] = '\0';

    char user_magic[20];
    // get magic string from user
    printf("Enter magic string : ");
    scanf("%s", user_magic);

    // compare both strings
    if (strcmp(decoded_magic, user_magic) == 0)
    {
        printf("Magic string matched\n"); // if matched print message
        return e_success;
    }

    printf("Magic string mismatch\n"); // if mismatched print message
    return e_failure;
}

// This function is used to decode the size of the output file extension
Status decode_output_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    // read 32 bytes from stego image
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    // decode 32 bytes from stego image
    decode_lsb_to_int(&decInfo->extn_size, image_buffer);

    return e_success;
}

// Decode secret file extension
Status decode_output_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];

    // read and decode extension byte by byte
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decode_lsb_to_byte(&decInfo->secret_file_extn[i], image_buffer);
    }

    // null terminate the extension string
    decInfo->secret_file_extn[decInfo->extn_size] = '\0';

    return e_success;
}

// Decode secret file size
Status decode_output_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    // read 32 bytes from stego image
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    // decode LSB bits into file size
    decode_lsb_to_int(&decInfo->secret_file_size, image_buffer);

    return e_success;
}

/* Decode secret file data */
Status decode_output_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    // loop till secret file size
    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        // read 8 bytes from stego image
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        // decode one character from 8 LSB bits
        decode_lsb_to_byte(&ch, image_buffer);

        // write decoded character to output file
        fwrite(&ch, 1, 1, decInfo->fptr_output_file);
    }

    return e_success;
}

// This function is used to decode a character from 8 LSB bits
Status decode_lsb_to_byte(char *ch, char *image_buffer)
{
    // initialize character to 0
    *ch = 0;

    // run loop for 8 times
    for (int i = 0; i < 8; i++)
    {
        // extract LSB from each image byte and form a character
        *ch = (((image_buffer[i] & 1) << (7 - i)) | *ch);
    }

    return e_success; // return success
}

// This function is used to decode an integer from 32 LSB bits
Status decode_lsb_to_int(int *size, char *image_buffer)
{
    // initialize size to 0
    *size = 0;

    // run loop for 32 bits
    for (int i = 0; i < 32; i++)
    {
        // extract LSB from image bytes and form integer
        *size = (*size << 1) | (image_buffer[i] & 1);
    }

    return e_success;
}