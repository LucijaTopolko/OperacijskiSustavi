import random
import time
from sys import maxsize
import numpy as np

N = int(input("Unesi broj procesa: "))
M= int(input("Unesi broj okvira: "))
redakTab=0
def dohvati_fizicku_adresu(p, x):
    index_retka = x >> 6
    sadrzaj_retka = tablica[p][index_retka]
    fizAdresa = (sadrzaj_retka & 0xffc0) | (x & 0x3f)
    return fizAdresa

def dohvati_sadrzaj(p, x):
    global redakTab
    y = dohvati_fizicku_adresu(p, x)
    i = okvir[(y >> 6) & 0x3ff][y & 0x3f]
    
    sadrzaj_retka = tablica[p][x >> 6]
    if (sadrzaj_retka & 0x20) == 0:
        print("         Promašaj!")
        prazan_okvir = Prazni(okvir)
        if prazan_okvir == -1:
            najmanji_lru = maxsize
            index_najmanjeg = -1
            indexIzbacenog = -1
            for z in range(N):
                    for j in range(16):
                        if ( tablica[z][j] >> 5) & 1 == 1:
                            lru = tablica[z][j] & 0x1f
                            if lru <= najmanji_lru:
                                najmanji_lru = lru
                                index_najmanjeg = j
                                indexIzbacenog = z

            prazan_okvir = (tablica[indexIzbacenog][index_najmanjeg] >> 6) % M
            print("             izbacujem stranicu 0x" + f"{(x & 0x3c0):04x}" + " iz procesa "+ str(indexIzbacenog))
            print("             lru izbacene stranice: 0x" + f"{najmanji_lru:04x}")            
            for j in range(16):
                if (tablica[indexIzbacenog][j] >> 6) == prazan_okvir:
                    tablica[indexIzbacenog][j] = tablica[indexIzbacenog][j] & 0

            for j in range(64):
                disk[indexIzbacenog][x >> 6][j] = okvir[prazan_okvir][j]

        print("             dodjeljen okvir 0x" + f"{prazan_okvir:04x}")
        
        for j in range(64):
            okvir[prazan_okvir][j] = disk[p][x >> 6][j]

        redakTab = (prazan_okvir << 6) | (t) | 0x20
    else:
        print("         Pogodak!")
        redakTab = (tablica[p][x >> 6] | t) | 0x20

    print("         fiz. adresa: 0x" + f"{y:04x}")
    print("         zapis tablice: " + f"{redakTab:04x}")
    
    return i

def zapis_vrijednost(p, x, i):
    y = dohvati_fizicku_adresu(p, x)
    okvir[(y >> 6) & 0x3ff][y & 0x3f] = i

def Prazni(okvir):
    for i in range(M):
        empty = True
        for j in range(64):
            if okvir[i][j] != 0:
                empty = False
                break
        if empty:
            return i
    return -1


tablica = np.zeros((N,16), dtype=int)
okvir = np.zeros((M,64), dtype=int)
disk = np.zeros((N,16,64), dtype=int)

t = 0
while True:
    for p in range(N):
        

        print("-------------------------------------------")
        print("proces: "+ str(p))
        print("         t: " + str(t))
        #x = 0x1fe
        x = random.randint(0,0xffff) & 0x03fe #nasumična logička adresa
        print("         log. adresa: 0x" + f"{x:04x}")

        i = dohvati_sadrzaj(p, x)
        i += 1
        zapis_vrijednost(p, x, i)
        
        sadrzaj_adrese = okvir[(dohvati_fizicku_adresu(p,x) >> 6) & 0x3ff][dohvati_fizicku_adresu(p,x) & 0x3f]
        print("         sadrzaj adrese: " + str(sadrzaj_adrese))
        tablica[p][x >> 6]= redakTab
        t += 1
        if t==32: #vraćanje na 0 jer imam samo 5 bita
            t=0
            for a in range (N):
                for b in range(16):
                    tablica[a][b]=tablica[a][b] & 0xffe0 #micanje LRU metapodatka i bita prisutnosti
        time.sleep(1)
