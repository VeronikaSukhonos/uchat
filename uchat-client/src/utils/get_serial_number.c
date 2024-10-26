#include <uchat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

void get_serial_number(char *serial, size_t len) {
  HKEY hKey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0,
                   KEY_READ, &hKey) == ERROR_SUCCESS) {
    DWORD type = REG_SZ;
    DWORD size = len;
    RegQueryValueEx(hKey, "MachineGuid", NULL, &type, (BYTE *)serial, &size);
    RegCloseKey(hKey);
  } else {
    snprintf(serial, len, "Unknown");
  }
}

#elif defined(__APPLE__)
#include <stdlib.h>

void get_serial_number(char *serial, size_t len) {
  FILE *fp = popen(
      "ioreg -l | grep IOPlatformSerialNumber | awk '{print $4}' | tr -d '\"'",
      "r");
  if (fp) {
    fgets(serial, len, fp);
    pclose(fp);
  } else {
    snprintf(serial, len, "Unknown");
  }
}

#elif defined(__linux__)
#include <stdlib.h>

void get_serial_number(char *serial, size_t len) {
  FILE *file = fopen("/etc/machine-id", "r");
  if (file) {
    fgets(serial, len, file);
    fclose(file);
  } else {
    snprintf(serial, len, "Unknown");
  }
}

#else
void get_serial_number(char *serial, size_t len) {
  snprintf(serial, len, "Unsupported OS");
}
#endif
