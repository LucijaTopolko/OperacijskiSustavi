#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 3
#define radno_vrijeme 16
#define vrijeme_frizure 2
#define pauza 6
sem_t *frizerka_radi;
sem_t *ima_klijenata;
sem_t *otvoreno;
sem_t *ima_mjesta;
sem_t *spreman;
sem_t *spreman1;

int id1, id2, id3, id4, id5, id6;

int id_br, id_klijenata, id_trenutni, id_otv, id_i;
int *br, *klijenata, *trenutni, *otv, *ii;

void frizerka() {
  printf("Frizerka: Otvaram salon\n");
  printf("Frizerka: Postavljam znak OTVORENO\n");
  int c = 1;
  while (1) {
    sem_wait(otvoreno);
    int o = *otv;
    sem_post(otvoreno);

    if (o == 0 && c) {
      printf("Frizerka: Postavljam znak ZATVORENO\n");
      c = 0;
    }
    if (sem_trywait(ima_klijenata) == 0) {
      sem_post(frizerka_radi);
      sem_wait(spreman);
      printf("Frizerka: Idem raditi na klijentu %d\n", *trenutni);
      sem_post(spreman1);
      sleep(vrijeme_frizure);
      printf("Frizerka: Klijent (%d) gotov\n", *trenutni);
    } else if (o == 1) {
      printf("Frizerka: Spavam dok klijenti ne dođu\n");
      sem_wait(ima_klijenata);
      o = *otv;
      sem_post(ima_klijenata);
    }
    if (o == 0 && sem_trywait(ima_klijenata) == -1) {
      printf("Frizerka: Zatvaram salon\n");
      exit(0);
    }
  }
}

void klijent() {
  int i = *br;
  printf("   Klijent(%d): Želim na frizuru\n", i);

  *ii = *ii + 1;
  sem_wait(otvoreno);
  int o = *otv;
  sem_post(otvoreno);
  if (sem_trywait(ima_mjesta) == 0 && o == 1) {
    *klijenata = *klijenata + 1;
    printf("   Klijent(%d): Ulazim u čekaonicu (%d)\n", i, *klijenata);
    sem_post(ima_klijenata);
    sem_wait(frizerka_radi);
    *trenutni = i;
    *klijenata = *klijenata - 1;
    sem_post(spreman);
    sem_wait(spreman1);
    sem_post(ima_mjesta);
    printf("   Klijent(%d): frizerka mi radi frizuru\n", i);
  } else {
    printf("   Klijent(%d): Nema mjesta u čekaoni, vratit ću se sutra\n", i);
  }
  exit(0);
}

int main() {

  id_br = shmget(IPC_PRIVATE, sizeof(int), 0600);
  id_klijenata = shmget(IPC_PRIVATE, sizeof(int) * (N + 2), 0600);
  id_trenutni = shmget(IPC_PRIVATE, sizeof(int), 0600);
  id_otv = shmget(IPC_PRIVATE, sizeof(int), 0600);

  id1 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
  id2 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
  id3 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
  id4 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
  id5 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
  id6 = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);

  id_i = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);

  br = shmat(id_br, NULL, 0);
  *br = 0;
  ii = shmat(id_i, NULL, 0);
  *ii = 0;
  klijenata = shmat(id_klijenata, NULL, 0);
  *klijenata = 0;
  trenutni = shmat(id_trenutni, NULL, 0);
  *trenutni = 0;
  otv = shmat(id_otv, NULL, 0);
  *otv = 1;
  frizerka_radi = shmat(id1, NULL, 0);
  ima_klijenata = shmat(id2, NULL, 0);
  otvoreno = shmat(id3, NULL, 1);
  ima_mjesta = shmat(id4, NULL, 0);
  spreman = shmat(id5, NULL, 0);
  spreman1 = shmat(id6, NULL, 0);

  sem_init(frizerka_radi, 1, 0);
  sem_init(ima_klijenata, 1, 0);
  sem_init(otvoreno, 1, 1);
  sem_init(ima_mjesta, 1, N);
  sem_init(spreman, 1, 0);
  sem_init(spreman1, 1, 0);
  sem_post(otvoreno);
  if (fork() == 0) {
    frizerka();
  }
  for (int i = 0; i < 5; i++) {
    if (fork() == 0) {
      *br = *br + 1;
      klijent();
    }
    sleep(1);
  }
  sleep(pauza);
  for (int i = 0; i < 2; i++) {
    if (fork() == 0) {
      *br = *br + 1;
      klijent();
    }
    sleep(1);
  }
  sleep(radno_vrijeme - pauza);
  sem_wait(otvoreno);
  *otv = 0;
  sem_post(otvoreno);
  sem_post(ima_klijenata);
  for (int i = 0; i < *ii; i++) {
    wait(NULL);
  }
  sem_destroy(frizerka_radi);
  sem_destroy(ima_klijenata);
  sem_destroy(otvoreno);
  sem_destroy(ima_mjesta);
  sem_destroy(spreman);
  sem_destroy(spreman1);
  shmdt(frizerka_radi);
  shmdt(ima_klijenata);
  shmdt(otvoreno);
  shmdt(ima_mjesta);
  shmdt(spreman);
  shmdt(spreman1);
  shmctl(id1, IPC_RMID, NULL);
  shmctl(id2, IPC_RMID, NULL);
  shmctl(id3, IPC_RMID, NULL);
  shmctl(id4, IPC_RMID, NULL);
  shmctl(id5, IPC_RMID, NULL);
  shmctl(id6, IPC_RMID, NULL);
  shmdt((char *)br);
  shmctl(id_br, IPC_RMID, NULL);
  shmdt((char *)klijenata);
  shmctl(id_klijenata, IPC_RMID, NULL);
  shmdt((char *)trenutni);
  shmctl(id_trenutni, IPC_RMID, NULL);
  shmdt((char *)otv);
  shmctl(id_otv, IPC_RMID, NULL);
  shmdt((char *)ii);
  shmctl(id_i, IPC_RMID, NULL);
  return 0;
}