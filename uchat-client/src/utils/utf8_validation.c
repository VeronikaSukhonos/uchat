#include <uchat.h>

gboolean ensure_valid_utf8(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        return FALSE;
    }

    // First check if input is valid UTF-8
    if (g_utf8_validate(input, -1, NULL)) {
        g_strlcpy(output, input, output_size);
        return TRUE;
    }

    // If not valid UTF-8, try to convert from current locale
    GError *error = NULL;
    gchar *utf8_str = g_locale_to_utf8(input, -1, NULL, NULL, &error);
    
    if (error) {
        // If conversion fails, create a sanitized version
        gchar *sanitized = g_strescape(input, NULL);
        g_strlcpy(output, sanitized, output_size);
        g_free(sanitized);
        g_error_free(error);
        return FALSE;
    }

    g_strlcpy(output, utf8_str, output_size);
    g_free(utf8_str);
    return TRUE;
}
