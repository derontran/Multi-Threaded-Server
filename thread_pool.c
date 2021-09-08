#include "threadpool.h"

void threadpool_init(void *routine, void *dispatch, int thread_num)
{

   pool.total_thread = thread_num;
   pool.shutdown = 0;
   pool.function = routine;
   pool.threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);

   // lock for shutdown variable used in clean()
   sem_init(&(pool.semClean), 1);

   //initialize worker
   int i;
   for (i = 0; i < thread_num; i++)
   {
      int res = pthread_create(&(pool.threads[i]), NULL, dispatch, NULL);
      if (res != 0)
      {
         clean(&pool);
      }
   }
}
void clean(thread_pool *pool)
{
   sem_wait(&(pool->semClean));
   // set shutdown flag, indicate time to end, -1 match TERMINATION flag
   pool->shutdown = -1;
   // wake up sleeping thread, wrap up to exit
   //sem_broadcast(&pool->semInit);
   sem_post(&pool->semClean);

   int i;
   //ensure that all worker thread are done before clean up
   for (i = 0; i < pool->total_thread; i++)
   {
      pthread_join(pool->threads[i], NULL);
   }
   if (pool->threads)
   {
      free(pool->threads);
   }
   sem_destroy(&pool->semClean);
   exit(0);
}