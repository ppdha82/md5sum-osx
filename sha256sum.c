#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sha256.h"

int printSHA256(const char *fileName) {
    char *sha256 = sha256sum(fileName);
    if(sha256) {
    printf("%s  %s\n", sha256, fileName);
    return 1;
    } else {
        return 0;
    }
}

void checkSHA256(const char *sumFile) {
    FILE *fp;
    char *line = malloc(1024);
    char *sha256, *storedSHA256;
    char *file;
    char *b;
    int fileok = 0, fileerr = 0, filetotal = 0, lineno = 0;
    if(!strncmp("-", sumFile, 1) || !strncmp("/dev/stdin", sumFile, 10)) {
		fp = stdin;
	} else {
		fp = fopen(sumFile, "r");
	}
    if(!fp) {
        fprintf(stderr, "ERROR: Cannot open sumfile: %s(%d)\n", strerror(errno), errno);
    }
    
    /*fprintf(stderr, "DEBUG: Entering to reading file...\n");*/
    while(fgets(line, 1024, fp)) {
        /*fprintf(stderr, "DEBUG: line='%s'\n", line);*/
        lineno++;
        storedSHA256 = strtok(line, " ");
        if(storedSHA256 == NULL) {
            /* Maybe an empty line given */
            continue;
        }
        if(!strncmp("#", storedSHA256, 1)) {
            continue;
        }
        file = strtok(NULL, " ");
        if(!strncmp(" ", file, 1)) {
            /* Maybe a broken strtok... */
            fprintf(stderr, "WARNING: a broken strtok...");
            file = strtok(NULL, " ");
            if(!strncmp(" ", file, 1)) {
                /* We got a separator again, something is not so good... */
                fprintf(stderr, "ERROR: something went to totally wrong, crashing...\n");
                fprintf(stderr, "DEBUG: stack: storedSHA256='%s', sumFile='%s', line='%s'\n", storedSHA256, sumFile, line);
                free(line);
                return;
            }
        }
        
        if(!file || !strncmp("\n", file, 1)) {
            fprintf(stderr, "WARNING: invalid formatted line given at %s:%d\n", sumFile, lineno);
            fprintf(stderr, "WARNING: but keep going...");
            continue;
        }
        /* My little chomp() implementations */
        for(b = file; *b != 0; b++) {
            if(*b == '\n') {
                *b = 0;
                break;
            }
        }
        for(b = storedSHA256; *b != 0; b++) {
            if(*b == ' ') {
                *b = 0;
                break;
            }
        }
        /*fprintf(stderr, "DEBUG: stack: storedSHA256='%s', sumFile='%s', file='%s'\n", storedSHA256, sumFile, file);*/
        
        sha256 = sha256sum(file);
        filetotal++;
        
        if(!sha256 || strncmp(sha256, storedSHA256, 32)) {
            fileerr++;
            printf("%s: FAILED\n", file);
        } else {
            fileok++;
            printf("%s: OK\n", file);
        }
    }
    if(fileerr > 0) {
        printf("WARNING: %d of %d did NOT match\n", fileerr, filetotal);
    }
}

int main(int argc, char **argv) {
    int i;
    const char *fileName;
    const char *sumFile;
	int got_stdin = 0;
    
    struct stat x; /* Not really used */    
    
    if(argc < 2 || !strncmp("--help", argv[1], 6)) {
        printf("Usage: %s file1 file2\n", argv[0]);
        printf("       %s -c sumfile\n", argv[0]);
        return 1;
    }
    if(!strncmp("-c", argv[1],2)) {
        sumFile = argv[2];
        argc -= 2;
        argv +=2;
        checkSHA256(sumFile);
    
    } else {
       for(i = 1; i < argc; i++) {
            fileName = argv[i];
			if(!strncmp("-", fileName, 1) || !strncmp("/dev/stdin", fileName, 10)) {
				if(got_stdin) {
					fprintf(stderr, "WARNING: stdin twice in command line, skipping...\n");
					continue;
				} else {
					got_stdin = 1;
				}
			} else {
	            if(stat(fileName, &x) < 0) {
	                fprintf(stderr, "ERROR: file cannot be opened: %s : %s\n", fileName, strerror(errno));
	                continue;
	            }
			}
            printSHA256(fileName);
        }
       
    }
    return 0; 
}