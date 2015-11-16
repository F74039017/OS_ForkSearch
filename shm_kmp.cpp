#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX_KEYWORD_LEN 100
int pi[MAX_KEYWORD_LEN];

void fail(char* keyword);
int KMP(char* mptr, size_t len, char* keyword);

/* ./exec filename keyword */
int main(int argc, char** argv)
{
	if(argc!=3)
	{
		puts("Wrong param");
		return -1;
	}

	/* open file */
	int fd = open(argv[1], O_RDONLY);
	if(fd==-1)
	{
		puts("Fail to open file");
		exit(EXIT_FAILURE);
	}
	struct stat sb;
	if(fstat(fd, &sb) == -1)
	{
		puts("fstat error");
		exit(EXIT_FAILURE);
	}
	size_t len = sb.st_size;
	/* memory map */
	char* mptr = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);

	/* cal. fail function and count matching patterns */
	fail(argv[2]);
	int result = KMP(mptr, len, argv[2]);
	printf("Result = %d\n", result);

	return 0;
}

/* Create fail function for keyword */
void fail(char* keyword)
{
	int len = strlen(keyword);
	if(len>MAX_KEYWORD_LEN)
	{
		printf("Keyword should < 100");
		exit(0);
	}

	for(int i=1, cur_pos=-1; i<len; ++i)
	{
		pi[0] = -1;
		while(cur_pos>=0 && keyword[i]!=keyword[cur_pos+1])
			cur_pos=pi[cur_pos];
		if(keyword[i]==keyword[cur_pos+1])
			++cur_pos;
		pi[i] = cur_pos;
	}

	// DEBUG - FAIL FUNCTION
	/*
	for(int i=0; i<len; i++)
		printf("%d ", pi[i]);
	puts("");
	*/
}

/* Count number of matching */
int KMP(char* mptr, size_t blen, char* keyword)
{
	int count = 0;
	int klen = strlen(keyword);
	for(int i=0, cur_pos=-1; i<blen; ++i)
	{
		while(cur_pos>=0 && mptr[i]!=keyword[cur_pos+1])
			cur_pos=pi[cur_pos];
		if(mptr[i]==keyword[cur_pos+1])
			++cur_pos;
		if(cur_pos+1 == klen)
		{
			cur_pos=pi[cur_pos];
			++count;
		}
	}
	return count;
}
