#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int base_file;
    short word;
    int read_bytes = 1;
  
    short Housekeeping = 0; //0x484b;
    short Mask = 0; //0x4d4b;
    short Data = 0; //0x3253;
    short Flush = 0; //0x4e4c;
    short TAS_H = 0; //0x5454;
    short TAS_V = 0; //0x4141;



    base_file = open(argv[1], O_RDONLY);

    while (read_bytes > 0){
        read_bytes = read(base_file, &word, sizeof(word));
        //printf("%04x\n", word);
 
        switch(word){

	    case 0x484b:
		Housekeeping++;

 	    case 0x4d4b:
		Mask++;
	    
	    case 0x3253:
		Data++;

	    case 0x4e4c:
		Flush++;

	    case 0x5454:
		TAS_H++;

	    case 0x4141:
		TAS_V++;
        }

    }
 
    close(base_file);

    printf("HouseKeeping Count: %d\n", Housekeeping);
    printf("Mask Count: %d\n", Mask);
    printf("Data Count: %d\n", Data);
    printf("Flush Count: %d\n", Flush);
    printf("TAS_H Count: %d\n", TAS_H);
    printf("TAS_V Count: %d\n", TAS_V);

    return 0;
}
