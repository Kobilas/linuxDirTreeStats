#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>

//no idea how deep these directories are supposed to get
//so here is where you set the max amount of files
#define MAX 512

int sumSize = 0;

void getDirSize(char* path);

int main(int argc, char* argv[]){
  char* fp = argv[1];
  getDirSize(fp);
  printf("%d\n", sumSize);
  return 0;
}

//declaration for the part of program that is recursive
void getDirSize(char* path){
  int fd = open(path, O_RDONLY);
  char fileName[MAX];
  if(fd == -1){
    perror("ERR: file not found");
    return;
  }

  struct dirent{
    int d_type;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[MAX];
  };
  struct stat statBuffer;
  int charsRead;
  char buffer[MAX];
  struct dirent* dirEnt;
  int bPos = 0;
  char dTyp;

  //looping through files and recursively looping thorugh directories
  for(;;){
    charsRead = syscall(SYS_getdents, fd, buffer, MAX);
    if(charsRead <= 0) break;
    dirEnt = (struct dirent*)(buffer + bPos);
    dTyp = *(buffer + bPos + (dirEnt -> d_reclen) - 1);
    //the path might be longer than MAX if you set it too low though
    char temPath[MAX];
    //concatenating to path for next potential directory
    strcpy(temPath, path);
    strcat(temPath, "/");
    strcat(temPath, (dirEnt -> d_name));
    if((strcmp((dirEnt -> d_name), "." ))
      && (strcmp((dirEnt -> d_name), ".."))){
        if(dTyp != DT_DIR){
          stat(temPath, &statBuffer);
          if(dTyp == DT_LNK) lstat(temPath, &statBuffer);
          sumSize += statBuffer.st_size;
      }
      //recursion
      else getDirSize(temPath);
    }
    lseek(fd, (dirEnt -> d_off), SEEK_SET);
  }
}
