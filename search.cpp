// search.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include "openssl/hmac.h"
#include <random>
#include <bitset>

using namespace std;

#define NUM_TOKEN 200000
#define NUM_READ 2000
//#define NUM_FILE 12000  //文件数


char key[] = "012345678";

//id列表结点
typedef struct Node_id {
	int id;
	int num;  //一篇文献中的某一索引词出现的次数
	struct Node_id *next;
}Node_id;

//树结点
typedef struct Ttree
{
	char data[20];
	//double weight; //索引词对应的IDF值
	int n;  //索引词出现在几个文档中
	Node_id *list;
	struct Ttree *lchild;   //左儿子
	struct Ttree *rchild;  //右儿子
}Ttree;

double NF = NUM_READ; //文件数double类型
int NUM_WORD = 0; //单词数
int i = 0;
double total_t = 0;
const char *except[69] = { "our", "my", "your", "they", "we", "you", "I", "me", "us", "them", "he", "she", "is", "are", "be", "the", "a", "an", "in", "at", "of", "on", "and", "for", "to", "that", "this", "it", "would", "will", "can", "could", "may", "should", "com", "all", "bit", "content", "all", "charset", "re", "pst", "plain", "text", "version", "thyme", "transfer", "subject", "type", "bcc", "cc", "date", "evans", "filename", "id", "javamail", "merriss", "message", "mime", "notes", "nsf", "origin", "documents", "encoding" ,"enron", "by", "folders", "folder", "from" };  //需要去除的单词




long gettime() {
	clock_t start = clock();
	return start;
}

/*每个文件建立一棵二叉树*/
Ttree *createTtree(Ttree *root, FILE *fp, int identifier, int file_size)
{
	int i = 0, t = 0, k, flag;
	Ttree *p = NULL, *q;  //定义中间指针变量
	Node_id *s, *r;
	char ch;

	if (!fp)
	{
		printf("\nCannot open the file and exit!\n");
		return NULL;
	}//end if
	ch = fgetc(fp); //读取一个字符

	if (root == NULL)
	{
		p = (Ttree*)malloc(sizeof(Ttree));	//申请新的存储空间   
		p->data[0] = '\0';

		while ((ch != EOF) && (t == 0)) //读取第一个单词并生成根节点
		{
			if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z'))//如果这个字符是字母
			{
				if (ch <= 'Z') ch = ch + 32; //如果该字母大写则转成小写
				p->data[i] = ch;
				i++;
			} //end if  
			else
			{
				if (p->data[0] == '\0')
				{
					ch = fgetc(fp);
					continue;
				}//end if 
				p->data[i] = '\0';
				p->n = 1;

				s = (Node_id *)malloc(sizeof(Node_id));
				s->id = identifier;
				s->num = 1;
				//s->max = file_size / 10;
				s->next = NULL;

				p->list = s;
				p->lchild = NULL;  //初始化头节点的左右儿子为空指针
				p->rchild = NULL;

				i = 0;
				t = 1;
				root = p;
			}//end else   
			ch = fgetc(fp);
		}// end while
	}//end if

	q = (Ttree*)malloc(sizeof(Ttree));
	q->data[0] = '\0';

	while (ch != EOF) //读取文本直到结束
	{
		if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z'))
		{
			if (ch <= 'Z') ch = ch + 32;
			q->data[i] = ch;
			i++;
			ch = fgetc(fp);
		}//end if
		else
		{
			if (q->data[0] == '\0')
			{
				ch = fgetc(fp);
				continue;
			}//end if   
			q->data[i] = '\0';

			flag = 0;
			if (i == 1)  //除去一个字母的情况
			{
				free(q);
				i = 0;
				flag = 1;
				ch = fgetc(fp);
			}//end if

			for (k = 0; k < 69 && flag == 0; k++)
			{
				if (!strcmp(q->data, except[k]))  //若该词是应删除的词
				{
					free(q);
					i = 0;
					flag = 1;
					ch = fgetc(fp);
					break;
				}//end if
			}//end for

			if (flag == 0)  //正常单词
			{
				i = 0;
				q->lchild = NULL;
				q->rchild = NULL;	 //初始化头节点的左右儿子为空指针

				if (p == NULL)
					p = root;
				ch = fgetc(fp);

				while (p != NULL)	//寻找待插入节点的位置
				{
					if (strcmp(q->data, p->data) < 0) //如果待插入的节点的值小于当前节点的值
					{
						if (p->lchild == NULL) //且其左子树为空,是一个新单词，则插入
						{
							q->n = 1;
							s = (Node_id *)malloc(sizeof(Node_id));
							s->id = identifier;
							s->num = 1;
							//s->max = file_size / 10;
							s->next = NULL;
							q->list = s;

							p->lchild = q;
							p = NULL;
						} //end if //并置当前节点为空，退出当前的while循环
						else
							p = p->lchild;
					}//end if// 否则继续访问其左子树
					else if (strcmp(q->data, p->data) > 0)
					{ //如果待插入的节点的值大于当前节点的值
						if (p->rchild == NULL)   //且其右子树为空，是一个新单词，则插入  
						{
							q->n = 1;
							s = (Node_id *)malloc(sizeof(Node_id));
							s->id = identifier;
							s->num = 1;
							//s->max = file_size / 10;
							s->next = NULL;
							q->list = s;

							p->rchild = q;
							p = NULL;
						}//end if  //并置当前节点为空，退出当前的while循环
						else
							p = p->rchild;
					} //end else //否则继续访问其右子树
					else //若待插入节点的值与当前节点的值相同
					{
						s = p->list;
						while (s != NULL)
						{
							if (s->id == identifier) { s->num++; break; }
							r = s;
							s = s->next;
						}//end while

						if (!s) //该词不是新词，但出现在新的文件中
						{
							s = (Node_id *)malloc(sizeof(Node_id));
							s->id = identifier;
							s->num = 1;
							//s->max = file_size / 10;
							s->next = NULL;
							r->next = s;

							p->n++;
						}//end if
						free(q);
						p = NULL;
					}//end else
				} //while
			}//end if
			q = (Ttree*)malloc(sizeof(Ttree));
			q->data[0] = '\0';
		} //else
	}//while
	return root;
}

