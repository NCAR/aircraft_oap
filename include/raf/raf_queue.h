#ifndef QUEUE_H
#define QUEUE_H

struct dat
	{
	void	*next;
	void	*datum;
	};

typedef struct
	{
	struct dat	*head;
	struct dat	*tail;
	} Queue;

#ifdef __cplusplus
extern "C" {
#endif
Queue *CreateQueue();
void EnQueue(Queue *q, void *datum);
void *FrontQueue(Queue *q);
void *DeQueue(Queue *q);
void DestroyQueue(Queue *q);
#ifdef __cplusplus
}
#endif

#endif
