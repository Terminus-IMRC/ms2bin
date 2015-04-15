/* ms2bin.c -- converts line magic squares to binary format */

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS<64
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
#include "libms.h"

/* return: 0=success, 1=normal failure, 2=library function failure, 3=system call failure */
int ms2bin(int cmd_X, char *cmd_filename_in, char *cmd_filename_out, _Bool verbose)
{
	int X;
	int fdin;
	int chars_per_input_line;
	int buffersize;
	char *inbuf;
	int *ms;
	ms_state_t st;
	ms_bin_seq_write_t mbw;
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
	}

	chars_per_input_line=	\
		1*(X*X<9?X:9) /* the numbers that are smaller than 9 */ \
		+2*(X*X<9?0:X*X-9) /* the numbers that are greater than 9 */	\
		+1*(X*X-1)	/* spaces after each of the numbers except for the last number */	\
		+1	/* a newline character after the last number */;
	buffersize=chars_per_input_line;

	if(verbose)
		printf("info: chars_per_input_line=%d\n", chars_per_input_line);

	if(verbose)
		printf("info: buffersize=%d\n", buffersize);

	inbuf=(char*)malloc(buffersize*sizeof(char));
	if(inbuf==NULL){
		fprintf(stderr, "error: failed to malloc inbuf of which size is %d\n", buffersize*sizeof(char));
		return 2;
	}

	ms_init(cmd_X, MS_ORIGIN_ONE, &st);
	ms=ms_alloc(&st);
	ms_bin_seq_write_open(cmd_filename_out, MS_BIN_SEQ_WRITE_FLAG_CREAT|MS_BIN_SEQ_WRITE_FLAG_TRUNC, &mbw, &st);

	if(verbose)
		printf("info: initialization finished\n");

	while((rc=read(fdin, inbuf, buffersize))!=0){
		if(rc==-1){
			fprintf(stderr, "read: %s\n", strerror(errno));
			return 3;
		}else if(rc<0){
			fprintf(stderr, "error: read returned negative value, but this is not -1\n");
			return 2;
		}else if(rc%chars_per_input_line!=0){
			fprintf(stderr, "error: read count(%d) is not times of chars_per_input_line(%d)\n", rc, chars_per_input_line);
			return 1;
		}
		if(verbose&&(rc!=buffersize))
			printf("warning: rc!=buffersize\n");
		if(verbose)
			printf("info: inbuf: %s\n", inbuf);

		inbuf[buffersize-1]='\0';
		str_to_ms(ms, inbuf, &st);
		ms_bin_seq_write_next(ms, &mbw, &st);
	}

	close(fdin);
	free(inbuf);
	ms_bin_seq_write_close(&mbw, &st);
	ms_free(ms, &st);
	ms_finalize(&st);

	return 0;
}
