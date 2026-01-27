#include <stdio.h>
#include <string.h>

#include "svg.h"

int main(int argc, char **argv)
{
  char *filename;
  char *path = "/tmp/test-out.svg";
  int r;
  svg *s;

  if (argc < 2)
  {
    fprintf(stderr, "Please provide a path that contains an SVG file to test.\n");
    return 1;
  }

  filename = argv[1];

fprintf(stderr,"main(): calling svg_read(%s)\n",filename);fflush(stderr);
  s = svg_read(filename);
fprintf(stderr, "svg_read()=%p\n",s);fflush(stderr);

fprintf(stderr,"main(): calling svg_write(%p,%s)\n",s,path);fflush(stderr);
  r = svg_write(s, path);
fprintf(stderr, "svg_write()=%d\n",r);fflush(stderr);
  if (r)
  {
    fprintf(stderr, "Failed to write '%s'.\n", path);
    fflush(stderr);
  }

  if (s) svg_free(s);

  return 0;
}

