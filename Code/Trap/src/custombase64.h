#ifndef custombase64_h
#define custombase64_h

#ifdef __cplusplus
 extern "C" {
#endif

void base64_encode_P(unsigned char* input, unsigned char* output, unsigned int input_length, unsigned int* output_length);

#ifdef __cplusplus
}
#endif
#endif