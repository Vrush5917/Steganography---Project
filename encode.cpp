#include <iostream>
#include "encode.h"
#include "types.h"
#include <iomanip>
#include <cstring>
#include "common.h"
using namespace std;

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

Status read_and_validate_encode_args(char **argv, EncodeInfo *encInfo)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if (encInfo->image_capacity > (54 + (2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char header[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_stego_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned int mask = 1 << 7;
    for (int i = 0; i < 8; i++)
    {
        // data and mask will give you a bit from msb of data, after that bring the data to lsb by right shift, clear lsb bit and or them to get encoded data
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    // Fetch 8 bytes of data untill the size of the string
    for (int i = 0; i < size; i++)
    {
        // read 8bytes from beautiful.bmp
        fread(encInfo->image_data, 8, sizeof(char), fptr_src_image);
        // Call encode_byte_to_lsb
        encode_byte_to_lsb(data[i], encInfo->image_data);
        // after encoding write the encoded data to stego.bmp
        fwrite(encInfo->image_data, 8, sizeof(char), fptr_stego_image);
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // every string data encoding needs to call encode_data_to_image function
    encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for (int i = 0; i < 32; i++)
    {
        // data and mask will give you a bit from msb of data, after that bring the data to lsb by right shift, clear lsb bit and or them to get encoded data
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // to read 32 bytes of rgc data from beautiful.bmp
    char str[32];
    fread(str, 32, sizeof(char), fptr_src_image);
    // Reusable function to encode the size
    encode_size_to_lsb(str, size);
    fwrite(str, 32, sizeof(char), fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;

}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // to read 32 bytes of rgc data from beautiful.bmp
    char str[32];
    fread(str, 32, sizeof(char), encInfo -> fptr_src_image);
    // Reusable function to encode the size
    encode_size_to_lsb(str, file_size);
    fwrite(str, 32, sizeof(char), encInfo -> fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    //Bring the file pointer to starting position
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    for (int i = 0; i < encInfo -> size_secret_file; i++)
    {
        //read 8 bytes of rgb from beautiful.bmp
        fread(encInfo -> image_data, 8, sizeof(char), encInfo -> fptr_src_image);
        //Read a character from the secret file
        fread(&ch, sizeof(char), 1,  encInfo -> fptr_secret);
        //Call reusable function to encode the data from secret file .txt
        encode_byte_to_lsb(ch, encInfo -> image_data);
        fwrite(encInfo -> image_data, 8, sizeof(char), encInfo -> fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image)
{
  char ch;
  while(fread(&ch, 1, 1, fptr_src_image) > 0)
  {
    fwrite(&ch, 1, 1, fptr_stego_image);
  }
  return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
    // call rest of the encoding function
    if (open_files(encInfo) == e_success)
    {
        cout << "Opened files successfully" << endl;
        cout << "Started Encoding" << endl;
        if (check_capacity(encInfo) == e_success)
        {
            cout << "Check capacity function is successfull" << endl;
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                cout << "Copied the header successfully" << endl;
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    cout << "Encoded Magic string successfully" << endl;
                    if (encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        cout << "Successfully encode secret fle extenson size" << endl;
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            cout << "Successfully encoded the secret file extension" << endl;
                            if (encode_secret_file_size(encInfo -> size_secret_file, encInfo) == e_success)
                            {
                                cout << "Encoded secret file size successfully" << endl;
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    cout << "Encode secret data successfully" << endl;
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        cout << "Successfully copied remaining bytes" << endl;
                                    }
                                    else
                                    {
                                        cout << "Failed to copy the remaining bytes" << endl;
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    cout << "Failed to encode the secret file data" << endl;
                                    return e_failure;
                                }
                            }
                            else
                            {
                                cout << "Failed to encode the secret file size successfully" << endl;
                                return e_failure;
                            }
                        }
                        else
                        {
                            cout << "Failed to encode the secret file extension" << endl;
                            return e_failure;
                        }
                    }
                    else
                    {
                        cout << "Failed to encode secret fle extenson size" << endl;
                        return e_failure;
                    }
                }
                else
                {
                    cout << "Failed to encode the magic string" << endl;
                }
            }
            else
            {
                cout << "Failed to copy the header" << endl;
                return e_failure;
            }
        }
        else
        {
            cout << "Dont have enough capacity to encode the data" << endl;
            return e_failure;
        }
    }
    else
    {
        cout << "Failed to open the files" << endl;
        return e_failure;
    }
    return e_success;
}
