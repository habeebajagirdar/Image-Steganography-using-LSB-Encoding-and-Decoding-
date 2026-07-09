#include <stdio.h>
#include "encode.h"
#include "common.h"
#include <string.h>
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

// getting file size by using fseek and ftell
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// this function is used to read the command line arguments and validate them
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // check in argv[2] .bmp is present or not
    if (strstr(argv[2], ".bmp") != NULL)
    {
        // update into structure
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure; // if no return e_failure
    }

    // check in argv[3] . is present or not
    if (strstr(argv[3], ".") != NULL)
    {
        // update into structure
        encInfo->secret_fname = argv[3];
        char *extn = strrchr(encInfo->secret_fname, '.');
        strcpy(encInfo->extn_secret_file, extn);
    }
    else
    {
        return e_failure; // if no return e_failure
    }

    // check argv[4] is NULL or not
    if (argv[4] != NULL) // if not NULL
    {
        if (strstr(argv[4], ".bmp") != NULL) // check in argv[4] .bmp is present or not
        {
            encInfo->stego_image_fname = argv[4]; // update into structure
        }
        else
        {
            return e_failure;
        }
    }

    // if it is NULL
    else
    {
        encInfo->stego_image_fname = "stego.bmp"; // store deafault name
    }

    return e_success;
}

// this function is used to check the capacity of the image and size of the secret file
Status check_capacity(EncodeInfo *encInfo)
{
    // get the image capacity by calling the get_image_size_for_bmp function and assign it to image_capacity
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // if the image capacity is greater than the size of the secret file plus the size of the header then return e_success
    if (encInfo->image_capacity > (2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

// this function is used to copy the header of the bmp image from source image to destination image
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    // rewind both src and dest address
    rewind(fptr_src_image);

    rewind(fptr_dest_image);

    // read the 54 bytes from src
    fread(buffer, 54, 1, fptr_src_image);

    // write the 54 bytes into dest
    fwrite(buffer, 54, 1, fptr_dest_image);

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
    {
        return e_failure; // return e_failure
    }

    if (check_capacity(encInfo) == e_failure)
    {
        return e_failure; // return e_failure
    }

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }
    if (encode_secret_file_extn_size(encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        return e_failure;
    }
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}

// This function is used to hide a predefined magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];
    // loop for magic_string length times
    for (int i = 0; i < strlen(magic_string); i++)
    {
        // read the 8 bytes from beautiful.bmp and store into image buffer
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        // encode the magic string into the image buffer by using the lsb technique
        encode_byte_to_lsb(magic_string[i], image_buffer);

        // write the 8 bytes of data from image_buffer and store into stego.bmp
        fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

// This function is used to encode the size of secret file extension
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char image_buffer[32];

    // get the extension from secret file
    char *extn = strrchr(encInfo->secret_fname, '.');

    printf("Extension  = %s\n",extn);

    // get the length of extension
    int extn_size = strlen(extn);

     printf("Extension size = %d\n", extn_size);  
    // read the 32 bytes of data from beautiful.bmp and store it into image buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);

    encode_int_to_lsb(extn_size, image_buffer);

    // write the 32 bytes of encoded data into stego.bmp
    fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image);

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
     printf("Encoding extension = %s\n", file_extn);
    char image_buffer[8];

    // Run loop for file extension size
    for (int i = 0; i < strlen(file_extn); i++)
    {
        // read the 8 bytes from beautiful.bmp and store it into image buffer
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        // encode the file extension into image buffer using lsb technique
        encode_byte_to_lsb(file_extn[i], image_buffer);

        // write the 8 bytes of data from image buffer to stego.bmp
        fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

// Function to encode the size of the secret file into the image
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];

    // read 32 bytes from beautiful.bmp and store it into image buffer
    fread(image_buffer, 32, 1, encInfo->fptr_src_image);

    // encode the secret file size into image buffer using lsb technique
    encode_int_to_lsb(file_size, image_buffer);

    // write the 32 bytes of data from image buffer to stego.bmp
    fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char image_buffer[8];
    char data;

    // loop until the entire secret file encoded, read 1 byte from the sectret file
    while (fread(&data, 1, 1, encInfo->fptr_secret) == 1)
    {
        // read the 8 bytes from beautiful.bmp and store into image buffer
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        // encode the secret byte into LSBs of those 8 image bytes
        encode_byte_to_lsb(data, image_buffer);

        // write the modified 8 image bytes of data from image_buffer to stego.bmp
        fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

// Encode a byte into LSB of image data array
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // run loop for 8 times because to store 8 bits
    for (int i = 0; i < 8; i++)
    {
        // clear the lsb of image_buffer
        // extract each bit of secret byte
        //  get 1 bit from the msb of data
        // set the get bit and image buffer and store into image buffer
        image_buffer[i] = image_buffer[i] & ~1 | (data >> (7 - i)) & 1;
    }
    return e_success;
}

Status encode_int_to_lsb(int size, char *image_buffer)
{
    // run loop for 32 times
    for (int i = 0; i < 32; i++)
    {
        // clear the lsb of image buffer
        image_buffer[i] = image_buffer[i] & ~1;

        // get 1 bit from msb of data and shift the get 1 bit to lsb side
        int bit = (size >> (31 - i)) & 1;

        // set the get bit and image buffer and store into image buffer
        image_buffer[i] = image_buffer[i] | bit;
    }
}

// Copy remaining image bytes from src to stego image after encoding
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch; // variable to store 1 byte read from source image

    // read 1 byte at a time from source image until EOF
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        // write the read byte to stego image
        fwrite(&ch, 1, 1, fptr_dest);
    }
}