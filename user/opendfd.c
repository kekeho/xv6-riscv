#include "kernel/types.h"
#include "kernel/fs.h"
#include "user/user.h"
#include "kernel/stat.h"

int main(int argc, const char *argv[]) {
    int fd;
    uint16 n;
    n = opendfd();
    printf("%d\n", n);  // 7 = 0000 0000 0000 0111

    fd = open(".", 0);
    printf("open: %d\n", fd);
    n = opendfd();
    printf("%d\n", n);  // 15 = 0000 0000 0000 1111

    close(fd);
    n = opendfd();
    printf("%d\n", n);  // 7
}
