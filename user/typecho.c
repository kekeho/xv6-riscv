// 6/10 課題1
// Hiroki TAKEMURA (72144403)
// for xv6

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    char buf[8];
    int n;
    while((n = read(0, buf, sizeof(buf))) > 0) {
        write(1, buf, n);
    }
    exit(0);
}
