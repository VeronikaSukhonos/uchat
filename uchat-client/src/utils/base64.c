#include <uchat.h>

static const unsigned char base64_dec_table[256] = {
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
    ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
    ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
    ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
    ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
    ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
    ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63};

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
  printf("Opening file: %s\n", filepath);
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    perror("File open failed");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  if (file_size == 0) {
    fprintf(stderr, "File is empty\n");
    fclose(file);
    return NULL;
  }
  rewind(file);

  unsigned char *file_content = malloc(file_size);
  if (!file_content) {
    fprintf(stderr, "Memory allocation failed for file content\n");
    fclose(file);
    return NULL;
  }

  size_t read_size = fread(file_content, 1, file_size, file);
  if (read_size != file_size) {
    fprintf(stderr, "Failed to read the file completely\n");
    free(file_content);
    fclose(file);
    return NULL;
  }
  fclose(file);

  printf("File read successfully, size: %ld bytes\n", file_size);

  char *encoded_content = base64_encode(file_content, file_size);
  free(file_content);

  if (!encoded_content || strlen(encoded_content) == 0) {
    fprintf(stderr, "Base64 encoding failed or returned empty string\n");
    return NULL;
  }

  return encoded_content;
}

unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {
  if (input_length % 4 != 0)
    return NULL;

  *output_length = input_length / 4 * 3;
  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data)
    return NULL;

  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a =
        data[i] == '=' ? 0 & i++ : base64_dec_table[(unsigned char)data[i++]];
    uint32_t sextet_b =
        data[i] == '=' ? 0 & i++ : base64_dec_table[(unsigned char)data[i++]];
    uint32_t sextet_c =
        data[i] == '=' ? 0 & i++ : base64_dec_table[(unsigned char)data[i++]];
    uint32_t sextet_d =
        data[i] == '=' ? 0 & i++ : base64_dec_table[(unsigned char)data[i++]];

    uint32_t triple =
        (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

    if (j < *output_length)
      decoded_data[j++] = (triple >> 16) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = triple & 0xFF;
  }

  return decoded_data;
}

// Function to decode Base64 and save to file (pseudo-code, replace with actual
// decode function)
void save_decoded_file(const char *encoded_data, const char *output_file) {
  size_t decoded_length;
  unsigned char *decoded_data =
      base64_decode(encoded_data, strlen(encoded_data), &decoded_length);
  // Use your base64 decode function here

  FILE *file = fopen(output_file, "wb");
  fwrite(decoded_data, 1, decoded_length, file);
  fclose(file);

  free(decoded_data);
}
