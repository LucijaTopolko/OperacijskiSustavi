#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

atomic_int *A;
atomic_int N, M;
int id;
atomic_int *ulaz, *broj;

void dretva(int br) {
  for (int z = 0; z < M; z++) {
    ulaz[br] = 1;
    broj[br] = broj[br - 1] + 1;
    ulaz[br] = 0;

    for (int j = 0; j < N; j++) {
      while (ulaz[j]) {
      }
      while (broj[j] != 0 &&
             (broj[j] < broj[br] || (broj[j] == broj[br] && j < br))) {
      }
    }
    atomic_fetch_add(A, 1);
    broj[br] = 0;
  }
}

int main(int argc, char *argv[]) {
  N = atoi(argv[1]);
  M = atoi(argv[2]);
  // printf("%d %d\n", N, M);
  pthread_t t[N];

  ulaz = (atomic_int *)malloc(N * sizeof(atomic_int));
  broj = (atomic_int *)malloc(N * sizeof(atomic_int));

  id = shmget(IPC_PRIVATE, sizeof(atomic_int), 0666);

  if (id == -1)
    exit(1);

  A = (atomic_int *)shmat(id, NULL, 0);
  *A = 0;

  for (int i = 0; i < N; i++) {
    ulaz[i] = 0;
    broj[i] = 0;

    if (pthread_create(&t[i], NULL, (void *)dretva, (void *)i)) {
      printf("Ne mogu stvoriti novu dretvu!");
      exit(1);
    }
  }

  for (int i = 0; i < N; i++)
    pthread_join(t[i], NULL);

  printf("A = %d\n", *A);
  shmdt((char *)A);
  shmctl(id, IPC_RMID, NULL);
  return 0;
}