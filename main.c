#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define SIGN_L 0x41465300
#define SIGN_B 0x00534641

typedef struct Token
{
  uint32_t size;
  uint32_t offset;
}Token;

typedef struct Header
{
  uint32_t sign;
  uint32_t nbfile;
  Token* tok;
}Header;

void get_header(FILE* in, Header* h)
{
  fread(&h->sign, sizeof(char), 4, in);

  if((h->sign == SIGN_L) || (h->sign == SIGN_B))
  {
    printf("Signature : OK\n");
  }
  else
  {
    printf("Signature : KO\n");
    exit(EXIT_FAILURE);
  }

  fread(&h->nbfile, sizeof(char), 4, in);
  printf("File : %d\n", h->nbfile);

  h->tok = calloc(h->nbfile, sizeof(Token));

  if(!h->tok)
  {
    printf("Allocation impossible\n");
  }

  for(int i = 0; i < h->nbfile; i++)
  {
    fread(&h->tok[i].offset, sizeof(char), 4, in);
    fread(&h->tok[i].size, sizeof(char), 4, in);
    //printf("File[%d] : size : 0x%.8X\toffset : 0x%.8X\n", i+1, h->tok[i].size, h->tok[i].offset);
  }

}

void dump_file(FILE* in, Header* h)
{
  char name[20] = {0};
  unsigned char c = 0;
  const char* dirname = "DATA1";
  int err = 0;

  err = mkdir(dirname, S_IRWXU);

  if (err != 0 && errno != EEXIST)
  {
    printf("Failed to create directory \"%s\": %s\n", dirname, strerror(errno));
    exit(err);
  }

  for(int i = 0; i < h->nbfile; i++)
  {
    sprintf(name, "%s/%d", dirname, i);
    FILE* out = fopen(name, "wb+");
    if (out == NULL)
    {
      printf("Failed to create file \"%s\": %s\n", name, strerror(errno));
      exit(errno);
    }
    fseek(in, h->tok[i].offset, SEEK_SET);
    printf("Dumping %s...", name);
    fflush(stdout);
    for(int j = 0; j < h->tok[i].size; j++)
    {
      fread(&c, sizeof(char), 1, in);
      fwrite(&c, sizeof(char), 1, out);
    }
    printf("...done\n");
    fclose(out);
  }

}

int main(int argc, char* argv[])
{
  FILE* in = fopen(argv[1], "rb+");
  Header head;

  if(in == NULL)
  {
    printf("Impossible d'ouvrir le fichier %s\n", argv[1]);
  }

  get_header(in, &head);
  dump_file(in, &head);

  free(head.tok);
  return 0;
}
