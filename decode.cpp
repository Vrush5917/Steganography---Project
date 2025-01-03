#include <iostream>
#include "decode.h"
#include "types.h"
#include <iomanip>
#include <cstring>
#include "common.h"
#include <cstring>
using namespace std;

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint decode_get_image_size_for_bmp(FILE *fptr_image)
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

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status decode_open_files(DecodeInfo *decInfo)
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

    // Secret file
    decInfo->fptr_output_file = fopen(decInfo->output_file_name, "r");
    // Do Error handling
    if (decInfo->fptr_output_file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_file_name);

        return e_failure;
    }

    // Final output txt file
    decInfo->fptr_op_name = fopen(decInfo->final_op_name, "w+");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->final_op_name);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status decode_read_and_validate_encode_args(char **argv, DecodeInfo *decInfo)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        decInfo->output_file_name = argv[3];
    }
    else
    {
        return e_failure;
    }

    if (argv[4] != NULL)
    {
        decInfo->final_op_name = argv[4];
    }
    else
    {
        decInfo->final_op_name = "Output.txt";
    }
    return e_success;
}

Status decode_check_capacity(DecodeInfo *decInfo)
{
    decInfo->image_data_size = decode_get_image_size_for_bmp(decInfo->fptr_stego_image);
    decInfo->op_size_secret_file = decode_get_file_size(decInfo->fptr_output_file);
    if (decInfo->image_data_size > (54 + (2 + 4 + 4 + 4 + decInfo->op_size_secret_file) * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

uint decode_get_file_size(FILE *fptr_output_file)
{
    fseek(fptr_output_file, 0, SEEK_END);
    return ftell(fptr_output_file);
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    // Initialize decode_magic_string buffer to zero
    memset(decInfo->decode_magic_string, 0, strlen(magic_string) + 1);

    // Set the file pointer to the start of the image data (after the BMP header)
    rewind(decInfo->fptr_stego_image);
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // Decode the magic string from the image
    decode_data_to_image(magic_string, strlen(magic_string), decInfo->fptr_stego_image, decInfo);

    // Compare the decoded string with the expected magic string
    if (strcmp(magic_string, decInfo->decode_magic_string) == 0)
    {
        cout << "Decoded magic string is " << decInfo->decode_magic_string << endl;
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status decode_size(int size, FILE *fptr_stego_image, DecodeInfo *decInfo)
{
    // to read 32 bytes of rgc data from stego.bmp
    char str[32];
    fread(str, 32, sizeof(char), fptr_stego_image);
    // Reusable function to encode the size
    decode_size_to_lsb(str, decInfo);
    long a = static_cast<long>(size);
    long b = static_cast<long>(decInfo->decode_image_size);
    if (a == b)
    {
        cout << "Decode size is " << b << endl;
    }
    else
    {
        return e_failure;
    }
    return e_success;
}



Status decode_secret_file_extn(const char *file_extn, DecodeInfo *decInfo)
{

    file_extn = ".txt";
    decode_data_to_image(file_extn, strlen(file_extn), decInfo->fptr_stego_image, decInfo);
    if (strncmp(decInfo->decode_magic_string, ".txt", 4) == 0)
    {
        cout << "The secret file extension is " << decInfo->decode_magic_string << endl;
    }
    else
    {
        return e_failure;
    }
    return e_success;
}

Status decode_data_to_image(const char *data, int size, FILE *fptr_stego_image, DecodeInfo *decInfo)
{
    // Decode the data from the image until the size of the string
    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the image (which holds one byte of encoded data)
        fread(decInfo->image_data, sizeof(char), 8, fptr_stego_image);

        // Decode the byte from the LSBs of the image data
        decode_byte_to_lsb(decInfo->image_data, decInfo->decode_magic_string + i);
    }
    return e_success;
}



Status decode_secret_file_size(long file_size, DecodeInfo *decInfo)
{
    // to read 32 bytes of rgc data from beautiful.bmp
    char str[32];
    fread(str, 32, sizeof(char), decInfo -> fptr_stego_image);
    // Reusable function to encode the size
    decode_size_to_lsb(str, decInfo);
    long a = static_cast<long>(file_size);
    long b = static_cast<long>(decInfo->decode_image_size);
    if (a == b)
    {
        cout << "Decoded secret_file_size is " << b << endl;
    }
    else
    {
        return e_failure;
    }
    
    return e_success;
}

Status decode_size_to_lsb(char *image_buffer, DecodeInfo *decInfo)
{
    // Initialize the size value
    unsigned int decoded_size = 0;

    // Extract the least significant bit from each byte
    for (int i = 0; i < 32; i++)
    {
        decoded_size |= ((image_buffer[i] & 0x01) << (31 - i));
    }

    // Store the decoded size
    decInfo->decode_image_size = decoded_size;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    char decoded_data[256]; // Adjust size as needed
    int index = 0;

    // Bring the file pointer to the starting position
    fseek(decInfo->fptr_output_file, 0, SEEK_SET);

    for (int i = 0; i < decInfo->op_size_secret_file; i++)
    {
        // Read 8 bytes of RGB from stego image
        fread(decInfo->image_data, 8, sizeof(char), decInfo->fptr_stego_image);
        // Read a character from the secret file
        fread(&ch, sizeof(char), 1, decInfo->fptr_output_file);
        // Decode the byte from the image data
        decode_byte_to_lsb(decInfo->image_data, &decoded_data[index]);
        index++;
    }

    decoded_data[index] = '\0'; // Null-terminate the string

    // Check if the decoded data matches the expected passcode
    if (memcmp(decoded_data, decInfo->passcode, strlen(decInfo->passcode)) == 0)
    {
        cout << "The Decoded secret file data is :" << decoded_data << endl;
        fseek(decInfo->fptr_output_file, 0, SEEK_SET);
        fseek(decInfo->fptr_op_name, 0, SEEK_SET);
        char ch;
        while((ch = fgetc(decInfo -> fptr_output_file)) != EOF)
        {
            fputc(ch, decInfo -> fptr_op_name);
        }
    }
    else
    {
        return e_failure;
    }

    return e_success;
}


Status decode_byte_to_lsb(char *image_buffer, char *decode_magic_string)
{
    char decoded_char = 0;

    for (int i = 0; i < 8; i++)
    {
        // Extract the LSB from each byte in the image buffer and set it in the correct position in the decoded byte
        decoded_char |= ((image_buffer[i] & 0x01) << (7 - i));
    }

    // Store the decoded character in the output string
    *decode_magic_string = decoded_char;

    return e_success;
}



Status do_decoding(DecodeInfo *decInfo)
{
    if (decode_open_files(decInfo) == e_success)
    {
        cout << "Successfully opened the files" << endl;
        cout << "Started decoding" << endl;
        if (decode_check_capacity(decInfo) == e_success)
        {
            cout << "Check capacity function is successfull" << endl;
            if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
            {
                cout << "Decoded Magic string successfully" << endl;
                if (decode_size(strlen(".txt"), decInfo->fptr_stego_image, decInfo) == e_success)
                {
                    cout << "Successfully decode the size" << endl;
                    if (decode_secret_file_extn(decInfo->output_file_name, decInfo) == e_success)
                    {
                        cout << "Successfully decoded the secret file extn" << endl;
                        if (decode_secret_file_size(decInfo->op_size_secret_file, decInfo) == e_success)
                        {
                            cout << "Successfully decoded the secret file size" << endl;
                            if (decode_secret_file_data(decInfo) == e_success)
                            {
                                cout << "Successfully decoded the data" << endl;
                            }
                            else
                            {
                                cout << "Failed to decode the data" << endl;
                                return e_failure;
                            }
                        }
                        else
                        {
                            cout << "Failed to decode the secret file size" << endl;
                            return e_failure;
                        }
                    }
                    else
                    {
                        cout << "Failed to decode the secret file extn" << endl;
                        return e_failure;
                    }
                }
                else
                {
                    cout << "Failed to decode the size" << endl;
                    return e_failure;
                }
            }
            else
            {
                cout << "Failed to decode the magic string" << endl;
                return e_failure;
            }
        }
        else
        {
            cout << "Failed to check the capacity" << endl;
            return e_success;
        }
    }
    else
    {
        cout << "Failed to open the files";
        return e_failure;
    }
    return e_success;
}
