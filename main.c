#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms2bin.h"

void usage(char *progname, FILE *outfp)
{
	fprintf(outfp, "Usage: %s -x X -i input_file -o output_file [-H] [-v] [-h]\n", progname);
	return;
}

int main(int argc, char *argv[])
{
	int r;
	int opt;
	int X;
	_Bool X_set=0;
	_Bool verbose=0;
	_Bool is_host_width=0;
	char *filename_in=NULL, *filename_out=NULL;

	while((opt=getopt(argc, argv, "x:i:o:Hvh"))!=-1){
		switch(opt){
			char *endptr;

			case 'x':
				X=strtol(optarg, &endptr, 10);
				if((errno==ERANGE)&&((X==LONG_MAX)||(X==LONG_MIN))){
					perror("strtol");
					exit(EXIT_FAILURE);
				}else if(endptr==optarg){
					fprintf(stderr, "error: no digits were found on X\n");
					usage(argv[0], stderr);
					exit(EXIT_FAILURE);
				}
				X_set=!0;
				break;

			case 'i':
				filename_in=optarg;
				break;

			case 'o':
				filename_out=optarg;
				break;

			case 'v':
				verbose=!0;
				break;

			case 'H':
				is_host_width=!0;
				break;

			case 'h':
				usage(argv[0], stdout);
				exit(EXIT_SUCCESS);

			default:
				fprintf(stderr, "error: unknown option character: 0x%x\n", opt);
				usage(argv[0], stderr);
				exit(EXIT_FAILURE);
		}
	}

	if(!X_set){
		fprintf(stderr, "error: specify X\n");
		usage(argv[0], stderr);
		exit(EXIT_FAILURE);
	}

	if(filename_in==NULL){
		fprintf(stderr, "error: specify input_filename\n");
		usage(argv[0], stderr);
		exit(EXIT_FAILURE);
	}

	if(filename_out==NULL){
		fprintf(stderr, "error: specify output_filename\n");
		usage(argv[0], stderr);
		exit(EXIT_FAILURE);
	}

	r=ms2bin(X, filename_in, filename_out, is_host_width, verbose);

	if(verbose)
		fprintf(r==0?stdout:stderr, "ms2bin returned with %d\n", r);
	if(r!=0)
		exit(EXIT_FAILURE);

	return 0;
}
