#include <Arduino.h>

// Definisikan pin GPIO untuk a-g
const int pins[] = {33, 32, 27, 15, 2, 25, 26}; // a, b, c, d, e, f, g
const int tombol[] = {4,16,17};           // 3 tombol untuk 3 peserta
// Definisikan pin untuk common
const int common_pins[] = {13, 12, 14}; // common
const int juri[] = {23,22,1};
int nilai_peserta[3] = {8, 1, 2};
int pesertaAktif = -1;
int jumlahPesertaAktif = 3;
int kondisi = 0;                // 
bool tombolSudahDitekan = false; // Variabel untuk memeriksa apakah sudah ada tombol yang ditekan
int timer = 0;
bool prevJuriState[3] = {};
bool currentDecreaseState = HIGH;
bool prevIncreaseState = HIGH;
bool semua = true;
bool standby = false;
const int MAX_PESERTA = 3;
unsigned long startTime = millis(); // Ambil waktu mulai
unsigned long limaBelasS = 15000;   // 15 detik dalam milidetik
const int angka_7seg[10][7] = {
    {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW},  // 0
    {LOW, HIGH, HIGH, LOW, LOW, LOW, LOW},      // 1
    {HIGH, HIGH, LOW, HIGH, HIGH, LOW, HIGH},   // 2
    {HIGH, HIGH, HIGH, HIGH, LOW, LOW, HIGH},   // 3
    {LOW, HIGH, HIGH, LOW, LOW, HIGH, HIGH},    // 4
    {HIGH, LOW, HIGH, HIGH, LOW, HIGH, HIGH},   // 5
    {HIGH, LOW, HIGH, HIGH, HIGH, HIGH, HIGH},  // 6
    {HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW},     // 7
    {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}, // 8
    {HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH}   // 9
};
const int huruf_7seg[3][7] = {
    {HIGH, HIGH, HIGH, HIGH, HIGH, LOW, HIGH}, // a
    {LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH},  // b
    {HIGH, LOW, LOW, HIGH, HIGH, HIGH, LOW}    // c
};
struct PesertaStates
{
    bool states[MAX_PESERTA];

    PesertaStates()
    {
        for (int i = 0; i < MAX_PESERTA; i++)
        {
            states[i] = true; // Inisialisasi semua peserta sebagai aktif
        }
    }

    void setActive(int index)
    {
        if (index >= 0 && index < MAX_PESERTA)
        {
            states[index] = true;
        }
    }

    void setInactive(int index)
    {
        if (index >= 0 && index < MAX_PESERTA)
        {
            states[index] = false;
        }
    }

    bool isActive(int index) const
    {
        return (index >= 0 && index < MAX_PESERTA) ? states[index] : false;
    }

    int getActiveCount() const
    {
        int count = 0;
        for (int i = 0; i < MAX_PESERTA; i++)
        {
            if (states[i])
                count++;
        }
        return count;
    }
};

PesertaStates pesertaStates;

void tampilkanAngka(int angka, int displayIndex)
{
    // Nonaktifkan semua common pin
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(common_pins[i], HIGH);
    }

    // Set nilai pins sesuai dengan angka yang akan ditampilkan
    for (int i = 0; i < 7; i++)
    {
        digitalWrite(pins[i], angka_7seg[angka][i]);
    }

    // Aktifkan common pin yang sesuai
    digitalWrite(common_pins[displayIndex], LOW);
}

void tampilkanSemuaPeserta()
{
    for (int i = 0; i < 3; i++)
    {
        tampilkanAngka(nilai_peserta[i], i);
        delay(5); // Delay kecil untuk multiplexing
    }
}

int resetNilaiPeserta()
{
    for (int i = 0; i < 3; i++)
    {
        nilai_peserta[i] = 0;
    }
    // tombolDitekan = false;
    pesertaAktif = 0;
    return 0;
}

int tambahNilai(int pesertaAktif)
{
    if (nilai_peserta[pesertaAktif] < 9)
    {
        nilai_peserta[pesertaAktif]++;
    }
    return nilai_peserta[pesertaAktif];
}

int kurangNilai()
{
    if (nilai_peserta[pesertaAktif] > 0)
    {
        nilai_peserta[pesertaAktif]--;
    }
    return nilai_peserta[pesertaAktif];
}

int switchPeserta()
{
    pesertaAktif = (pesertaAktif + 1) % 3;
    return pesertaAktif;
}

