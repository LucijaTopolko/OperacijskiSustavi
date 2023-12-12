#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct timespec t0; /* vrijeme pocetka programa */

/* postavlja trenutno vrijeme u t0 */
void postavi_pocetno_vrijeme() { clock_gettime(CLOCK_REALTIME, &t0); }

/* dohvaca vrijeme proteklo od pokretanja programa */
void vrijeme() {
  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  t.tv_sec -= t0.tv_sec;
  t.tv_nsec -= t0.tv_nsec;
  if (t.tv_nsec < 0) {
    t.tv_nsec += 1000000000;
    t.tv_sec--;
  }

  printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec / 1000000);
}

/* ispis kao i printf uz dodatak trenutnog vremena na pocetku */
#define PRINTF(format, ...)                                                    \
  do {                                                                         \
    vrijeme();                                                                 \
    printf(format, ##__VA_ARGS__);                                             \
  } while (0)

int k_zastavice[3] = {0, 0, 0};
int tek_prioritet = 0;

typedef struct {
  int elementi[4];
  int top;
} Stog;

Stog stog;

void inicijaliziraj(Stog *s) { s->top = -1; }
int is_empty(Stog *s) { return s->top == -1; }
void push(Stog *s, int sig) { s->elementi[++s->top] = sig; }
int pop(Stog *s) {
  if (is_empty(&stog)) {
    return -1;
  } else {
    return s->elementi[s->top--];
  }
}

void stanje() {
  PRINTF("K_Z=%d%d%d, T_P=%d, stog: ", k_zastavice[0], k_zastavice[1],
         k_zastavice[2], tek_prioritet);
  if (is_empty(&stog))
    printf("-\n");
  else {
    for (int i = stog.top; i >= 0; i--) {
      printf("%d, reg[%d]", stog.elementi[i], stog.elementi[i]);
      if (i > 0)
        printf("; ");
    }
  }
  printf("\n");
}

void obradi_signal(int sig);

void obrada(int sig) {
  PRINTF("Počela obrada prekida razine %d\n", sig);
  tek_prioritet = sig;
  k_zastavice[sig - 1] = 0;
  stanje();
  printf("\n");
  

  
  for (int i = 0; i < 10; i++) {
	    if (k_zastavice[2] && tek_prioritet <= 2) {
			PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 3 procesoru\n");
      push(&stog, tek_prioritet);
      obrada(3);
    }
  if (k_zastavice[1] && tek_prioritet <= 1) {
	  	PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 2 procesoru\n");
      push(&stog, tek_prioritet);
      obrada(2);
    }
  if (k_zastavice[0] && tek_prioritet <= 0) {
	  	PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 1 procesoru\n");
      push(&stog, tek_prioritet);
      obrada(1);
    }
    sleep(1);
  }
  PRINTF("Završila obrada prekida razine %d\n", sig);
  tek_prioritet = pop(&stog);
  if (tek_prioritet > 0) {
    PRINTF("Nastavlja se obrada prekida razine %d\n", tek_prioritet);
    stanje();
  } else {
    tek_prioritet = 0;
    PRINTF("Nastavlja se izvođenje glavnog programa\n");
    stanje();
  }
}

void obradi_signal(int sig) {
  k_zastavice[sig - 1] = 1; // podignem zastavicu
  if (sig > tek_prioritet) {
    PRINTF("SKLOP: Dogodio se prekid razine %d i prosljeđuje se procesoru\n",
           sig);
    stanje();
    push(&stog, tek_prioritet);
    obrada(sig);
  } else {
    PRINTF("SKLOP: Dogodio se prekid razine %d, ali on se pamti i ne "
           "prosljeđuje procesoru\n",
           sig);
    stanje();
    printf("\n");
  }
}

void obradi_sigusr1() { obradi_signal(1); }
void obradi_sigterm() { obradi_signal(2); }
void obradi_sigint() { obradi_signal(3); }

int main() {
  inicijaliziraj(&stog);
  struct sigaction act;

  postavi_pocetno_vrijeme();

  PRINTF("Program s PID=%ld krenuo s radom\n", (long)getpid());
  stanje();

  /* 1. maskiranje signala SIGUSR1 */
  /* kojom se funkcijom signal obradjuje */
  act.sa_handler = obradi_sigusr1;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0; /* naprednije mogucnosti preskocene */
  sigaction(SIGUSR1, &act,
            NULL); /* maskiranje signala - povezivanje sucelja OS-a */

  /* 2. maskiranje signala SIGTERM */
  act.sa_handler = obradi_sigterm;
  sigemptyset(&act.sa_mask);
  sigaction(SIGTERM, &act, NULL);

  /* 3. maskiranje signala SIGINT */
  act.sa_handler = obradi_sigint;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);

  while (1) {
    if (k_zastavice[2]) {
		PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 3 procesoru\n");
		push(&stog, tek_prioritet);
      obrada(3);
    }
    if (k_zastavice[1]) {
		PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 2 procesoru\n");
		push(&stog, tek_prioritet);
      obrada(2);
    }
    if (k_zastavice[0]) {
		PRINTF("SKLOP: promijenio se T_P, prosljeđuje prekid razine 1 procesoru\n");
		push(&stog, tek_prioritet);
      obrada(1);
    }
  }
}