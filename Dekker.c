#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int idA, idPravo, idZastavica;
int *A;
int *pravo;
int *zastavica;
int M;

void proces() {
  int i;
  for (i = 0; i < M; i++) {
    *zastavica = 1;
    while (*(zastavica + 1) != 0) {
      if (*pravo == 1) {
        *zastavica = 0;
        while (*pravo == 1) {
        }
        *zastavica = 1;
      }
    }
    (*A)++;
    // printf("Dijete, A = %d\n", *A);
    zastavica[0] = 0;
    *pravo = 1;
  }
  exit(1);
}

int main(int argc, char *argv[]) {

  M = atoi(argv[1]);

  idA = shmget(IPC_PRIVATE, sizeof(int), 0600);
  idPravo = shmget(IPC_PRIVATE, sizeof(int), 0600);
  idZastavica = shmget(IPC_PRIVATE, 2 * sizeof(int), 0600);

  if (idA == -1 || idPravo == -1 || idZastavica == -1)
    exit(1);

  A = (int *)shmat(idA, NULL, 0);
  pravo = (int *)shmat(idPravo, NULL, 0);
  zastavica = (int *)shmat(idZastavica, NULL, 0);
  *A = 0;
  *pravo = 0;
  *zastavica = *(zastavica + 1) = 0;

  switch (fork()) {
  case -1: {
    printf("Nije uspjelo stvaranje djeteta!");
    return 1;
  }
  case 0: {
    proces();
  } break;

  default:
    break;
  }

  int j;
  for (j = 0; j < M; j++) {
    *(zastavica + 1) = 1;
    while (*zastavica != 0) {
      if (*pravo == 0) {
        *(zastavica + 1) = 0;
        while (*pravo == 0) {
        }
        *(zastavica + 1) = 1;
      }
    }
    (*A)++;
    // printf("Roditelj, A = %d\n", *A);
    *(zastavica + 1) = 0;
    *pravo = 0;
  }

  wait(NULL);
  printf("A = %d\n", *A);
  shmdt((char *)A);
  shmdt((char *)pravo);
  shmdt((char *)zastavica);
  shmctl(idA, IPC_RMID, NULL);
  shmctl(idPravo, IPC_RMID, NULL);
  shmctl(idZastavica, IPC_RMID, NULL);
  return 0;
}