#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"
//last1
struct node {
  int value;
  struct node* next ;
  pthread_mutex_t m;

};

struct list {
  struct node* head;
  pthread_mutex_t m1;
};


void print_node(node* node)
{
  // DO NOT DELETE
  if(node)
  {
    printf("%d ", node->value);
  }
}

list* create_list()
{
  struct list* s;

  if ((s=(struct list*)malloc (sizeof (struct list)))==NULL)
  {
    printf ("Cannot allocate memory\n");
    exit (1);
  }
  if((pthread_mutex_init(&s->m1,NULL))!=0)
  {
      free(s);
      exit(1);
  }
   s->head=NULL;
  return s;
}

void delete_list(list* list)
{

  if(list!=NULL)
  {
    if(list->head!=NULL)
    {
        int val;
      struct node* p1=list->head;

      while(p1!=NULL)
      {
          val=p1->value;
          p1=p1->next;
         remove_value(list,val);
      }
    }

    pthread_mutex_destroy(&list->m1);
    free(list);
  }
}



void insert_value(list* list, int value)
{
      if(list!=NULL){


  struct node* p1;
  struct node* p2;

  if ((p1=(struct node*)malloc(sizeof (struct node)))==NULL) {
    printf ("Cannot allocate memory\n");
    exit (1);
  }
  if((pthread_mutex_init(&p1->m,NULL))!=0)
  {
      free(p1);
      exit(1);
  }
  p1->value=value;

  if(list->head!=NULL)
  pthread_mutex_lock(&list->head->m);

  else {
        pthread_mutex_lock(&list->m1);}

  struct node* head=list->head;

  if (head==NULL )
  {
    p1->next=NULL;
    list->head=p1;
    pthread_mutex_unlock(&list->m1);

    return;
  }







  if(head->value >= value)
  {
    p1->next=list->head;
    list->head=p1;
    pthread_mutex_unlock(&list->head->next->m);
    return;

  }
  else
  {
    p2=list->head;
    while (p2->next!=NULL && p2->next->value < value) {
      pthread_mutex_lock(&p2->next->m);
      pthread_mutex_unlock(&p2->m);
      p2=p2->next ;
    }
    p1->next=p2->next;
    p2->next=p1;
    pthread_mutex_unlock(&p2->m);


  }
  }
}

void remove_value(list* list, int value)
{
  if (list!=NULL && list->head!=NULL)
  {
    pthread_mutex_lock(&list->head->m);
    struct node* current=list->head->next;
    struct node* previous=list->head;
    if(value==list->head->value)
    {
      list->head=current;
      pthread_mutex_unlock(&previous->m);
      pthread_mutex_destroy(&previous->m);
      free(previous);

      return;
    }

    else if(current!=NULL){
    pthread_mutex_lock(&current->m);
    while(current->next!=NULL && current->value < value)
    {
      pthread_mutex_lock(&current->next->m);
      pthread_mutex_unlock(&previous->m);
      previous=current;
      current=current->next;
    }
    if(current->value == value)
    {
      previous->next=current->next;
      pthread_mutex_unlock(&current->m);
      pthread_mutex_destroy(&current->m);
      pthread_mutex_unlock(&previous->m);
      free(current);
    }
    else
    {
      pthread_mutex_unlock(&current->m);
      pthread_mutex_unlock(&previous->m);
    }
    }
  }
  }


void print_list(list* list)
{

  if(list!=NULL &&list->head!=NULL){
  pthread_mutex_lock(&list->head->m);
  struct node* head=list->head;
  while(head!=NULL)
  {
    print_node(head);
    if(head->next!=NULL)
    pthread_mutex_lock(&head->next->m);
    pthread_mutex_unlock(&head->m);
    head=head->next;
  }
  }
  printf("\n"); // DO NOT DELETE


}

void count_list(list* list, int (*predicate)(int))
{
  int count = 0; // DO NOT DELETE
  if(list!=NULL && list->head!=NULL){
  pthread_mutex_lock(&list->head->m);
  struct node* p=list->head;
  while(p!=NULL)
  {
    count+=predicate(p->value);

    if(p->next!=NULL)
    pthread_mutex_lock(&p->next->m);

    pthread_mutex_unlock(&p->m);
    p=p->next;

  }

  }
  printf("%d items were counted\n", count); // DO NOT DELETE
}
