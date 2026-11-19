// Teste de Corners3d::getEnvelopeVolume (eq. 21 do paper)
//
// Compilar:
//   g++ -std=c++17 test_volume.cpp Corners2d.cpp Corners3d.cpp -o test_volume
//
// Pré-requisitos (ajuste se necessário):
//   - Corners2d::getEnvelopeArea static, retornando long long e tratando vetor vazio (W*H)
//   - Corners3d::getEnvelopeVolume(const vector<Package>&, const vector<Package>&, ll W, ll H, ll D)
//   - Package com campos: position (Coordinates3d), l (x), a (y), c (z)
//     Se o seu Package tiver construtor próprio, altere apenas makePackage().

#include "Corners3d.h"
#include <iostream>
#include <vector>
#include "../src/Types.h"

using namespace std;

static ll failures = 0;
static ll total = 0;

#define CHECK_EQ(actual, expected, name)                                          \
    do {                                                                          \
        total++;                                                                  \
        long long a_ = (actual), e_ = (expected);                                 \
        if (a_ == e_) {                                                           \
            cout << "[ OK ] " << name << " (" << a_ << ")" << endl;               \
        } else {                                                                  \
            failures++;                                                           \
            cout << "[FAIL] " << name << ": esperado " << e_                      \
                 << ", obtido " << a_ << endl;                                    \
        }                                                                         \
    } while (0)

// Cria um pacote com canto (x,y,z) e dimensões l (x), a (y), c (z)
static Package makePackage(ll x, ll y, ll z, ll l, ll a, ll c)
{
    Package p;
    p.position = Coordinates3d(x, y, z);
    p.l = l;
    p.a = a;
    p.c = c;
    return p;
}

// Caixa 1x1x1 usada como "caixa restante" que sempre cabe
static vector<Package> tinyRemaining()
{
    return vector<Package>(1, makePackage(0, 0, 0, 1, 1, 1));
}

// 1. Nenhuma caixa colocada: envelope vazio -> volume 0
static void testNoInserted()
{
    vector<Package> inserted;
    long long v = Corners3d::getEnvelopeVolume(inserted, tinyRemaining(), 10, 10, 10);
    CHECK_EQ(v, 0, "sem caixas colocadas");
}

// 2. Nenhuma caixa restante: tudo é envelope -> volume W*H*D
static void testNoRemaining()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 5, 6));
    vector<Package> remaining;
    long long v = Corners3d::getEnvelopeVolume(inserted, remaining, 10, 10, 10);
    CHECK_EQ(v, 1000, "sem caixas restantes");
}

// 3. Uma caixa na origem: envelope = volume da caixa (4*5*6 = 120)
static void testSingleBox()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 5, 6));
    long long v = Corners3d::getEnvelopeVolume(inserted, tinyRemaining(), 10, 10, 10);
    CHECK_EQ(v, 120, "uma caixa na origem");
}

// 4. Duas caixas empilhadas em z com a mesma base: 4*5*3 + 4*5*3 = 120
static void testStackedInZ()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 5, 3));
    inserted.push_back(makePackage(0, 0, 3, 4, 5, 3));
    long long v = Corners3d::getEnvelopeVolume(inserted, tinyRemaining(), 10, 10, 10);
    CHECK_EQ(v, 120, "duas caixas empilhadas em z");
}

// 5. Escada 2D: A (4x6) e B (3x3) lado a lado, profundidade 5
//    área do envelope = 4*6 + 3*3 = 33  ->  volume = 33*5 = 165
static void testStaircase()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 6, 5));
    inserted.push_back(makePackage(4, 0, 0, 3, 3, 5));
    long long v = Corners3d::getEnvelopeVolume(inserted, tinyRemaining(), 10, 10, 10);
    CHECK_EQ(v, 165, "escada 2D (duas caixas lado a lado)");
}

// 6. Break por profundidade: a menor restante tem c=6 e z'=6 já não comporta.
//    O último termo do paper vira (D - 0) * A = 10 * 20 = 200 (estimativa conservadora)
static void testDepthBreak()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 5, 6));
    vector<Package> remaining(1, makePackage(0, 0, 0, 1, 1, 6));
    long long v = Corners3d::getEnvelopeVolume(inserted, remaining, 10, 10, 10);
    CHECK_EQ(v, 200, "break por profundidade (termo final do paper)");
}

// 7. Remoção de corner polls: restante com l=7 não cabe ao lado da caixa
//    (4+7 > 10), só sobra o corner (0,5) -> área do envelope = 10*5 = 50
//    volume = 6 * 50 = 300
static void testCornerRemoval()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 5, 6));
    vector<Package> remaining(1, makePackage(0, 0, 0, 7, 1, 1));
    long long v = Corners3d::getEnvelopeVolume(inserted, remaining, 10, 10, 10);
    CHECK_EQ(v, 300, "remoção de corner polls inviáveis");
}

// 8. Independência da ordem de entrada das caixas colocadas
static void testOrderIndependence()
{
    vector<Package> a;
    a.push_back(makePackage(0, 0, 0, 4, 6, 5));
    a.push_back(makePackage(4, 0, 0, 3, 3, 5));

    vector<Package> b;
    b.push_back(makePackage(4, 0, 0, 3, 3, 5));
    b.push_back(makePackage(0, 0, 0, 4, 6, 5));

    long long va = Corners3d::getEnvelopeVolume(a, tinyRemaining(), 10, 10, 10);
    long long vb = Corners3d::getEnvelopeVolume(b, tinyRemaining(), 10, 10, 10);
    CHECK_EQ(vb, va, "ordem de entrada não altera o volume");
}

// 9. Sanidade: o envelope nunca é menor que o volume das caixas nem maior que B
static void testBounds()
{
    vector<Package> inserted;
    inserted.push_back(makePackage(0, 0, 0, 4, 6, 5));
    inserted.push_back(makePackage(4, 0, 0, 3, 3, 5));
    inserted.push_back(makePackage(0, 0, 5, 5, 5, 2));

    long long sumV = 0;
    for (const Package& p : inserted)
        sumV += (long long)p.l * p.a * p.c;

    long long v = Corners3d::getEnvelopeVolume(inserted, tinyRemaining(), 10, 10, 10);

    total++;
    if (v >= sumV && v <= 1000) {
        cout << "[ OK ] limites: " << sumV << " <= " << v << " <= 1000" << endl;
    } else {
        failures++;
        cout << "[FAIL] limites: volume " << v << " fora de [" << sumV << ", 1000]" << endl;
    }
}

int main()
{
    testNoInserted();
    testNoRemaining();
    testSingleBox();
    testStackedInZ();
    testStaircase();
    testDepthBreak();
    testCornerRemoval();
    testOrderIndependence();
    testBounds();

    cout << endl << (total - failures) << "/" << total << " testes passaram" << endl;
    return failures == 0 ? 0 : 1;
}
