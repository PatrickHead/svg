#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "svg.h"

int main(int argc, char **argv)
{
  DIR *dir;
  struct dirent *de;
  char *dirname;
  char *name;
  char path[4096];
  char *ext;
  int len;
  int r;
  svg *s;

  if (argc < 2)
  {
    fprintf(stderr, "Please provide a path that contains SVG files to test.\n");
    return 1;
  }

  memset(path, 0, 4096);

  dirname = argv[1];
  len = strlen(dirname);
  if (dirname[len-1] == '/') dirname[len-1] = 0;

  dir = opendir(dirname);

  while ((de = readdir(dir)))
  {
    name = de->d_name;
    if (name[0] == 'X') continue;
    if (!strcmp(name, ".")) continue;
    if (!strcmp(name, "..")) continue;

    len = strlen(name);
    ext = name + len - 1;

    while (ext > name)
    {
      if (*ext == '.')
      {
        ++ext;
        break;
      }
      --ext;
    }

    if (ext == name) continue;
    if (strcasecmp(ext, "svg")) continue;

    sprintf(path, "%s/%s", dirname, name);
    s = svg_read(path);
    if (!s)
    {
      fprintf(stderr, "svg_read(%s) FAILED\n", path);
      continue;
    }

    sprintf(path, "%s/X%s", dirname, name);
    r = svg_write(s, path);
    if (r)
    {
      fprintf(stderr, "Failed to write '%s'.\n", path);
      svg_free(s);
      return 1;
    }

    svg_free(s);
  }

  closedir(dir);

  return 0;
}

