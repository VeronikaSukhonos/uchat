#include <uchat.h>

char *base64_encode(const unsigned char *data, size_t input_length) {
  static const char base64_table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t output_length = 4 * ((input_length + 2) / 3);
  char *encoded_data = malloc(output_length + 1);
  if (!encoded_data)
    return NULL;

  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;

    uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
    encoded_data[j++] = base64_table[(triple >> 18) & 0x3F];
    encoded_data[j++] = base64_table[(triple >> 12) & 0x3F];
    encoded_data[j++] =
        (i > input_length + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
    encoded_data[j++] = (i > input_length) ? '=' : base64_table[triple & 0x3F];
  }

  encoded_data[output_length] = '\0';
  return encoded_data;
}

// Function to read file and encode it in Base64
char *read_and_encode_file(const char *filepath) {
  FILE *file = fopen(filepath, "rb");
  if (!file)
    return NULL;

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  unsigned char *file_content = malloc(file_size);
  if (!file_content) {
    fclose(file);
    return NULL;
  }

  fread(file_content, 1, file_size, file);
  fclose(file);

  char *encoded_content = base64_encode(file_content, file_size);
  free(file_content);
  return encoded_content;
}
