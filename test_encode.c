/*
NAME: Habeebazainab Jagirdar
DATE: 29-06-2026
DESCRIPTION:Developed a C-based steganography application to hide and retrieve secret files inside BMP images
            using the Least Significant Bit (LSB) technique. Implemented encoding and decoding of magic string, 
            file extension, file size, and secret data. 
*/

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include <string.h>
#include "types.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: ./a.out -e/-d <files>\n");
        return 0;
    }

    int ret = check_operation_type(argv);

    // check the ret is e_encode or not
    if (ret == e_encode)
    {
        EncodeInfo encInfo;
        // check argc will be 4 or 5
        if (argc != 4 && argc != 5)
        {
            printf("Invalid arguments\n");
            return 0;
        }

        // if the read and validate encode args function returns e_failure then print error
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("Error\n"); // printf error message and return 0
            return 0;
        }

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("Encoding Failed\n"); // printf error message and return 0
            return 0;
        }

        printf("Encoding Done Successfully!!!\n"); // print encoding done succesfully
    }

    // perform decode function
    else if (ret == e_decode)
    {
        DecodeInfo decInfo;

        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("Decoding arguments invalid\n");
            return 0;
        }

        if (do_decoding(&decInfo) == e_failure)
        {
            printf("Decoding Failed\n");//print error message
            return 0;
        }

        printf("Decoding Done Successfully!!!\n");//print decoding done message
    
}

else
{
    printf("Error\n"); // print error terminate the program
    return 0;
}
return 0;
}

OperationType check_operation_type(char *argv[])
{
    // check the argv[1] is -e or not
    if (strcmp(argv[1], "-e") == 0)
        return e_encode; // if yes return e_encode

    // check the argv[1] is -d or not
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode; // if yes return e_decode

    else
        return e_unsupported; // else return e_unsupported
}