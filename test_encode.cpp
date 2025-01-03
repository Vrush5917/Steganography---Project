#include <iostream>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <iomanip>
#include <cstring>
using namespace std;

int main(int argc, char **argv)
{
    if (check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        cout << "Selected Encoding" << endl;
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            cout << "Read and validate arguements successfully" << endl;
            if (do_encoding(&encInfo) == e_success)
            {
                cout << "Encoding completed successfully" << endl;
            }
            else
            {
                cout << "Failed to encode the data" << endl;
            }
        }
        else
        {
            cout << "Failed to read and validate arguements" << endl;
        }
        
    }
    else if (check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        cout << "Selected Decoding" << endl;
        if(decode_read_and_validate_encode_args(argv, &decInfo) == e_success)
        {
             cout << "Read and validate arguement is successfull" << endl;
             if (do_decoding(&decInfo) == e_success)
             {
                cout << "Decoding completed successfully" << endl;
             }
             else
             {
                cout << "Failed to decode" << endl;
             }
        }
        else
        {
            cout << "Failed to read and validate the arguement" << endl;
        }
        
    }
    else
    {
        cout << "Invalid Option\n" << endl;
        cout << "--------------------Usage of the project--------------------" << endl;
        cout << "For Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp" << endl;
        cout << "For Decoding : ./a.out -d beuatiful.bmp secret.txt" << endl;
        cout << "------------------------------------------------------------" << endl;
    }
    return 0;
}

OperationType check_operation_type(char **argv)
{
    if (strcmp(argv[1], "-e") == 0)
    {
        
        return e_encode;
        
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
