#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

pthread_mutex_t m;
pthread_cond_t camac_str;
pthread_cond_t red[2];
pthread_cond_t usli;

typedef struct {
  int misionara;
  int kanibala;
  int rbr_misionara;
  int rbr_kanibala;
  int ukrcano;
  int strana_camca;
  int m[2];
  int k[2];
  char na_camcu[100];
  char lijevo[100];
  char desno[100];
} data;

int id;
data *d;

void ispis_stanja() {
  if (d->strana_camca == 0) {
    printf("C[D]={%s } LO={%s } DO={%s }\n", d->na_camcu, d->lijevo, d->desno);
  } else {
    printf("C[L]={%s } LO={%s } DO={%s }\n", d->na_camcu, d->lijevo, d->desno);
  }
  printf("\n");
}

void camac() {
  while (1) {
    pthread_mutex_lock(&m);
    if (d->ukrcano == 0) {
      if (d->strana_camca == 1) {
        printf("C: prazan na lijevoj obali\n");
      } else {
        printf("C: prazan na desnoj obali\n");
      }
    }
    ispis_stanja();

    while (d->ukrcano < 3) {
      pthread_cond_wait(&camac_str, &m);
    }
    printf("C: tri putnika ukrcana, polazim za jednu sekundu\n");
    ispis_stanja();
    pthread_cond_broadcast(&red[1 - d->strana_camca]);
    pthread_mutex_unlock(&m);
    sleep(1);
    if (d->strana_camca == 1) {
      printf("C: prevozim s lijeve na desnu obalu: %s\n\n", d->na_camcu);
    } else {
      printf("C: prevozim s desne na lijevu obalu: %s\n\n", d->na_camcu);
    }
    int j = d->strana_camca;
    d->strana_camca = -1;
    sleep(2);

    pthread_mutex_lock(&m);
    d->strana_camca = j;
    if (d->strana_camca == 1) {
      printf("C: preveo s lijeve na desnu obalu: %s\n\n", d->na_camcu);
    } else {
      printf("C: preveo s desne na lijevu obalu: %s\n\n", d->na_camcu);
    }
    pthread_cond_broadcast(&usli);
    memset(d->na_camcu, '\0', sizeof(d->na_camcu));
    d->ukrcano = 0;
    d->kanibala = 0;
    d->misionara = 0;
    d->strana_camca = 1 - j;
    pthread_cond_broadcast(&red[1 - d->strana_camca]);
    pthread_mutex_unlock(&m);
  }
}

void misionar() {
  pthread_mutex_lock(&m);
  d->rbr_misionara++;
  int rbr = d->rbr_misionara;
  int s = rand() % 2;
  char a[5] = " M";
  char broj[5];
  sprintf(broj, "%d", rbr);
  strcat(a, broj);
  if (s == 1) {
    printf("M%d: došao na lijevu obalu\n", rbr);
    strcat(d->lijevo, a);
    ispis_stanja();
    d->m[0] = d->m[0] + 1;
  } else {
    printf("M%d: došao na desnu obalu\n", rbr);
    strcat(d->desno, a);
    ispis_stanja();
    d->m[1] = d->m[1] + 1;
  }
  while ((d->ukrcano >= 7 || (d->kanibala - 1) > d->misionara ||
          s != d->strana_camca) &&
         (d->m[!s] + d->misionara < d->kanibala || s != d->strana_camca)) {
    if (s == 1)
      pthread_cond_wait(&red[0], &m);
    else
      pthread_cond_wait(&red[1], &m);
  }
  printf("M%d: ušao u čamac\n", rbr);
  strcat(d->na_camcu, a);
  int z = 1;
  if (rbr > 9)
    z = 2;
  if (s == 1) {
    char *pos = strstr(d->lijevo, a);
    memmove(pos, pos + z + 2, strlen(pos + z + 2) + 1);
    d->m[0] = d->m[0] - 1;
  } else {
    char *pos = strstr(d->desno, a);
    memmove(pos, pos + z + 2, strlen(pos + z + 2) + 1);
    d->m[1] = d->m[1] - 1;
  }
  d->misionara++;
  d->ukrcano++;
  ispis_stanja();
  pthread_cond_broadcast(&red[d->strana_camca]);
  pthread_cond_signal(&camac_str);
  pthread_cond_wait(&usli, &m);
  pthread_mutex_unlock(&m);
  return;
}

void kanibal() {
  pthread_mutex_lock(&m);
  d->rbr_kanibala++;
  int rbr = d->rbr_kanibala;
  int s = rand() % 2;
  char a[5] = " K";
  char broj[5];
  sprintf(broj, "%d", rbr);
  strcat(a, broj);
  if (s == 1) {
    printf("K%d: došao na lijevu obalu\n", rbr);
    strcat(d->lijevo, a);
    ispis_stanja();
    d->k[0] = d->k[0] + 1;
  } else {
    printf("K%d: došao na desnu obalu\n", rbr);
    strcat(d->desno, a);
    ispis_stanja();
    d->k[1] = d->k[1] + 1;
  }
  while (d->ukrcano >= 7 ||
         (d->kanibala >= d->misionara && d->misionara != 0) ||
         s != d->strana_camca) {
    if (s == 1)
      pthread_cond_wait(&red[0], &m);
    else
      pthread_cond_wait(&red[1], &m);
  }
  printf("K%d: ušao u čamac\n", rbr);
  strcat(d->na_camcu, a);
  int z = 1;
  if (rbr > 9)
    z = 2;
  if (s == 1) {
    char *pos = strstr(d->lijevo, a);
    memmove(pos, pos + z + 2, strlen(pos + z + 2) + 1);
    d->k[0] = d->k[0] - 1;
  } else {
    char *pos = strstr(d->desno, a);
    memmove(pos, pos + z + 2, strlen(pos + z + 2) + 1);
    d->k[1] = d->k[1] - 1;
  }
  d->kanibala++;
  d->ukrcano++;
  ispis_stanja();
  pthread_cond_signal(&camac_str);
  pthread_cond_wait(&usli, &m);
  pthread_mutex_unlock(&m);
  return;
}

int main() {
  srand(time(0));
  printf("Legenda: M - misionar, K - kanibal, C - čamac,\n         LO - lijeva "
         "obala, DO - desna obala\n         L - lijevo, D - desno\n\n");
  pthread_t t[30];
  int i = 0;
  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&camac_str, NULL);
  pthread_cond_init(&red[0], NULL);
  pthread_cond_init(&red[1], NULL);
  pthread_cond_init(&usli, NULL);

  id = shmget(IPC_PRIVATE, sizeof(data), 0600);
  d = (data *)shmat(id, NULL, 0);

  pthread_create(&t[0], NULL, (void *)camac, NULL);

  while (1) {
    pthread_create(&t[(i++) % 30], NULL, (void *)misionar, NULL);
    pthread_create(&t[(i++) % 30], NULL, (void *)kanibal, NULL);
    sleep(1);
    pthread_create(&t[(i++) % 30], NULL, (void *)kanibal, NULL);
    sleep(1);
  }

  for (int i = 0; i < 30; i++) {
    pthread_join(t[i], NULL);
  }
  pthread_mutex_destroy(&m);
  pthread_cond_destroy(&camac_str);
  pthread_cond_destroy(&red[0]);
  pthread_cond_destroy(&red[1]);
  pthread_cond_destroy(&usli);

  shmdt((char *)d);
  shmctl(id, IPC_RMID, NULL);

  return 0;
}