/*中序遍历计算并输出权值 */
void InThread(Ttree *root, FILE *inv, Node_id *s, Node_id *begin, Node_id *end)
{
	if (root == NULL)
		return;

	InThread(root->lchild, inv, s, begin, end); //中序遍历二叉树左子树

	//if (root->n > max_f_w)
		//max_f_w = root->n; //包含某个关键词最多的文件数
	NUM_WORD++;

	//root->weight = log(1 + NF / (double)(root->n));
	begin = root->list;
	s = begin;
	if (NUM_WORD % 1 == 0)
	{
		if (i == NUM_TOKEN)
			return;
		s = root->list;
		//fprintf(inv,"%s\t%d>>>",root->data,root->n);
		fprintf(inv, "%s ", root->data);
		while (s != NULL)
		{
			fprintf(inv, "%d ", s->id);
			//fprintf(inv, "%.0f ", s->tf_idf );
			s = s->next;
		}
		fprintf(inv, "\n");
		i++;
	}
	InThread(root->rchild, inv, s, begin, end); //中序遍历二叉树右子树
}


void search(string searchkeyword) {
	char path[100] = { '0' };
	sprintf(path, "./indextable/index%d", NUM_READ);
	ifstream infile;
	infile.open(path);
	string s, s2;
	int temp;
	unsigned char *digest;

	digest = HMAC(EVP_sha1(), key, strlen(key), (const unsigned char*)searchkeyword.data(), strlen(searchkeyword.data()), NULL, NULL);
	char mdString[42] = { '0' };
	for (int i = 0; i < 20; i++)
		//outfile <<  (unsigned int)digest[i];
		sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
	while (!infile.eof())
	{
		getline(infile, s);//每次获取一行
		stringstream ss(s); //将获取的一行信息导入流中
		//ss >> str;
		ss >> s2;
		if (!strcmp((const char*)mdString, s2.data())) {
			cout << "find it keyword(plaintext)" << searchkeyword << endl;
			ss >> s2;
			for (vector<int>::size_type i = 0; i < NUM_READ; i++)
			{
				(s2.data()[i] + digest[i % 20]) % 2;//输出到文件
			}
			
			break;
		}
	}
	infile.close();

}

void build_index(vector<string>&keyword) {
	ifstream infile;//ifstream文件读操作，存储设备读取到内存中 
	ofstream outfile;//ofstream 文件写操作，内存写入存储设备 
	//ofstream keyword;
	char path[100] = { '0' };
	sprintf(path, "./output/inv%d", NUM_READ);
	infile.open(path); //要处理的文件名


	sprintf(path, "./indextable/index%d", NUM_READ);
	outfile.open(path);//输出的文件名
	//keyword.open("./output/keyword");
	if (!infile)
	{
		cerr << "error1" << endl;
		exit(1);
	}
	if (!outfile)
	{
		cerr << "error" << endl;
		exit(1);
	}

	const int col = NUM_READ; //列数，文件数
	string s;//临时字符串
	int temp;//临时数字
	string str;//第一列的字母
	vector<int> number(col, 0);//每行输出数组
	getline(infile, s);
	int j = 0;
	//keyword = (char**)malloc(100 * 10 * NUM_WORD);
	while (!infile.eof())
	{
		//每次获取一行
		//cout << s<<endl;  //输出测试
		stringstream ss(s); //将获取的一行信息导入流中
		ss >> str;
		unsigned char *digest;


		digest = HMAC(EVP_sha1(), key, strlen(key), (unsigned char*)str.data(), strlen(str.data()), NULL, NULL);

		char mdString[42] = { '0' };
		for (int i = 0; i < 20; i++) {

			sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
		}



		outfile << mdString << ' ';//第一列字母输出到文件
		(keyword[j++]) = str;
		//strcpy(keyword[j++], str.data());

		while (ss >> temp)// 字母后面的数字详情（0 1）
		{
			number[temp - 1] = 1; //有数字的置为1
		}

		for (vector<int>::size_type i = 0; i < number.size(); i++)
		{
			outfile << (number[i] + digest[i % 20]) % 2;//输出到文件
			number[i] = 0;//重置
		}
		getline(infile, s);
		outfile << endl;//输出到文件换行，表示此行处理完毕
	}
	infile.close();
	outfile.close();
	//keyword.close();
}

