// Synthetic-only offline PNG benchmark for the exact rm-shot encoder.
// No framebuffer, tablet, configuration, key or API access.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
static double now(void) {struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 const int w=1620,h=2160;
 unsigned char *rgb=malloc((size_t)w*h*3);if(!rgb)return 1;
 for(int kind=0;kind<3;++kind) {
  for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
   int ink=kind==1 ? (y>100 && y<560 && y%80<5 && x>120 && x<1150 && x%37<24) :
       kind==2 ? (x%52==0 || y%52==0) : 0;
   const unsigned char value=ink?0:255;
   for(int c=0;c<3;++c)rgb[((size_t)y*w+x)*3+c]=value;
  }
  for(int trial=0;trial<3;++trial) {
   char path[1024];snprintf(path,sizeof(path),"%s/%d-%d.png",argv[1],kind,trial);
   double start=now();if(!stbi_write_png(path,w,h,3,rgb,w*3))return 1;
   double elapsed=now()-start;struct stat st;if(stat(path,&st))return 1;
   printf("{\"fixture\":%d,\"trial\":%d,\"seconds\":%.6f,\"bytes\":%lld}\n",kind,trial,elapsed,(long long)st.st_size);
  }
 }
 free(rgb);return 0;
}
