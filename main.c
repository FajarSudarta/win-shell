// NOTE this C SHELL is for Windows

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <direct.h>
#include <dirent.h>
#include "sys/stat.h"
#include <errno.h>

#ifdef _WIN32
#include <io.h>     // _setmode, _fileno
#include <fcntl.h>  // _O_BINARY
#endif

#define MAX_LINE 1024
#define MAX_ARGS 64

typedef int (*builtin_fn)(int *argc, char *argv[]);
int parse_command(char *line, char *argv[]);
// int parse_command2(char *line, char *argv[]);
void debugger(int *argc, char *argv[]);
static int builtin_echo(int *argc, char *argv[]);
static int builtin_cd(int *argc, char *argv[]);
static int builtin_pwd();
static int builtin_clear();
static int builtin_create_file(int *argc, char *argv[]);
static int builtin_ls(int *argc, char *argv[]);
static int builtin_mkdir(int *argc, char *argv[]);

static int builtin_cat(int *argc, char *argv[]);


static int builtin_move(int *argc, char *argv[]);
// static int builtin_remove(int *argc, char *argv[]);
// static int builtin_copy(int *argc, char *argv[]);



static const struct {
    const char *name;
    builtin_fn  fn;
} builtins[] = {
    { "echo", builtin_echo },
    { "cd",   builtin_cd   },
    // { "pwd",  builtin_pwd  },
    // {"clear",builtin_clear},        //sudah dibikin
    {"create",builtin_create_file},
    {"mkdir", builtin_mkdir},
    { "ls", builtin_ls },
    {"cat", builtin_cat},

        // yang mau dibikin

    {"move",builtin_move}, // belum dibikin
    // {"remove",builtin_remove},
    // {"copy",builtin_copy},

};
int main(void)
{

    char line[MAX_LINE];
    char *argv[MAX_ARGS];
    int argc = 0;
    int command_status = 0;
    int found = 0;
    while(1)
    {

        printf("8==D$ ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        argc = parse_command(line,argv);

        if (argc < 0) {
            fprintf(stderr, "ATTACHI: unclosed quote\n");
            continue;
        }
        if (argc == 0) {
            continue;
        }
        // Built in commands
        if (strcmp(argv[0], "exit") == 0) {
            break;
        }
        found = 0;
        if (strcmp(argv[0], "clear") == 0) {
            builtin_clear();
            continue;
        }
        if (strcmp(argv[0], "pwd") == 0) {
            builtin_pwd();
            continue;
        }
        // debugger(&argc,argv);
        // continue;
        for (size_t i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
            if (strcmp(argv[0], builtins[i].name) == 0) {
                command_status = builtins[i].fn(&argc, argv);
                found  = 1;
                break;
            }

        }
        if(found == 0)
        {
             fprintf(stderr, "%s: command not found\n", argv[0]);
             // break;
        }

    }
    return 0;
}

static int builtin_create_file(int *argc, char *argv[])
{
    if(*argc < 2)
    {
        printf("Type name and format for create a file\n");
        return -1;
    }
    FILE *f = fopen(argv[1], "a");   /* "a": buat jika belum ada, TIDAK mengosongkan */
    if (f == NULL) {
        perror("create");
        return 1;
    }
    fclose(f);
    return 0;
}

static int builtin_mkdir(int *argc, char *argv[])
{

    if(*argc != 2)
    {
        printf("Tolong beri nama folder yang benar\n");
        return 1;

    }
    int check = _mkdir(argv[1]);
    if(check == -1)
    {
        printf("creating %s directory is failed\n",argv[1]);
        return 1;
    }
    printf("creating %s directory is successfully\n",argv[1]);

    return 0;
}

static int cat_one(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "cat: %s: %s\n", path, strerror(errno));
        return 1;
    }

    int ch;  // int, bukan char!
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    int status = 0;
    if (ferror(file)) {
        fprintf(stderr, "cat: %s: gagal membaca file\n", path);
        status = 1;
    }
    fclose(file);
    return status;
}

static int builtin_cat(int *argc, char *argv[])
{
    if (*argc < 2) {
        fprintf(stderr, "Penggunaan: cat <file> [file...]\n");
        return 1;
    }

    fflush(stdout);
#ifdef _WIN32
    int old_mode = _setmode(_fileno(stdout), _O_BINARY);
#endif

    int status = 0;
    for (int i = 1; i < *argc; i++) {
        if (cat_one(argv[i]) != 0)
            status = 1;
    }

    fflush(stdout);
#ifdef _WIN32
    _setmode(_fileno(stdout), old_mode);
#endif
    return status;
}

static int builtin_move(int *argc, char *argv[])
{
    (void)argc;   /* cast to void = "I know this is unused" */
    (void)argv;
    printf("move: still developed\n");
    return 0;
}



// static int builtin_remove(int *argc, char *argv[])
// {
//     printf("remove: still developed\n");
//     return 0;

// }
// static int builtin_copy(int *argc, char *argv[])
// {
//     printf("copy: still developed\n");
//     return 0;