int main()
{
	FILE *fp, *inv;
	ofstream outputdata;
	outputdata.open("data.txt", ios::app);
	Ttree *root = NULL;
	Node_id *s = NULL, *begin = NULL, *end = NULL;
	char path[50];  //文件路径
	int i;
	struct stat buf;
	//outputdata= fopen("data.txt", "a+");
	time_t t;
	time(&t);
	//打印log时间信息
	//fprintf(outputdata, "INFO(%s) :\n", des);
	outputdata << "INFO: " << ctime(&t) ;
	cout << "INFO: " << ctime(&t)  ;

	sprintf(path, "./output/inv%d", NUM_READ);
	//把提取出来的关键词存储到该路径下
	inv = fopen(path, "w+");
	if (!inv)
	{
		printf("Cannot open the file 'inv'!\n");
		exit(0);
	}

	long long start = clock();
	for (i = 1; i <= NUM_READ; i++)
	{
		sprintf(path, "./DataSet/12000/%d", i);
		//打开这个路径下的前i个文件
		fp = fopen(path, "r");
		if (!fp)
		{
			printf("The file '%d' doesn't exist!\n", i);
			exit(0);
		}

		stat(path, &buf);
		//得到文件的信息
		//printf("id = %d\n", i);
		//gettimeofday(&start, NULL);
		root = createTtree(root, fp, i, buf.st_size);
		fclose(fp);
	}
	long long stop = clock();
	cout << "-----1. keyword extract time : " << (stop - start) * 1000 / CLOCKS_PER_SEC << "-----" << endl;
	outputdata << "-----1. keyword extract time : " << (stop - start) * 1000 / CLOCKS_PER_SEC << "-----" << endl;
	//t1=clock();
	start = clock();
	InThread(root, inv, s, begin, end);
	//t2=clock();

	fclose(inv);
	//printf("the time is %d \n",t2-t1);
	printf("The number of documents is NUM_READ=%d\n", NUM_READ);
	printf("The number of words is NUM_WORD=%d\n", NUM_WORD);
	outputdata << "The number of documents is: " << NUM_READ << endl;
	outputdata << "The number of words is: " << NUM_WORD << endl;

	vector<string>keyword(NUM_WORD);
	build_index(keyword);
	stop = clock();
	cout << "-----2. index build time : " << (stop - start) * 1000 / CLOCKS_PER_SEC << "-----"<< endl;
	outputdata << "-----2. index build time : " << (stop - start) * 1000 / CLOCKS_PER_SEC << "-----" << endl;







	long long aver_time=0;

	for (size_t i = 0; i < 10; i++)
	{
		start = clock();
		std::random_device r;

		// Choose a random mean between 1 and 6
		std::default_random_engine e1(r());
		std::uniform_int_distribution<int> uniform_dist(1, NUM_WORD - 1);
		int mean = uniform_dist(e1);

		srand(time(NULL));
		int index = rand() % NUM_WORD;
		printf("index:%d\n", mean);
		search(keyword[mean]);
		//fprintf(outputdata, "-----Search keyword: %s-----:\n", c_str(keyword[mean]));
		outputdata << "Search keyword:" << keyword[mean] << endl;
		stop = clock();
		aver_time += (stop - start) * 1000 / CLOCKS_PER_SEC;
		cout << "-----search time:(" << i<<")" << (stop - start) * 1000 / CLOCKS_PER_SEC << "-----" << endl;
	}
	//fprintf(outputdata, "3.-----(Average)search time: %lld-----:\n\n", aver_time/10);
	outputdata << "-----3. (average)search time : " << aver_time / 10 << "-----" << endl<<endl;
	//fclose(outputdata);
	outputdata.close();
	
	//free(keyword);
	return 0;
}



