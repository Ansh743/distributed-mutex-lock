#include "helper.h"

void printsin(struct sockaddr_in *sin, char *m1, char *m2)
{
    printf("%s %s:\n", m1, m2);
    printf("  family %d, addr %s, port %d\n", sin->sin_family,
           inet_ntoa(sin->sin_addr), ntohs((unsigned short)(sin->sin_port)));
}

char *get_file_path()
{
    char *home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        perror("get_file_path: HOME environment variable not set");
        return NULL;
    }

    // Allocate memory for the file path string
    char *file_path = malloc(strlen(home_dir) + strlen(FILE_NAME) + 1);
    if (file_path == NULL)
    {
        perror("get_file_path: Failed to allocate memory for file path");
        return NULL;
    }

    // Copy the home directory path and the file name into the file path string
    strcpy(file_path, home_dir);
    strcat(file_path, FILE_NAME);

    return file_path;
}

char *write_ip_to_file(char *filename)
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *ip_address;

    if (getifaddrs(&ifap) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    FILE *file = fopen(filename, "a+");
    if (file == NULL)
    {
        perror("fopen");
        return NULL;
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            ip_address = inet_ntoa(sa->sin_addr);

            // Ignore loopback address
            if (strcmp(ip_address, "127.0.0.1") != 0)
            {
                fprintf(file, "%s\n", ip_address);
                break;
            }
        }
    }

    fclose(file);
    freeifaddrs(ifap);
    return ip_address;
}



int read_ip_from_file(const char* filename, char** strings) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    char buf[MAX_STR_LEN + 1];
    int count = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL && count < NUM_HOSTS) {
        if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n') {
            buf[strlen(buf)-1] = '\0';
        }
        strings[count] = strdup(buf);
        if (!strings[count]) {
            perror("Error allocating memory for string");
            fclose(fp);
            return -1;
        }
        count++;
    }

    fclose(fp);
    return count;
}