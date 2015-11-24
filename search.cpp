#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Fail function */
#define MAX_KEYWORD_LEN 100
int pi[MAX_KEYWORD_LEN];

#define OPEN_FLAG O_CREAT|O_RDWR
#define OPEN_MODE 0644
#define FILE_SIZE sizeof(int)*atoi(argv[3])

void fail(char* keyword);
int KMP(char* mptr, size_t len, char* keyword, bool istail);

/* ./exec filename keyword proc_num */
int main(int argc, char** argv)
{
	if(argc!=4)
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
	/* get file size */
	struct stat sb;
	if(fstat(fd, &sb) == -1)
	{
		puts("fstat error");
		exit(EXIT_FAILURE);
	}
	size_t len = sb.st_size-1;
	/* memory map */
	char* mptr = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if(mptr == MAP_FAILED)
	{
		perror("mmap failed");
		exit(EXIT_FAILURE);
	}

	/* segment info. */
	int proc_num = atoi(argv[3]);
	size_t avg_len = len/proc_num;
	size_t remain = len%proc_num;
	size_t key_len = strlen(argv[2]);
	size_t normal_seg_len = avg_len+key_len-1;
	size_t tail_seg_len = avg_len+remain;
	
	/* Check prcesses and length */
	if(avg_len<key_len)
	{
		puts("Too many processes or key length is greater than that of the string");
		exit(EXIT_FAILURE);
	}

	/* Calculate the fail function */
	fail(argv[2]);

	/* Create shared memory to save result */
	int rfd = shm_open("match_num", OPEN_FLAG, OPEN_MODE);
	if(rfd == -1)
	{
		puts("shm_open failed");
		exit(EXIT_FAILURE);
	}
	if(ftruncate(rfd, FILE_SIZE) == -1)
	{
		puts("ftruncate failed");
		exit(EXIT_FAILURE);
	}
	int* rptr = (int*)mmap(NULL, FILE_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, rfd, 0);
	if(rptr == MAP_FAILED)
	{
		puts("Result mmap error");
		exit(EXIT_FAILURE);
	}

	/* fork n processes */
	for(int i=0; i<proc_num; i++)
	{
		pid_t pid = fork();
		if(pid==0)
		{
			/* Set info for normal and tail segments */
			size_t seg_len;
			bool istail;
			if(i==proc_num-1)
			{
				seg_len = tail_seg_len;
				istail = true;
			}
			else
			{
				seg_len = normal_seg_len;
				istail = false;
			}
			// Use shared memory to get the result from child processes.
			int result = *(rptr+i) = KMP(mptr+i*avg_len, seg_len, argv[2], istail);

			//	DEBUG - CHECK SEGMENT
			/*
			char check[100];
			strncpy(check, mptr+i*avg_len, seg_len);
			check[seg_len] = '\0';
			printf("This is child %d, result = %d, word = %s, seg_len = %zu\n", i, result, check, seg_len);
			*/

			exit(EXIT_SUCCESS);
		}
	}

	/* wait and check all child processes */
	int status;
	pid_t cpid;
	for(int i=0; i<proc_num; i++)
	{
		cpid = wait(&status);
		if(status!=EXIT_SUCCESS)
		{
			puts("Some child process failed");
			exit(EXIT_FAILURE);
		}
	}

	/* printf result and total */
	int sum = 0;
	for(int i=0; i<proc_num; i++)
	{
		printf("process%d: %d ", i+1, *(rptr+i));
		sum += *(rptr+i);
	}
	printf("total: %d\n", sum);

	// close mmap
	munmap(mptr, len);
	munmap(rptr, FILE_SIZE);
	close(fd);
	close(rfd);

	return 0;
}

/* Create fail function for keyword */
void fail(char* keyword)
{
	size_t len = strlen(keyword);
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
int KMP(char* mptr, size_t blen, char* keyword, bool istail)
{
	int count = 0;
	size_t klen = strlen(keyword);
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
