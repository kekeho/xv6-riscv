// 6/17 課題
// Hiroki TAKEMURA (72144403)
// for xv6

// ..を開いていけば、OK

#include "kernel/types.h"
// #include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"
#include "kernel/stat.h"



#define MAX_PATH 1024


char* strcat(char *dest, char *src) {
	int	dest_len;
	int	src_len;

	dest_len = strlen(dest);
    src_len = strlen(src);
	dest += dest_len;
	while (*src != '\0')
		*dest++ = *src++;
	*dest = '\0';
	return (dest - src_len - dest_len);
}

void get_full_path(char *buf, int size) {
    char path[MAX_PATH] = "";
    int fd, parent_fd;
    struct dirent de;
    struct stat st, parent_st;


    fd = open(".", 0);
    if (fd < 0) {
        fprintf(2, "open: error\n");
        exit(1);
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "stat: error\n");
        close(fd);
        exit(1);
    }

    while (1) {

        parent_fd = open("..", 0);
        if (parent_fd < 0) {
            fprintf(2, "open parent: error\n");
            close(fd);
            exit(1);
        }

        if (fstat(parent_fd, &parent_st) < 0) {
            fprintf(2, "stat parent: error\n");
            close(fd);
            close(parent_fd);
            exit(1);
        }

        if (st.ino == parent_st.ino) {
            // root?
            break;
        }

        while (read(parent_fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == st.ino) {
                if (strlen(path) > 0) {
                    char temp[MAX_PATH];
                    strcpy(temp, path);
                    strcpy(path, "/");
                    strcat(path, de.name);
                    strcat(path, temp);
                } else {
                    strcpy(path, "/");
                    strcat(path, de.name);
                }
                break;
            }
        }

        close(fd);
        fd = parent_fd;
        st = parent_st;
    }

    if (strlen(path) == 0) {
        strcpy(path, "/");
    }

    close(fd);

    if (strlen(path) < size) {
        strcpy(buf, path);
    } else {
        fprintf(2, "path is too long\n");
        exit(1);
    }
}



int main(void) {
    char buf[MAX_PATH];

    get_full_path(buf, sizeof(buf));
    printf("%s\n", buf);

    exit(0);
}
