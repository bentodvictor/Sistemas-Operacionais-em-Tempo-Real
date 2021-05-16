int buffer = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;


void *thread_A(){
  while (1) {
    pthread_mutex_lock(&m);
    buffer++;
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&m);
  }
}

void *thread_B(){
  while (1) {
    pthread_mutex_lock(&m);
    while(buffer < 1)
      pthread_cond_wait(&c, &m);
    buffer--;
    pthread_mutex_unlock(&m);
  }
}