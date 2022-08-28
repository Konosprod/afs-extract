#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>

#define SIGN_L 0x41465300
#define SIGN_B 0x00534641

#define CHUNK_SIZE 2048

#define NAME_SIZE 20
#define ATTR_SIZE 30

typedef struct Token
{
  uint32_t size;
  uint32_t offset;
  char name[NAME_SIZE];
}Token;

typedef struct Header
{
  uint32_t sign;
  uint32_t nbfile;
  uint32_t attr_offset;
  uint32_t attr_size;
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

  fread(&h->attr_offset, sizeof(uint32_t), 1, in);
  fread(&h->attr_size, sizeof(uint32_t), 1, in);

  printf("%#010x and %#010x\n", h->attr_offset, h->attr_size);

  fseek(in, h->attr_offset, SEEK_SET);
  for(int i = 0; i < h->nbfile; i++)
  {
    fread(&h->tok[i].name, sizeof(char), NAME_SIZE, in);
    printf("name \"%s\"\n", h->tok[i].name);
    // Skip other attributes (date)
    fseek(in, 28, SEEK_CUR);
  }
}

void dump_file(FILE* in, Header* h)
{
  char name[20] = {0};
  unsigned char file_buffer[CHUNK_SIZE] = {0};
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
    sprintf(name, "%s/%s", dirname, h->tok[i].name);
    FILE* out = fopen(name, "wb+");
    if (out == NULL)
    {
      printf("Failed to create file \"%s\": %s\n", name, strerror(errno));
      exit(errno);
    }
    fseek(in, h->tok[i].offset, SEEK_SET);
    printf("Dumping %s...", name);
    fflush(stdout);

    int bytes_remaining = h->tok[i].size;
    size_t bytes_read = 0;
    size_t bytes_to_read = 0;
    while(bytes_remaining > 0)
    {
      bytes_to_read = MIN(CHUNK_SIZE, bytes_remaining);
      bytes_read = fread(&file_buffer, sizeof(unsigned char), bytes_to_read, in);
      fwrite(&file_buffer, sizeof(unsigned char), bytes_read, out);
      bytes_remaining -= bytes_read;
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
