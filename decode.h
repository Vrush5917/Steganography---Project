#ifndef DECODE_H
#define DECODE_H


#include "types.h" // Contains user defined types
#include "common.h"  // Contains common functions and macros
#include <cstring>

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
   /* Encoded stego image info */
	char *stego_image_fname;
	FILE *fptr_stego_image;
	uint image_data_size;
	char image_data[MAX_IMAGE_BUF_SIZE];

	/* Decoded output file info */
	char *output_file_name;
	FILE *fptr_output_file;
	char extn_output_file[MAX_FILE_SUFFIX];
	char decode_data[MAX_SECRET_BUF_SIZE];
    long op_size_secret_file;
    char decode_magic_string[30];
    uint decode_image_size;
    char decode_secret_file_extn[strlen(".txt")];
    char passcode[30] = {"My password is SECRET ;)"};

    /*Ouput text File*/
    const char *final_op_name;
    FILE *fptr_op_name;

} DecodeInfo;


/* Decoding function prototype */


/* Read and validate Encode args from argv */
Status decode_read_and_validate_encode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status decode_open_files(DecodeInfo *decInfo);

/* check capacity */
Status decode_check_capacity(DecodeInfo *decInfo);

/* Get image size */
uint decode_get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint decode_get_file_size(FILE *fptr);

/* Copy bmp image header */
Status decode_copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Encode secret file extenstion */
Status decode_secret_file_extn(const char *file_extn, DecodeInfo *decInfo);

/* Encode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo);

/* Encode secret file data*/
Status encode_secret_file_data(DecodeInfo *decInfo);

/* Encode function, which does the real encoding */
Status decode_data_to_image(const char *data, int size, FILE *fptr_stego_image, DecodeInfo *decInfo);

/* Encode a byte into LSB of image data array */
Status decode_byte_to_lsb(char *image_buffer, char *decode_magic_string);

/* Copy remaining image bytes from src to stego image after encoding */
Status decode_copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode secret file extenstion size*/
Status decode_size(int size, FILE *fptr_stego_image, DecodeInfo *decInfo);

/*Encode size to LSB*/
Status decode_size_to_lsb(char *image_buffer, DecodeInfo *decInfo);

#endif