int tampilkanHuruf(int huruf, int displayIndex)
{
    if (huruf < 0 || huruf > 2)
    {
        Serial.println("Huruf tidak valid! Masukkan 0 untuk 'a', 1 untuk 'b', atau 2 untuk 'c'.");
        return -1;
    }

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(common_pins[i], HIGH);
    }
    // Loop untuk mengatur segmen
    for (int i = 0; i < 7; i++)
    {
        digitalWrite(pins[i], huruf_7seg[huruf][i]);
    }
    // Aktifkan common pin yang sesuai
    digitalWrite(common_pins[displayIndex], LOW);
    return 0;
}

void setDisplay(int kondisi)
{
    if (kondisi == 0) {
        for (int i = 0; i < 3; i++)
        {
            tampilkanAngka(nilai_peserta[i], i);
        }
    }
    else if (kondisi == 1) {
        for (int i = 0; i < 3; i++)
        {
            tampilkanHuruf(i, i);
        }
    }
    else if (kondisi == 2) {
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(common_pins[i], HIGH);
            if (i == pesertaAktif) {
                tampilkanHuruf(i, i);
            }
        }
    }

    else if (kondisi == 3) {
        for (int i = 0; i < 3; i++)
        {
            if (i == pesertaAktif) {
                tampilkanAngka(nilai_peserta[i], i);
            }
            else {
                tampilkanHuruf(i, i);
            }
        }
    }

}

void setup()
{
    Serial.begin(115200);

    // Inisialisasi semua pin a-g sebagai OUTPUT
    for (int i = 0; i < 7; i++)
    {
        pinMode(pins[i], OUTPUT);
    }

    // Inisialisasi semua pin common sebagai OUTPUT
    for (int i = 0; i < 3; i++)
    {
        pinMode(common_pins[i], OUTPUT);
    }

    // Inisialisasi semua tombol sebagai INPUT_PULLUP
    for (int i = 0; i < 3; i++)
    {
        pinMode(tombol[i], INPUT_PULLUP);
        pesertaStates.setActive(i);
    }

    // Inisialisasi tombol juri sebagai INPUT_PULLUP
    for (int i = 0; i < 3; i++)
    {
        pinMode(juri[i], INPUT_PULLUP);
    }

    // Mengatur status pin common
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(common_pins[i], LOW);
        // Serial.println("Common pin " + String(common_pins[i]) + " =" + String(digitalRead(common_pins[i])));
        // delay(2000);
    }


    Serial.println("Program Started");
    // int pinStatus = digitalRead(14);

    // Menampilkan status pin 14 melalui Serial Monitor
    // if (pinStatus == HIGH) {
    //     Serial.println("Pin 14 is HIGH");
    // } else {
    //     Serial.println("Pin 14 is LOW");
    // }
    // delay(5000);
}

void loop()
{
    setDisplay(kondisi);
    // int pinStatus = digitalRead(14);

    // Menampilkan status pin 14 melalui Serial Monitor
    // if (pinStatus == HIGH) {
    //     Serial.println("Pin 14 is HIGH");
    // } else {
    //     Serial.println("Pin 14 is LOW");
    // }

    Serial.println("kondisi: " + String(kondisi));

    

    // Membaca user input
    for (int i = 0; i < 3; i++)
    {
        if (digitalRead(tombol[i]) == LOW && pesertaStates.isActive(i) && (kondisi == 1 || kondisi == 3))
        {
            tombolSudahDitekan = true;
            pesertaAktif = i;
            kondisi = 2;
            Serial.println("Tombol " + String(i) + " ditekan");
        }
    }

    if (digitalRead(juri[0])==LOW && prevJuriState[0]==HIGH && tombolSudahDitekan==false && pesertaStates.getActiveCount() == 3){
        kondisi = 1;
    }

    if (digitalRead(juri[1]) == LOW && prevJuriState[1] == HIGH && tombolSudahDitekan == true){
        tambahNilai(pesertaAktif);
        kondisi = 0;
        for (int i = 0; i < 3; i++){
            pesertaStates.setActive(i);
        }
        tombolSudahDitekan = false;
    }

    if (digitalRead(juri[2]) == LOW && prevJuriState[2] == HIGH && tombolSudahDitekan == true){
        kurangNilai();
        kondisi = 3;
        pesertaStates.setInactive(pesertaAktif);
        tombolSudahDitekan = false;
    }

    for (int i = 0; i < 3; i++){
        prevJuriState[i] = digitalRead(juri[i]);
    }


}