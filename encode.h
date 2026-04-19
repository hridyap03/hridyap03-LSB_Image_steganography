#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

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

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
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

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{

    char *ret = strstr(argv[2], ".bmp");

    if (ret != NULL && (strcmp(ret, ".bmp")) == 0)
    {

        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf(".bmp file not present\n");
        return e_failure;
    }

    char *ret1 = strstr(argv[3], ".txt");
    if (ret1 != NULL && (strcmp(ret1, ".txt")) == 0)
    {

        encInfo->secret_fname = argv[3];
        strcpy(encInfo->extn_secret_file, ret1);
    }
    else
    {
        printf(".txt file not present\n");
        return e_failure;
    }

    if (argv[4] != NULL)
    {
        char *ret2 = strstr(argv[4], ".bmp");
        if (ret2 != NULL && (strcmp(ret2, ".bmp")) == 0)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
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
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
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

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    long capacity = (4 * 8 + 2 * 8 + 4 * 8 + 4 * 8 + 4 * 8 + (encInfo->size_secret_file * 8));
    long image_capacity_bits = encInfo->image_capacity * 8;
    if (image_capacity_bits < capacity)
    {
        printf("Image doesn't have the capacity\n");
        return e_failure;
    }
    return e_success;

    // get the image size from get_image_size_for_bmp()
    // image_cpacity= get_image_size_for_bmp()
    // do the same for secret file also
    // check the image size is greater than(4*8+2*8+4*8+4*8+4*8+(size of secret data*8))
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{

    unsigned char buffer[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(buffer, 1, 54, fptr_src_image);
    fwrite(buffer, 1, 54, fptr_dest_image);
    return e_success;
}
Status encode_magic_string_size(int size, EncodeInfo *encInfo)
{
    unsigned char image_buffer[32];

    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    if (encode_size_to_lsb(size, image_buffer) == e_success)
    {
        fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    }
    else
    {
        return e_failure;
    }

    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    unsigned char img_buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(img_buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], img_buffer);
        fwrite(img_buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    unsigned char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    unsigned char image_buffer[8];
    for (int i = 0; i < strlen(encInfo->extn_secret_file); i++)
    {
        fread(image_buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], image_buffer);
        fwrite(image_buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    unsigned char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, image_buffer);
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    unsigned char buffer[8];
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        char ch = getc(encInfo->fptr_secret);
        if (encode_byte_to_lsb(ch, buffer) == e_success)
        {
            fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
        }
        else
        {
            return e_failure;
        }
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    unsigned char ch;
    while (fread(&ch, 1, 1, fptr_src) != 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        char bit = (data >> (7 - i) & 1);
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        int bit = (size >> (31 - i) & 1);
        imageBuffer[i] = (imageBuffer[i] & 0xFE) | bit;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR : Files opened successfully\n");
        return e_failure;
    }
    printf("Opened files successfully\n");
    if (check_capacity(encInfo) == e_failure)
    {
        printf("Error : check capacaity unsuccessful\n");
        return e_failure;
    }
    printf("Check capacity successful\n");
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Copying bmp header unsuccessful\n");
        return e_failure;
    }
    printf("Copied bmp header successfully\n");
    if (encode_magic_string_size(strlen(MAGIC_STRING), encInfo) == e_failure)
    {
        printf("ERROR : Encoding magic string size unsuccessful\n");
        return e_failure;
    }
    printf("Encoded magic string size successfully\n");
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR : Encoding magic string unsuccessful\n");
        return e_failure;
    }
    printf("Encoded magic string successfully\n");
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure)
    {
        printf("ERROR : Encoding size of secret file extension unsuccessful\n");
        return e_failure;
    }
    printf("Encoded size of secret file extension successfully\n");
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR : Encoding secret file extension unsuccessful\n");
        return e_failure;
    }
    printf("Encoded secret file extension successfully\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR : Encoding secret file size unsuccessful\n");
        return e_failure;
    }
    printf("Encoded secret file size succesfully\n");
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR : Encoding secret file unsuccessful\n");
        return e_failure;
    }
    printf("Encoded secret file successfully\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR : Copying remaining image data unsuccessful\n");
        return e_failure;
    }
    printf("Encoded remaining image data successfully\n");

    return e_success;

    // return e_failure
    // call check_capacity()==
    // call copy_bmp_header()==
}
