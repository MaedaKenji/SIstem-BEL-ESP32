#include <Arduino.h>
#include <unity.h>

extern int nilai_peserta[3];
extern int tambahNilai(int pesertaAktif);
extern int kurangNilai(int pesertaAktif);

void test_tambah_nilai() {
    int pesertaAktif = 0;
    nilai_peserta[pesertaAktif] = 5;
    TEST_ASSERT_EQUAL(6, tambahNilai(pesertaAktif)); // nilai harus menjadi 6
}

void test_kurang_nilai() {
    int pesertaAktif = 1;
    nilai_peserta[pesertaAktif] = 5;
    TEST_ASSERT_EQUAL(4, kurangNilai(pesertaAktif)); // nilai harus menjadi 4
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_tambah_nilai);
    RUN_TEST(test_kurang_nilai);
    UNITY_END();
}

void loop() {
    // loop kosong, karena semua tes dilakukan di setup
}
