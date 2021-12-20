#include "custombase64.h"
#include "Arduino.h"

void base64_encode_P(unsigned char* input, unsigned char* output, unsigned int input_length, unsigned int* output_length){
    // Write base64 encoded characters to a pointer

    // Stop if input length is too small
    if(input_length <= 0){
        *output_length = 0;
        return;
    }

    // initialize character table
    // 0-25 - A-Z = 65-90
    // 26-51 - a-z = 97-122
    // 52-61 - 0-9 = 48-57
    char char_table[64];
    for (uint i = 0; i<26; i++){
        char_table[i] = 65+i;
    }
    for (uint i = 26; i<52; i++){
        char_table[i] = 97+i-26;
    }
    for (uint i = 52; i<62; i++){
        char_table[i] = 48+i-52;
    }
    char_table[62] = '+';
    char_table[63] = '/';

    // read 3 bytes at a time, write 4

    uint padding = 0;
    unsigned char* curr_byte = input;
    // find bytes left to read
    int bytes_left = input_length - ((int)curr_byte - (int)input);
    unsigned char* output_location = output;
    *output_length = 0;

    // Loop while there is data left to convert
    while (bytes_left > 0){
        byte in_bytes[3]; 
        // Read input bytes, increment current reading location
        if (bytes_left >= 3){
            in_bytes[0] = *(unsigned char*)(curr_byte++);
            in_bytes[1] = *(unsigned char*)(curr_byte++);
            in_bytes[2] = *(unsigned char*)(curr_byte++);
            padding = 0;
        }
        else if (bytes_left == 2){
            in_bytes[0] = *(unsigned char*)(curr_byte++);
            in_bytes[1] = *(unsigned char*)(curr_byte++);
            in_bytes[2] = 0;
            padding = 1;
        }
        else{
            in_bytes[0] = *(unsigned char*)(curr_byte++);
            in_bytes[1] = 0;
            in_bytes[2] = 0;
            padding = 2;
        }

        // Split 3 (8 bit) in-bytes into 4 (6 bit) out-bytes
        byte out_bytes[4];
        out_bytes[0] = (in_bytes[0] & 0b11111100)>>2;
        out_bytes[1] = ((in_bytes[0] & 0b00000011) << 4) | ((in_bytes[1] & 0b11110000) >> 4);
        out_bytes[2] = ((in_bytes[1] & 0b00001111) << 2) | ((in_bytes[2] & 0b11000000) >> 6);
        out_bytes[3] = in_bytes[2] & 0b00111111;

        // Write base64 data
        for (int i=0; i<4; i++){
            unsigned char out_char = char_table[out_bytes[i]];
            // Check if current char should be padding
            if (padding == 2 && (i==2 || i==3)){
                out_char = '=';
            }
            else if (padding == 1 && i==3){
                out_char = '=';
            }
            // Write to current output buffer location
            *output = out_char;
            output++;
        }
        // Increment output length counter
        *output_length += 4;
        // Update bytes left to convert
        bytes_left = input_length - ((int)curr_byte - (int)input);
    }

    // Null-terminate string
    *output = _NULL;
}