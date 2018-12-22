#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
//#include <cstdlib.h>
#include <time.h>

unsigned char hexdigit[] = {0x3F, 0x06, 0x5B, 0x4F,
                            0x66, 0x6D, 0x7D, 0x07, 
                            0x7F, 0x6F, 0x77, 0x7C,
			                      0x39, 0x5E, 0x79, 0x71};
			                      
typedef struct VECTOR
{
 	int option; 	//1: 7seg, 2: ledG, 3:ledR
 	unsigned data;  //dados
}Vect;

int main() {
  	int i, j=100;
	vect* k;
	k->
  int dev = open("/dev/de2i150_altera", O_RDWR);
  
  	int var=90;
    //var=read(dev, &j, 1);
  	printf("%d %d\n",j,var);
    var = write(dev, &k, 1);
 

  close(dev);
  return 0;
}
