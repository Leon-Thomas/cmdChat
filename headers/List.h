#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *	Data;

/*链表结点结构*/
typedef struct _ListNode
{
	Data data;
	struct _ListNode *next;
}ListNode;
//链表头结构
typedef struct _List
{
	size_t lt_len;
	ListNode *head;
	ListNode *rear;
}List;

//链表结点数据类型
typedef ListNode Lnode;

//索引数据类型
typedef ssize_t Index;

/*链表元素个数*/
//size_t listSize(List *list);
#define listSize(listp) \
    ((listp)->lt_len)
/*初始化链表*/
void listInit(List *list);

/*插入一个数据到链表中*/
bool listInsert(List *list,Index i, Data d,size_t dsize);

/*链尾追加一个元素*/
bool listAppend(List *list,Data d,size_t dsize);

/*删除值为d的结点，返回true , false*/
bool listDelByData(List *list,Data d,size_t dsize);

/*删除索引为i的结点，返回true , false*/
bool listDelByIndex(List *list,Index i);

/*返回索引i元素的值，索引失败返回一个无效的(NULL)数据*/
Data listGet(List *list,Index i);

/*设置索引为i元素的值*/
bool listSet(List *list,Index i,Data d,size_t dsize);

/*原地将链表逆序*/
void listReverse(List *list);

/*回收链表的内存*/
void destroyList(List *list);


#ifdef __cplusplus
}
#endif

#endif //LIST_H
