#include <iostream>
#include <thread>
#include <condition_variable>
#include <queue>
#include <vector>

const int liczba_drukarek = 4;

using namespace std;

//klasa monitor konsument-producent
class Monitor_PK {
private:
    //u�ywam kolejek, bo wygodniejszy dost�p do danych
    queue<int> drukarki; //kolejka dla drukarek
    queue<int> bufor_drukarki[liczba_drukarek]; //bufor
    mutex mtx_synchro; //zmienna mutex, synchronizacja z w�tkami 
    condition_variable cv_synchro; //wsp�pracuje z mutex, te� synchronizacja w�tk�w

public:
    //konstruktor podstawowy
    Monitor_PK() {
        for (int i = 0; i < liczba_drukarek; i++) {
            drukarki.push(i);
        }
    }

    //funkcja do uzycia wolnej drukarki
    int znajdz_drukarke() {
        unique_lock<mutex> lock(mtx_synchro); //dost�p dla jednego w�tku
        cv_synchro.wait(lock, [this] { return !drukarki.empty(); }); //czeka na woln� drukark�
        int drukarka = drukarki.front(); //dostep do pierwszego elementu w drukarkach
        drukarki.pop(); //wzi�cie pierwszej drukarki
        return drukarka;
    }

    //funkcja do druku i zwolnienia drukarki
    void odblokuj_drukarke(int drukarka) {
        unique_lock<mutex> lock(mtx_synchro); //dost�p dla jednego w�tku
        drukarki.push(drukarka); //wrzu� dane na koniec kolejki z drukarkami
        cout << "Drukarka " << drukarka << " jest gotowa do uzycia" << endl;
        cv_synchro.notify_one(); //odblokowuje jeden w�tek
    }

    //dodanie danych do bufora druku
    void dodaj_dane(int drukarka, int dane) {
        unique_lock<mutex> lock(mtx_synchro); //dost�p dla jednego w�tku
        bufor_drukarki[drukarka].push(dane); //wrzu� dane na koniec bufora
    }

    //pobranie danych do druku z bufora
    int pobierz_dane_do_druku(int drukarka) {
        unique_lock<mutex> lock(mtx_synchro); //dost�p dla jednego w�tku

        int dane = 0;

        if (!bufor_drukarki[drukarka].empty()) //sprawdzenie czy co� jest w buforze
        {
            dane = bufor_drukarki[drukarka].front(); //dostep do pierwszego elementu w buforze
            bufor_drukarki[drukarka].pop(); //wzi�cie pierwszego elementu z bufora
        }       
        return dane;
    }

    //symulacja druku danych
    void drukuj(int drukarka, int dane) {
        cout << "Drukarka nr " << drukarka << " drukuje dane nr " << dane << endl;
    }
};

//w�tek konsumenta
void watek_drukarki(Monitor_PK& monitor, int drukarka) {
    while (true) {
        int data = monitor.pobierz_dane_do_druku(drukarka);
        monitor.drukuj(drukarka, data);
        monitor.odblokuj_drukarke(drukarka);
    }
}

//w�tek producent�w
void watek_drukujacy(Monitor_PK& monitor) {
    int dane = 0;
    while (true) {
        int drukarka = monitor.znajdz_drukarke();
        monitor.dodaj_dane(drukarka, dane);
        dane++;
    }
}

int main() {
    Monitor_PK monitor;

    vector<thread> watki_drukarki;
    for (int i = 0; i < liczba_drukarek; i++) {
        watki_drukarki.push_back(thread(watek_drukarki, ref(monitor), i));
    }

    thread watki_drukujace(watek_drukujacy, ref(monitor));

    for (auto& thread : watki_drukarki) {
        thread.join(); //czeka na wykonanie pojedynczego w�tku
    }

    watki_drukujace.join();

    return 0;
}