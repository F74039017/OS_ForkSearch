#include <cstdio>
#include <cstring>
#include <cstdlib>

#define MAX_KEYWORD_LEN 100
int pi[MAX_KEYWORD_LEN];

void fail(char* keyword);
int KMP(char* buf, int len, char* keyword);

int main(int argc, char** argv)
{
	if(argc!=3)
	{
		puts("Wrong param");
		return -1;
	}

	fail(argv[2]);
	int result = KMP(argv[1], strlen(argv[1]), argv[2]);
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
int KMP(char* buf, int blen, char* keyword)
{
	int count = 0;
	int klen = strlen(keyword);
	for(int i=0, cur_pos=-1; i<blen; ++i)
	{
		while(cur_pos>=0 && buf[i]!=keyword[cur_pos+1])
			cur_pos=pi[cur_pos];
		if(buf[i]==keyword[cur_pos+1])
			++cur_pos;
		if(cur_pos+1 == klen)
		{
			cur_pos=pi[cur_pos];
			++count;
		}
	}
	return count;
}
