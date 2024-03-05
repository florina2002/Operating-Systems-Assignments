
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

#define MAX 256

typedef struct sectionHeader
{
    char sect_name[14];
    int sect_type;
    int sect_offset;
    int sect_size;

} section_header;

typedef struct fileHeader
{
    char magic[5];
    int header_size;
    int version;
    int no_of_sections;
    section_header *section_headers;
} sf_header;

int rSFheader(int fd, sf_header *sf_header)
{

    off_t offset = 0;
    lseek(fd, offset, SEEK_SET);

    int bytes_to_read;

    lseek(fd, 0, SEEK_SET);

    bytes_to_read = read(fd, &sf_header->magic, 4);
    if (bytes_to_read < 0)
    {
        printf("Error reading file");
        return -1;
    }

    sf_header->header_size = 0;

    bytes_to_read = read(fd, &sf_header->header_size, 2);
    if (bytes_to_read < 0)
    {
        printf("Error reading file");
        return -1;
    }

    sf_header->version = 0;

    bytes_to_read = read(fd, &sf_header->version, 4);
    if (bytes_to_read < 0)
    {
        printf("Error reading file");
        return -1;
    }

    sf_header->no_of_sections = 0;

    bytes_to_read = read(fd, &sf_header->no_of_sections, 1);
    if (bytes_to_read < 0)
    {
        printf("Error reading file");
        return -1;
    }

    sf_header->section_headers = malloc(sf_header->no_of_sections * sizeof(section_header));

    for (int i = 0; i < sf_header->no_of_sections; i++)
    {

        bytes_to_read = read(fd, &sf_header->section_headers[i].sect_name, 13);
        if (bytes_to_read < 0)
        {
            return -1;
        }

        sf_header->section_headers[i].sect_type = 0;

        bytes_to_read = read(fd, &sf_header->section_headers[i].sect_type, 2);
        if (bytes_to_read < 0)
        {
            return -1;
        }

        sf_header->section_headers[i].sect_offset = 0;

        bytes_to_read = read(fd, &sf_header->section_headers[i].sect_offset, 4);
        if (bytes_to_read < 0)
        {
            return -1;
        }

        sf_header->section_headers[i].sect_size = 0;

        bytes_to_read = read(fd, &sf_header->section_headers[i].sect_size, 4);
        if (bytes_to_read < 0)
        {
            return -1;
        }
    }
   
    return 0;
}

char *toArg(char *arg, char *param)
{
    size_t param_len = strlen(param);
    char *value = NULL;

    // iterate over the argument string
    while (*arg != '\0')
    {
        // check if the current position matches the parameter
        if (strncmp(arg, param, param_len) == 0 && arg[param_len] == '=')
        {
            // found the parameter, extract its value
            value = arg + param_len + 1; // skip the parameter and the '='
            break;
        }
        // move to the next token
        arg = strchr(arg, ' '); // skip until the next space
        if (arg == NULL)
        {
            break; // no more tokens
        }
        arg++; // skip the space
    }

    return value != NULL ? value : "";
}

void list(int recursive, int size_filter, int has_perm_write, char *path)
{

    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char *full_path = malloc(MAX * sizeof(char));

    dir = opendir(path);
    if (dir != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                strcpy(full_path, path);
                strcat(full_path, "/");
                strcat(full_path, entry->d_name);

                if (stat(full_path, &statbuf) == 0)
                {
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        if ((((S_IWUSR & statbuf.st_mode) && has_perm_write) || has_perm_write == 0) && (size_filter == 0))
                        {
                            printf("%s\n", full_path);
                        }

                        if (recursive)
                        {
                            list(recursive, size_filter, has_perm_write, full_path);
                        }
                    }
                    else
                    {
                        if (has_perm_write == 1)
                        {
                            if ((S_IWUSR & statbuf.st_mode))
                                printf("%s\n", full_path);
                        }

                        else if (statbuf.st_size > size_filter)
                        {
                            printf("%s\n", full_path);
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
    if (full_path)
     free(full_path);
}

void printSFHeader(sf_header *sf_header)
{
    printf("version=%d\n", sf_header->version);
    printf("nr_sections=%d\n", sf_header->no_of_sections);

    for (int i = 0; i < sf_header->no_of_sections; i++)
    {
        printf("section%d: ", i + 1);
        printf("%s ", sf_header->section_headers[i].sect_name);
        printf("%d ", sf_header->section_headers[i].sect_type);
        printf("%d", sf_header->section_headers[i].sect_size);
        printf("\n");
    }
}

void parse(char *path)
{
    int fd;
    sf_header sf_header;
    int i;
    bool success = true;

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        printf("ERROR\ninvalid path\n");
        success = false;
    }
    else
    {
        // read section file header then check if it is correct
        if (rSFheader(fd, &sf_header) < 0)
        {

            printf("ERROR\nfailed to read section file header\n");
            success = false;
        }
        else if (!(strcmp(sf_header.magic, "3zN4") == 0))
        {
            printf("ERROR\nwrong magic\n");
            success = false;
        }
        else if (sf_header.version < 86 || sf_header.version > 202)
        {
            printf("ERROR\nwrong version\n");
            success = false;
        }
        else if (sf_header.no_of_sections < 8 || sf_header.no_of_sections > 20)
        {
            printf("ERROR\nwrong sect_nr\n");
            success = false;
        }
        else
        {
            // read section headers and check if type is correct
            for (i = 0; i < sf_header.no_of_sections; i++)
            {
                int flag = 0;

                if (sf_header.section_headers[i].sect_type == 30 || sf_header.section_headers[i].sect_type == 19)
                {
                    flag = 1;
                }
                if (flag == 0)
                {
                    printf("ERROR\nwrong sect_types\n");
                    success = false;
                    break;
                }
            }
        }
        close(fd);
    }

    if (success)
    {
        printf("SUCCESS\n");
        printSFHeader(&sf_header);
    }

    if (sf_header.section_headers != NULL)
    {
        free(sf_header.section_headers);
    }
}

int main(int argc, char **argv)
{

    int i;
    char *path = "";
    int recursive = 0;
    int size_greater = 0;
    int perm_write = 0;
    DIR *dir;
    char *filter_criteria;

    if (argc >= 2)
    {
        if (strstr(argv[1], "variant"))
        {
            printf("75814\n");
        }
        else if (strstr(argv[1], "list"))
        {

            path = toArg(argv[argc - 1], "path");
            dir = opendir(path);

            if (dir)
                printf("SUCCESS\n");
            else
            {
                printf("ERROR\ninvalid directory path\n");
                exit(1);
            }

            closedir(dir);

            if (argc >= 3)
            {
                i = 2;
                while (i < argc)
                {

                    if (strcmp(argv[i], "recursive") == 0)
                    {
                        recursive = 1;
                    }

                    if (strcmp(argv[i], "has_perm_write") == 0)
                    {
                        perm_write = 1;
                    }

                    if (strstr(argv[i], "size_greater"))
                    {
                        filter_criteria = toArg(argv[i], "size_greater");
                        size_greater = atoi(filter_criteria);
                    }

                    i++;
                }
            }

            list(recursive, size_greater, perm_write, path);
        }
        else if (strstr(argv[1], "parse"))
        {
            path = toArg(argv[argc - 1], "path");
            parse(path);
        }
    }
    return 0;
}
