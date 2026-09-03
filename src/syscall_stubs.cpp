// Silence linker warnings
extern "C" {
    __attribute__((weak)) int _close(int file) { return -1; }
    __attribute__((weak)) int _lseek(int file, int ptr, int dir) { return 0; }
    __attribute__((weak)) int _read(int file, char *ptr, int len) { return 0; }
    __attribute__((weak)) int _write(int file, char *ptr, int len) { return len; }
    __attribute__((weak)) int _fstat(int file, void *st) { return 0; }
    __attribute__((weak)) int _isatty(int file) { return 1; }
    __attribute__((weak)) int _getpid(void) { return 1; }
    __attribute__((weak)) int _kill(int pid, int sig) { return -1; }
}
