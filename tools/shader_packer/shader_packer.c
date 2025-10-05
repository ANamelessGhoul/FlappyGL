#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

const char* shader_dir_path;
int shader_dir_length;

typedef void file_visitor_t(const char*);

// Adapted from: https://gist.github.com/luciditee/35660fb3eecd309c4d24947e389e8d64
void traverse_directory(const char *dir_path, file_visitor_t* visitor) {
#ifdef _WIN32
    WIN32_FIND_DATA find_data;
    HANDLE hFind;
    char search_path[1024];

    snprintf(search_path, sizeof(search_path), "%s\\*", dir_path);
    hFind = FindFirstFile(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening directory: %s\n", dir_path);
        return;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s\\%s", dir_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            traverse_directory(path, visitor);
        } else {
            visitor(path);
        }
    } while (FindNextFile(hFind, &find_data) != 0);

    FindClose(hFind);
#else
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        char path[1024];
        struct stat statbuf;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            traverse_directory(path, visitor);
        } else {
            visitor(path);
        }
    }

    closedir(dp);
#endif
}


void pack_shader(const char* shader_path)
{

    char scratch[512];

    snprintf(scratch, 512, "%s/%s.h", "src/gen", shader_path + shader_dir_length + 1);
    printf("Packing shader file \"%s\" into \"%s\"\n", shader_path, scratch);


    FILE* src = fopen(shader_path, "rb");
    if (!src)
    {
        perror("Open source");
        return;
    }

    FILE* dest = fopen(scratch, "w");
    if (!dest)
    {
        perror("Open dest");
        fclose(src);
        return;
    }

    // Find slice past last '/' in shader_path
    const char* filename = shader_path + strlen(shader_path);
    while(*filename != '/' && filename != shader_path) filename--;
    filename++;

    // Find slice past last '.' in filename
    const char* extension = filename + strlen(filename);
    while(*extension != '.' && extension != filename) extension--;
    extension++;

    // Get part of filename before extension
    size_t len = extension - filename;
    for (size_t i = 0; i < len; i++) {
        scratch[i] = filename[i];
    }
    scratch[len - 1] = 0;
    
    fprintf(dest, "//Auto generated with shader_packer DO NOT EDIT\nstatic const char %s_%s[] = \"", scratch, extension);

    
    int bytes_read;
    do{
        bytes_read = fread(scratch, 1, 512, src);
        for (int i = 0; i < bytes_read; i++) {
            fprintf(dest, "\\x%x", scratch[i]);
        }
    
    } while(bytes_read > 0);
    fprintf(dest, "\";");
        

    fclose(dest);
    fclose(src);
}


int main(int argc, char** argv)
{
    shader_dir_path = "./shaders";
    shader_dir_length = strlen(shader_dir_path);
    traverse_directory(shader_dir_path, pack_shader);
}