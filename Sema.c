#include "Sema.h"

// only one thread can call this
void sem_init(sem *s, int value)
{
   s->value = value;
   pthread_cond_init(&s->cond, NULL);
   pthread_mutex_init(&s->lock, NULL);
}

void sem_wait(sem *s)
{
   pthread_mutex_lock(&s->lock);
   while (s->value <= 0)
   {
      pthread_cond_wait(&s->cond, &s->lock);
   }

   s->value--;
   pthread_mutex_unlock(&s->lock);
}

void sem_post(sem *s)
{
   pthread_mutex_lock(&s->lock);
   s->value++;
   pthread_cond_signal(&s->cond);
   pthread_mutex_unlock(&s->lock);
}

void sem_broadcast(sem *s)
{
   pthread_cond_broadcast(&s->cond);
}

void sem_destroy(sem *s)
{
   pthread_mutex_destroy(&s->lock);
   pthread_cond_destroy(&s->cond);
}