// }
static int builtin_ls(int *argc, char *argv[])
{
    if (*argc >= 3)
    {
        fprintf(stderr, "too much argument for ls\n");
        return  1;
    }
     DIR *directory;
     struct dirent *entry;
     char cwd[PATH_MAX];

     if (getcwd(cwd, sizeof(cwd)) == NULL) {
         perror("pwd");
         return 1;
     }
     directory = opendir(cwd);
     if (directory == NULL) {
         fprintf(stderr, "error opening directory\n");

         return  1;
     }
     struct stat path_stat;
     while ((entry = readdir(directory)) != NULL) {
         if(argv[1] != NULL)
         {
             stat(entry->d_name, &path_stat);
             if(strcmp(argv[1],"--files") == 0)
             {
                 if (S_ISREG(path_stat.st_mode)) {
                     // Ini adalah file reguler
                     printf("%s\n",entry->d_name);

                 }
             }
             else if(strcmp(argv[1],"--folder") == 0)
             {
                 if (S_ISDIR(path_stat.st_mode)) {
                     // Ini adalah direktori / folder
                     printf("%s\n",entry->d_name);

                 }
             }
         }
         else
         {
             printf("%s\n",entry->d_name);
         }
     }
     if (closedir(directory) == -1) {
         fprintf(stderr, "error closing directory\n");
         return 1;
     }
     // printf("ls: still developed\n");
     return 0;
}


static int builtin_clear()
{
    printf("\033[2J\033[H");
    return 0;
}

static int builtin_pwd()
{
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        return 1;
    }
    printf("%s\n", cwd);
    return 0;
}

static int builtin_cd(int *argc, char *argv[])
{
    (void)argc;
    const char *target = argv[1];          /* NULL if no argument */
    if (target == NULL) {
        target = getenv("HOME");
        if (target == NULL)
            target = getenv("USERPROFILE");
    }

    if (target == NULL) {
        fprintf(stderr, "cd: HOME not set\n");
        return 1;
    }
    if (chdir(target) != 0) {
        perror("cd");
        return 1;
    }
    return 0;
}

static int builtin_echo(int *argc, char *argv[])
{

    long loop_count = 1;   /* default: cetak sekali */
    int start = 1;         /* index argumen pertama yang dicetak */

    if (*argc >= 2 && strcmp(argv[1], "--loop") == 0)
    {
        if (*argc < 3)
        {
            fprintf(stderr, "echo: --loop butuh angka\n");
            return 1;
        }

        char *end;
        errno = 0;
        loop_count = strtol(argv[2], &end, 10);

        /* tolak: overflow, string kosong, ada sisa karakter, atau negatif */
        if (errno != 0 || end == argv[2] || *end != '\0' || loop_count < 0)
        {
            fprintf(stderr, "echo: angka tidak valid: '%s'\n", argv[2]);
            return 1;
        }
        start = 3;

    }

    for (long n = 0; n < loop_count; n++)
    {
        for (int i = start; i < *argc; i++)
        {
            fputs(argv[i], stdout);
            if (i < *argc - 1)
            {
                putchar(' ');
            }
        }
        putchar('\n');
    }
    return 0;
}


int parse_command(char *line, char *argv[])
{
    int argc = 0;
    char *src = line;   /* posisi baca */
    char *dst = line;   /* posisi tulis (selalu <= src) */

    while (argc < MAX_ARGS - 1) {
        /* 1. Lewati spasi/tab di antara token */
        while (*src == ' ' || *src == '\t')
            src++;
        if (*src == '\0')
            break;

        /* 2. Token baru dimulai di posisi tulis */
        argv[argc++] = dst;
        char quote = 0;   /* 0 = di luar kutip, '"' atau '\'' = di dalam kutip */

        /* 3. Salin karakter sampai ketemu pemisah DI LUAR kutip */
        while (*src != '\0')
        {
            if (quote)
            {
                if (*src == quote)
                {
                    quote = 0;              /* kutip penutup: dibuang */
                }
                else
                {
                    *dst++ = *src;          /* spasi pun disalin */
                }
            }
            else if (*src == '"' || *src == '\'')
            {
                quote = *src;               /* kutip pembuka: dibuang */
            }
            else if (*src == ' ' || *src == '\t')
            {
                break;                      /* akhir token */
            }
            else
            {
                *dst++ = *src;
            }
            src++;
        }

        if (quote)
            return -1;                      /* kutip tidak ditutup */

        /* 4. Akhiri token. Simpan karakter di src DULU, karena
         *    jika dst == src, menulis '\0' akan menimpanya. */
        char end = *src;
        *dst++ = '\0';
        if (end == '\0')
            break;
        src++;
    }

    argv[argc] = NULL;
    return argc;
}
void debugger(int *argc, char *argv[])
{
    for (int i = 0; i < *argc; i++) {
        printf("argv[%d] = [%s]\n", i, argv[i]);
    }
}

// int parse_command(char *line, char *argv[])
// {
//     int argc = 0;

//     char *token = strtok(line, " \t");

//     while (token != NULL && argc < MAX_ARGS - 1) {
//         argv[argc++] = token;
//         token = strtok(NULL, " \t");
//     }

//     argv[argc] = NULL;

//     return argc;
// }


// static int builtin_cat(int *argc, char *argv[])
// {
//     // printf("cat: still developed\n");
//     if(*argc != 2)
//     {
//         printf("Please type command properly\n");
//         return 1;
//     }
//     FILE *file = fopen(argv[1],"r");
//     if(file == NULL)
//     {
//         printf("Tidak bisa membuka file %s\n",argv[1]);
//         return 1;
//     }
//     char ch;
//     while ((ch = fgetc(file)) != EOF) {
//         putchar(ch);
//     }
//     fclose(file);
//     return 0;
// }
