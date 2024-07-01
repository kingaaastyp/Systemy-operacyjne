#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Deklaracja muteksów i zmiennych warunkowych
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Muteks kontrolujący dostęp do współdzielonych zasobów
pthread_cond_t cond_czytelnicy = PTHREAD_COND_INITIALIZER;  // Zmienna warunkowa dla czytelników
pthread_cond_t cond_pisarze = PTHREAD_COND_INITIALIZER;     // Zmienna warunkowa dla pisarzy

// Liczniki
int liczba_czytelnikow = 0;               // Liczba aktualnie przebywających czytelników w czytelni
int liczba_czytelnikow_oczekujacych = 0;  // Liczba czytelników oczekujących na dostęp do czytelni
int liczba_pisarzy_oczekujacych = 0;      // Liczba pisarzy oczekujących na dostęp do czytelni

// Funkcja wypisująca aktualny stan systemu
void print_status() {
    printf("ReaderQ: %d WriterQ: %d [in: R:%d W:0]\n", liczba_czytelnikow_oczekujacych, liczba_pisarzy_oczekujacych, liczba_czytelnikow);
}

// Funkcja wykonująca operacje czytelnika
void* czytelnik(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);  // Blokowanie muteksu

        liczba_czytelnikow_oczekujacych++;  // Zwiększenie liczby oczekujących czytelników
        print_status();                      // Wywołanie funkcji wypisującej stan systemu

        // Czytelnicy zawsze mogą wejść, blokujemy pisarzy
        liczba_czytelnikow++;               // Zwiększenie liczby aktualnie przebywających czytelników
        liczba_czytelnikow_oczekujacych--;  // Zmniejszenie liczby oczekujących czytelników
        print_status();                      // Ponowne wywołanie funkcji wypisującej stan systemu

        pthread_mutex_unlock(&mutex);        // Odblokowanie muteksu

        // Symulacja odczytu danych z bazy
        usleep(rand() % 1000000);            // Opóźnienie dla symulacji odczytu danych

        pthread_mutex_lock(&mutex);          // Ponowne blokowanie muteksu
        liczba_czytelnikow--;                // Zmniejszenie liczby aktualnie przebywających czytelników
        print_status();                      // Wywołanie funkcji wypisującej stan systemu

        // Sygnalizujemy pisarzom, jeśli nie ma aktywnych czytelników
        if (liczba_czytelnikow == 0 && liczba_pisarzy_oczekujacych > 0) {
            pthread_cond_signal(&cond_pisarze);  // Sygnał dla pisarzy, że mogą wejść
        } else {
            pthread_cond_signal(&cond_czytelnicy);  // Sygnał dla innych czytelników
        }

        pthread_mutex_unlock(&mutex);  // Odblokowanie muteksu

        // Czekanie przed ponownym wejściem
        usleep(rand() % 1000000);  // Opóźnienie przed ponowną próbą wejścia do czytelni
    }
    return NULL;
}

// Funkcja wykonująca operacje pisarza
void* pisarz(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);  // Blokowanie muteksu

        liczba_pisarzy_oczekujacych++;  // Zwiększenie liczby oczekujących pisarzy
        print_status();                 // Wywołanie funkcji wypisującej stan systemu

        // Pisarze mogą wejść, tylko gdy nie ma aktywnych czytelników
        while (liczba_czytelnikow > 0 || liczba_czytelnikow_oczekujacych > 0) {
            pthread_cond_wait(&cond_pisarze, &mutex);  // Oczekiwanie na sygnał od innych procesów
        }

        liczba_pisarzy_oczekujacych--;  // Zmniejszenie liczby oczekujących pisarzy
        print_status();                  // Wywołanie funkcji wypisującej stan systemu

        // Symulacja zapisu danych do bazy (nigdy się nie wydarzy)
        usleep(rand() % 1000000);  // Opóźnienie dla symulacji zapisu danych

        pthread_mutex_unlock(&mutex);  // Odblokowanie muteksu

        // Czekanie przed ponownym wejściem
        usleep(rand() % 1000000);  // Opóźnienie przed ponowną próbą wejścia do czytelni
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    // Sprawdzenie, czy liczba argumentów jest prawidłowa
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_readers> <num_writers>\n", argv[0]);
        return 1;
    }

    int ilosc_czyt = atoi(argv[1]);  // Pobranie liczby czytelników z argumentów
    int ilosc_pis = atoi(argv[2]);   // Pobranie liczby pisarzy z argumentów

    pthread_t c[ilosc_czyt], p[ilosc_pis];  // Tablice wątków czytelników i pisarzy

    // Tworzenie wątków czytelników
    for (int i = 0; i < ilosc_czyt; i++) {
        pthread_create(&c[i], NULL, czytelnik, NULL);
    }

    // Tworzenie wątków pisarzy
    for (int i = 0; i < ilosc_pis; i++) {
        pthread_create(&p[i], NULL, pisarz, NULL);
    }

    // Czekanie na zakończenie wątków
    for (int i = 0; i < ilosc_pis; i++) {
        pthread_join(p[i], NULL);
    }
    for (int i = 0; i < ilosc_czyt; i++) {
        pthread_join(c[i], NULL);
    }

    // Zwalnianie zasobów
    pthread_mutex_destroy(&mutex);          // Zniszczenie muteksu
    pthread_cond_destroy(&cond_czytelnicy); // Zniszczenie zmiennej warunkowej dla czytelników
    pthread_cond_destroy(&cond_pisarze);    // Zniszczenie zmiennej warunkowej dla pisarzy

    return 0;
}
