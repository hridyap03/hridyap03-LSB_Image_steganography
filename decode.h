#include <stdio.h>
#include <string.h>
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ext = strstr(argv[2], ".bmp");

    if (ext != NULL && (strcmp(ext, ".bmp")) == 0)
    {

        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf(".bmp not present\n");
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        char *temp = strtok(argv[3], ".");
        strcpy(decInfo->output_file_name, argv[3]);
    }
    else
    {
        strcpy(decInfo->output_file_name, "output");
    }

    return e_success;
}
// Status open_decode_files(DecodeInfo *decInfo)
// {

// // Stego Image file
// decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
// // Do Error handling
// if (decInfo->fptr_stego_image == NULL)
// {
//     perror("fopen");
//     fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

//     return e_failure;
// }

// No failure return e_success
// return e_success;
// output file
// decInfo->fptr_output_txt = fopen(decInfo->output_file_name, "w");
//  Do Error handling
//  if (decInfo->fptr_output_txt== NULL)
//  {
//      perror("fopen");
//      fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_file_name);

//     return e_failure;
// }

// }

Status skip_bmp_header(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
    return e_success;
}

Status decode_magic_string_size(int *size, DecodeInfo *decInfo)
{
    if (decode_size_from_lsb(size, decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}

Status decode_magic_string(char *magic_string, DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->magic_string_size; i++)
    {
        decode_byte_from_lsb(&magic_string[i], decInfo->fptr_stego_image);
    }

    char magicstring[20];
    printf("Enter the magic string : ");
    scanf("%s", magicstring);

    if (strcmp(magic_string, magicstring) != 0)
    {
        printf("Magic string mismatch\n");
        return e_failure;
    }
    return e_success;
}
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    if (decode_size_from_lsb(size, decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->extn_secret_file_size; i++)
    {
        decode_byte_from_lsb(&decInfo->extn_secret_file[i], decInfo->fptr_stego_image);
    }

    strcat(decInfo->output_file_name, decInfo->extn_secret_file);
    return e_success;
}

Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    int temp;
    if (decode_size_from_lsb(&temp, decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    *file_size = temp;
    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    decInfo->fptr_output_txt = fopen(decInfo->output_file_name, "wb");
    if (decInfo->fptr_output_txt == NULL)
    {
        perror("fopen");
        fprintf(stderr, "Error:Unable to open %s\n", decInfo->output_file_name);
        return e_failure;
    }
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        char ch;
        decode_byte_from_lsb(&ch, decInfo->fptr_stego_image);
        fwrite(&ch, 1, 1, decInfo->fptr_output_txt);
    }
    return e_success;
}
Status decode_byte_from_lsb(char *data, FILE *image_buffer)
{
    unsigned char buffer;
    *data = 0;
    for (int i = 0; i < 8; i++)
    {
        fread(&buffer, 1, 1, image_buffer);
        int bit = buffer & 1;
        *data = (*data << 1) | bit;
    }
    return e_success;
}
Status decode_size_from_lsb(int *size, FILE *imageBuffer)
{
    unsigned char buffer;
    *size = 0;
    for (int i = 0; i < 32; i++)
    {
        fread(&buffer, 1, 1, imageBuffer);
        int bit = buffer & 1;
        *size = (*size << 1) | bit;
    }
    return e_success;
}
Status do_decoding(DecodeInfo *decInfo)
{

    if (skip_bmp_header(decInfo) == e_failure)
    {
        printf("ERROR : Skipping the header bytes unsuccessful\n");
        return e_failure;
    }
    printf("Skipped bmp header successfully\n");
    // return e_success;

    if (decode_magic_string_size(&decInfo->magic_string_size, decInfo) == e_failure)
    {
        printf("ERROR : Decoding magic string size unsuccessful\n");
        return e_failure;
    }
    printf("Decoded magic string size successfully\n");
    // return e_success;

    if (decode_magic_string(decInfo->magic_string, decInfo) == e_failure)
    {
        printf("ERROR : Decoding magic string unsuccessful\n");
        return e_failure;
    }
    printf("Decoded magic string successfully\n");
    // return e_success;

    if (decode_secret_file_extn_size(&decInfo->extn_secret_file_size, decInfo) == e_failure)
    {
        printf("ERROR : Decoding secret file extention size unsuccessful\n");
        return e_failure;
    }
    printf("Decoded secret file extention size successfully\n");
    // return e_success;

    if (decode_secret_file_extn(decInfo->extn_secret_file, decInfo) == e_failure)
    {
        printf("ERROR : Decoding secret file extention unsuccessful\n");
        return e_failure;
    }
    printf("Decoded secret file extention successfully\n");
    // return e_success;
    if (decode_secret_file_size(&decInfo->size_secret_file, decInfo) == e_failure)
    {
        printf("ERROR : Decoding secret file size unsuccessful\n");
        return e_failure;
    }
    printf("Decoded secret file size successfully\n");
    // return e_success;
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("Decoding secret data unsuccessful\n");
        return e_failure;
    }
    printf("Decoded secret data successfully\n");
    // return e_success;

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output_txt);

    return e_success;
}