/* ms2bin.c -- converts line magic squares to binary format */

#if _FILE_OFFSET_BITS<64
#undef _FILE_OFFSET_BITS
#endif /* _FILE_OFFSET_BITS<64 */
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif /* _FILE_OFFSET_BITS */

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif /* _LARGEFILE64_SOURCE */

#include <sys/types.h>
#include <limits.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "ms2bin.h"

/* return: 0=success, 1=normal failure, 2=library function failure, 3=system call failure */
int ms2bin(int cmd_X, char *cmd_filename_in, char *cmd_filename_out, _Bool verbose)
{
	int i;
	int X;
	int chars_per_input_line;
	int fdin, fdout;
	char *inbuf, *inbuf_orig, *outbuf;
	ssize_t rc;

	if(cmd_X<1){
		fprintf(stderr, "error: X must be 1 or greater\n");
		return 1;
	}else if(cmd_X>(int)floor(sqrt(0xff))){
		fprintf(stderr, "error: X greater than %d is not supported yet for endian problems\n", (int)floor(sqrt(0xff)));
		return 1;
	}
	X=cmd_X;

	if(cmd_filename_in==NULL){
		fprintf(stderr, "error: specified input filename is NULL\n");
		return 1;
	}else if((fdin=open(cmd_filename_in, O_RDONLY|O_LARGEFILE))==-1){
		fprintf(stderr, "open(\"%s\", O_RDONLY|O_LARGEFILE): %s\n", cmd_filename_in, strerror(errno));
		return 3;
	}

	if(cmd_filename_out==NULL){
		fprintf(stderr, "error: specified output filename is NULL\n");
		return 1;
	}else if((fdout=open(cmd_filename_out, O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))==-1){
		fprintf(stderr, "open(\"%s\", O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH): %s\n", cmd_filename_out, strerror(errno));
		return 3;
	}

	chars_per_input_line=	\
		1*(X*X<9?X:9) /* the numbers that are smaller than 9 */ \
		+2*(X*X<9?0:X*X-9) /* the numbers that are greater than 9 */	\
		+1*(X*X-1)	/* spaces after each of the numbers except for the last number */	\
		+1	/* a newline character after the last number */;

	if(verbose)
		printf("info: chars_per_input_line=%d\n", chars_per_input_line);

	inbuf=(char*)malloc(chars_per_input_line*sizeof(char));
	if(inbuf==NULL){
		fprintf(stderr, "error: failed to malloc inbuf of which size is %d\n", chars_per_input_line*sizeof(char));
		return 2;
	}
	inbuf_orig=inbuf;

	outbuf=(char*)malloc((X*X)*sizeof(char));
	if(outbuf==NULL){
		fprintf(stderr, "error: failed to malloc outbuf of which size is %d\n", (X*X)*sizeof(char));
		return 2;
	}

	if(verbose)
		printf("info: initialization finished\n");

	while((rc=read(fdin, inbuf, chars_per_input_line))!=0){
		if(rc!=chars_per_input_line){
			if(rc==-1){
				fprintf(stderr, "read: %s\n", strerror(errno));
				return 3;
			}else if(rc<0){
				fprintf(stderr, "error: read returned negative value, but this is not -1\n");
				return 2;
			}else{
				ssize_t totalread=rc;

				if(verbose)
					printf("info: read count %d is insufficient; trying to fill the buffer\n", rc);
				while(totalread!=chars_per_input_line){
					ssize_t rrc=0;
					rrc=read(fdin, inbuf+rrc, chars_per_input_line-rrc);
					if(rc==-1){
						fprintf(stderr, "read: %s\n", strerror(errno));
						return 3;
					}else if(rc<0){
						fprintf(stderr, "error: read returned negative value, but this is not -1\n");
						return 2;
					}
					totalread+=rrc;
				}
			}
		}
		inbuf[chars_per_input_line-1]='\0';

		if(verbose)
			printf("info: inbuf: %s\n", inbuf);

		for(i=0; i<X*X; i++){
			long int b;
			char *inbuf_cur;

			while(*inbuf==' ')
				inbuf++;
			inbuf_cur=inbuf;
			b=strtol(inbuf, &inbuf, 10);
			if(inbuf_cur==inbuf){
				fprintf(stderr, "error: invalid input: insufficient number of numbers on a line\n");
				return 1;
			}else if((b==LONG_MAX)||(b==LONG_MIN)){
				fprintf(stderr, "strtol: %s\n", strerror(errno));
				return 2;
			}
			outbuf[i]=(char)b;
		}

		rc=write(fdout, outbuf, X*X);
		if(rc==-1){
			fprintf(stderr, "write: %s\n", strerror(errno));
			return 3;
		}else if(rc<0){
			fprintf(stderr, "error: write returned negative value, but this is not -1\n");
			return 2;
		}

		inbuf=inbuf_orig;
	}

	close(fdin);
	close(fdout);
	free(inbuf);
	free(outbuf);

	return 0;
}
