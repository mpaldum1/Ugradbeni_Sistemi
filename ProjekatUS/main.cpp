#include "mbed.h"
#include "N5110.h"

#define X_MAX 83
#define Y_MAX 47

#define dp23 P0_0

DigitalIn taster(dp1);
bool off = true;

N5110 display(dp4, dp24, dp23, dp25, dp2, dp6, dp18);


class Lopta;

class Reket;

class Ekran;

enum FillType {
    FILL_TRANSPARENT, ///< Transparent with outline
    FILL_BLACK,       ///< Filled black
    FILL_WHITE,       ///< Filled white (no outline)
};


AnalogIn lijeviVRy(dp10);                                                      //joystick
AnalogIn desniVRy(dp11);
DigitalIn lijeviSW(dp9);
DigitalIn desniSW(dp13);

enum Pozicija {
    Gore = 1, Centar = 0, Dolje = -1
} pozicijaLijevi = Centar, pozicijaDesni = Centar; // provejriti je li dekalsrisano.,.........

float lijeviJoy = lijeviVRy; // vrijednost od 0 do 1
float desniJoy = desniVRy;   // vrijednost od 0 do 1

DigitalOut enableLed(dp14);

int tasterPressed(bool &off)
{
    if (off && taster) {
        off = false;
        return 1;
    } else if (!taster)
        off = true;

    return 0;
}


void azurirajPozicijuLijevi()
{
    lijeviJoy = lijeviVRy;
    Pozicija novaPozicija;

    if (lijeviJoy < 1.0 / 3.0 + 0.08) novaPozicija = Dolje;
    else if (lijeviJoy > 2.0 / 3.0 - 0.08) novaPozicija = Gore;
    else
        novaPozicija = Centar;

    if (novaPozicija != pozicijaLijevi)
        pozicijaLijevi = novaPozicija;

}

void azurirajPozicijuDesni()
{
    desniJoy = desniVRy;
    Pozicija novaPozicija;

    if (desniJoy < 1.0 / 3.0 + 0.08) novaPozicija = Dolje;
    else if (desniJoy > 2.0 / 3.0 - 0.08) novaPozicija = Gore;
    else
        novaPozicija = Centar;

    if (novaPozicija != pozicijaDesni)
        pozicijaDesni = novaPozicija;
};


class Lopta
{
    int x, y;                        // koordinate centra
    int radius;                      // poluprecnik lopte
    int delta_x, delta_y;            // pomjeraj - ovisi o modu igre ? // ili wait promijeniti?

public:

    Lopta(int x, int y, int radius, int delta_x, int delta_y) : x(x), y(y), radius(radius), delta_x(delta_x),
        delta_y(delta_y) {}

    int getX() const
    {
        return x;
    }

    void setX(int x)
    {
        Lopta::x = x;
    }

    int getY() const
    {
        return y;
    }

    void setY(int y)
    {
        Lopta::y = y;
    }

    int getRadius() const
    {
        return radius;
    }


    int getDelta_x() const
    {
        return delta_x;
    }

    void setDelta_x(int delta_x)
    {
        Lopta::delta_x = delta_x;
    }

    int getDelta_y() const
    {
        return delta_y;
    }

    void setDelta_y(int delta_y)
    {
        Lopta::delta_y = delta_y;
    }

    void pomjeri(N5110 &display)
    {
        display.drawCircle(x, y, radius, FILL_BLACK);                // brisemo li staru vrijendost
        x += delta_x;                                                // crta staru vrijednost i azurira koordinate
        y += delta_y;
    }

    void nacrtaj(N5110 &display)           // crtamo na zeljenoj koordinati koju navodimu paremtrima
    {
        display.drawCircle(x, y, radius, FILL_BLACK);
    }

    void obrniY()
    {
        delta_y *= -1;
    }


    bool isUdarilaDesniReket(Reket &desniReket);

    bool isUdarilaLijeviReket(Reket &lijeviReket);

    bool udarilaGornjiOkvir()
    {

        return y - radius <= 0;

    }

    bool udarilaDonjiOkvir()
    {

        return y + radius >= Y_MAX;

    }

    // u ovisnosti od ugla odbijanja lopte (ako je udarila reket) ili ako je udarila u okvir, azuriraju se vrijednosti pomaka
    void promijeniKretanje(Reket &lijeviReket, Reket &desniReket);

    // ugao pod kojim se vraca prilikom odbijanja od reket, poziva se samo ako znamo da je lopta udarila u reket
    int dajUgao(int y_R, int h)
    {

        if (y <= y_R - h + 4. / 10 * h) return 60;
        else if (y <= y_R - h + 8. / 10 * h) return 45;
        else if (y <= y_R - h + 12. / 10 * h) return 0;
        else if (y <= y_R - h + 16. / 10 * h) return -45;
        return -60;

    }


    void neIzlaziVanOkvira()
    {

        if (x + radius >= X_MAX)
            x = X_MAX - radius;

        if (x - radius <= 0)
            x = radius;

        if (y + radius >= Y_MAX)
            y = Y_MAX - radius;

        if (y - radius <= 0)
            y = radius;


    }

};


class Reket
{
    int x, y;              // koordinate centra
    int h, d;              // visina reketa od centra-> pozitivna vrijednost, d -> debljina reketa (2 piksela zasad)
    int bodovi;            // ostvareni bodovi igraca

public:

    Reket(int x, int y, int h, int d) : x(x), y(y), h(h), d(d), bodovi(0) {}

    void setX(int x)
    {
        Reket::x = x;
    }

    int getY() const
    {
        return y;
    }

    void setY(int y)
    {
        Reket::y = y;
    }

    int getH() const
    {
        return h;
    }

    int getBodovi() const
    {
        return bodovi;
    }

    int getD() const
    {
        return d;
    }

    void pomjeri(N5110 &display, int delta)
    {
        // delta predstavlja inkrement prilikom razicitih modova
        display.drawRect(x - d / 2, y - h, d, 2 * h, FILL_BLACK);
        y += delta;
    }

    void nacrtaj(N5110 &display)
    {
        display.drawRect(x - d / 2, y - h, d, 2 * h, FILL_BLACK);
    }

    // da li je lijevi reket osvojio poen
    bool osvojioPoenLijeviReket(Lopta &lopta, Reket &desniReket);

    bool osvojioPoenDesniReket(Lopta &lopta, Reket &lijeviReket);

    void dodajPoen()
    {

        ++bodovi;

    }

    void neIzlaziVanOkvira()
    {

        if (y + h >= Y_MAX)
            y = Y_MAX - h;

        if (y - h <= 0)
            y = h;


    }

};


bool Lopta::isUdarilaDesniReket(Reket &desniReket)
{

    return ((x + radius >= X_MAX - desniReket.getD()) && (delta_x > 0) &&
            (y + radius >= desniReket.getY() - desniReket.getH()) &&
            (y - radius <= desniReket.getY() + desniReket.getH()));

}

bool Lopta::isUdarilaLijeviReket(Reket &lijeviReket)
{

    return ((x - radius <= lijeviReket.getD()) && (delta_x < 0) &&
            (y + radius >= lijeviReket.getY() - lijeviReket.getH()) &&
            (y - radius <= lijeviReket.getY() + lijeviReket.getH()));

}


// OVA METODA SE POZIVA ZA KRETANJE LOPTE

void Lopta::promijeniKretanje(Reket &lijeviReket, Reket &desniReket)
{

    int ugao(0);

    if (isUdarilaLijeviReket(lijeviReket)) {

        ugao = dajUgao(lijeviReket.getY(), lijeviReket.getH());

        if (ugao == 0) {

            delta_x = 1;
            delta_y = 0;

        } else if (ugao == 45) {

            delta_x = 1;
            delta_y = -1;

        } else if (ugao == 60) {

            delta_x = 1;
            delta_y = -2;

        } else if (ugao == -45) {

            delta_x = 1;
            delta_y = 1;

        } else if (ugao == -60) {

            delta_x = 1;
            delta_y = 2;

        }

    } else if (isUdarilaDesniReket(desniReket)) {

        ugao = 180 - dajUgao(desniReket.getY(), desniReket.getH());

        if (ugao == 180) {

            delta_x = -1;
            delta_y = 0;

        } else if (ugao == 135) {

            delta_x = -1;
            delta_y = -1;

        } else if (ugao == 120) {

            delta_x = -1;
            delta_y = -2;

        } else if (ugao == 225) {

            delta_x = -1;
            delta_y = 1;

        } else if (ugao == 240) {

            delta_x = -1;
            delta_y = 2;

        }

    } else if (udarilaGornjiOkvir() || udarilaDonjiOkvir())
        obrniY();


}


bool Reket::osvojioPoenLijeviReket(Lopta &lopta, Reket &desniReket)
{

    return (lopta.getX() + lopta.getRadius() >= X_MAX && lopta.getY() - lopta.getRadius() >= 0 &&
            lopta.getY() + lopta.getRadius() <= Y_MAX) && !lopta.isUdarilaDesniReket(desniReket);

}

bool Reket::osvojioPoenDesniReket(Lopta &lopta, Reket &lijeviReket)
{

    return (lopta.getX() - lopta.getRadius() <= 0 && lopta.getY() - lopta.getRadius() >= 0 &&
            lopta.getY() + lopta.getRadius() <= Y_MAX) && !lopta.isUdarilaLijeviReket(lijeviReket);

}


class Ekran
{
    N5110 display;

public:


    Ekran(PinName vcc, PinName sce, PinName rst, PinName dc, PinName mosi, PinName sclk, PinName led) :
        display(vcc,
                sce,
                rst,
                dc,
                mosi,
                sclk,
                led) {}

    void init()
    {

        display.init();

    }

    void refresh()
    {


        display.refresh();

    }

    void clear()
    {

        display.clear();

    }

    void turnOff()
    {
        display.turnOff();
    }

    N5110 &getDisplay()
    {

        return this->display;

    }

    void nacrtajMrezu()
    {
        display.drawLine(X_MAX / 2, 0, X_MAX / 2, Y_MAX, 2);
    }


    void InitialScreen()                                                          // ispisuje imena
    {

        clear();
        char ime1[] = "Jusuf Delalic";
        char ime2[] = "Medin Paldum";

        display.printString(ime1, 3, 2);
        display.printString(ime2, 6, 3);
        refresh();
    }


    // opcija ovisi od joysitcka
    void StartMenu(int opcija)                                                  // 1 = start, 0 = exit
    {
        display.clear();
        // opcija je broj koji govori na kojem nivou je joystick

        char startMenu[] = "Start menu";
        char exitOption[] = "Exit";
        char start[] = "Start";

        display.printString(startMenu, 12, 0);

        display.printString(start, 42, 2);

        display.printString(exitOption, 42, 3);


        if (opcija == 1) {
            display.drawLine(27, 16, 39, 16, 2);                           //linija na start
        } else {
            display.drawLine(27, 28, 39, 28, 2);                           // linija na exit
        }

        display.refresh();
    }


    void SelectMode(int opcija)                                              // 1 - single, 2 - multi
    {

        display.clear();


        display.printString("Select mode", 12, 0);
        display.printString("Single", 42, 2);
        display.printString("Multi", 42, 3);

        if (opcija == 1) {
            display.drawLine(27, 16, 39, 16, 2);        // linija na single

        } else { // izmjena
            display.drawLine(27, 28, 39, 28, 2);        // linija na multi


        }

        display.refresh();

    };


    void SelectDifficulty(int opcija)                                          // 1 - east, 2 - medium, 3 - hard
    {

        display.clear();
        char select[] = "Select mode";
        char easy[] = "Easy";
        char medium[] = "Medium";
        char hard[] = "Hard";

        display.printString(select, 12, 0);

        display.printString(easy, 36, 2);

        display.printString(medium, 36,3);

        display.printString(hard, 36, 4);

        switch (opcija) {
            case 1:
                display.drawLine(21, 21, 33, 21, 2);                        //easy
                break;

            case 2:
                display.drawLine(21, 27, 33, 27, 2);                        //medium
                break;

            default:
                display.drawLine(21, 33, 33, 33, 2);                        //hard
                break;
        }

        display.refresh();

    }


    void postaviCentrirano(Reket &lijeviReket, Reket &desniReket, Lopta &lopta)
    {

        display.clear();

        lijeviReket.setX(lijeviReket.getD() / 2);
        lijeviReket.setY(Y_MAX / 2);

        desniReket.setX(X_MAX - desniReket.getD() / 2);
        desniReket.setY(Y_MAX / 2);

        lopta.setX(X_MAX / 2);
        lopta.setY(Y_MAX / 2);

        lijeviReket.nacrtaj(display);

        desniReket.nacrtaj(display);
        lopta.nacrtaj(display);

        display.refresh();


    }

    void ispisiBodove(Reket &lijeviReket, Reket &desniReket)
    {

        int xLijevo = 3 / 8 * X_MAX - 6;
        int xDesno = 5 / 8 * X_MAX;


        // ne clear jer imamo rekete i loptu

        if (lijeviReket.getBodovi() >=
                10) {                                                    // ispisivanje rezultata
            // (pazimo kada je u pitanju dvocifren broj)

            display.printChar(xLijevo + 6, 0, '1');
            display.printChar(xLijevo + 6, 0, '0' + lijeviReket.getBodovi() % 10);

        } else {
            display.printChar(xLijevo + 6, 0, '0' + lijeviReket.getBodovi());
            display.refresh();
        }


        if (desniReket.getBodovi() >= 10) {
            display.printChar(xDesno, 0, '1' + lijeviReket.getBodovi() % 10);
            display.printChar(xDesno + 6, 0, '0' + lijeviReket.getBodovi() % 10);
        } else {
            display.printChar(xDesno, 0, '0' + desniReket.getBodovi());
        }

        display.refresh();
    }


    void GameOver(int mode, int winner)                                      // mode 1 -> igrac; mode 2 -> computer
    {
        display.clear();
        // winner = {1,2}
        display.printString("Game over", 15, 0);


        if (mode == 1) {
            if (winner == 1) {
                display.printString("Player won", 12, 2);

            } else {
                display.printString("Computer won", 6, 2);

            }

        } else {
            if (winner == 1) {
                display.printString("Player 1 won", 6, 2);

            } else {
                display.printString("Player 2 won", 6, 2);

            }

        }

        display.refresh();

    }

    /* Prilikom ispisivanja teksta na ekran pretpostavili smo da kao koordinate pocetka saljemo donji lijevi pixel */
};
















class Pong
{

    int tokIgre;                         // pocetak igre
    int mode;                            // signle player
    int tezina;                          // easy / medium / hard
    int zapocetiIgru;                    // start/exit
    int pobjednik;

    Lopta lopta;
    Reket lijeviReket, desniReket;          // lijevi - Pl1 / desni - Pl2

    Ekran display;

public:

    Pong(int &tokIgre, Lopta &lopta, Reket &lijeviReket, Reket &desniReket, Ekran &display) :
        tokIgre(tokIgre), lopta(lopta), lijeviReket(lijeviReket), desniReket(desniReket), display(display)
    {
        tokIgre = 0;
        mode = 1;
        tezina = 1;
        zapocetiIgru = 1;
        pobjednik = 0;
    }

    void setMode(int mode)
    {
        Pong::mode = mode;
    }

    void setTezina(int tezina)
    {
        Pong::tezina = tezina;
    }

    void setPobjednik(int pobjednik)
    {
        Pong::pobjednik = pobjednik;
    }


    void setZapocetiIgru(int zapocetiIgru)
    {
        Pong::zapocetiIgru = zapocetiIgru;
    }

    void inicijalnoStanje()
    {
        // display.InitialScreen(display);
        wait(3);
        ++tokIgre;
    }

    void startGameStanje(int odabir)
    {
        //  display.StartMenu(odabir); // izmjena - true
    }

    void modeStanje(int mode)
    {
        //  display.SelectMode(mode);
        // pritisnut taster
        this->mode = mode;
    }

    void tezinaStanje(int tezina)
    {
        //  display.SelectDifficulty(tezina);
        // pritisnut taster
        this->tezina = tezina;
    }

    void igraStanje()
    {
        if (lijeviReket.getBodovi() >= 11 || desniReket.getBodovi() >= 11) {
            tokIgre++;
        } else {

            int osvojioBod = 1;                         // centrirati loptu i rekete
            bool centrirano = true;
            if (lijeviReket.osvojioPoenLijeviReket(lopta, desniReket)) {
                lijeviReket.dodajPoen();
                osvojioBod = 1;
                centrirano = true;

            } else if (desniReket.osvojioPoenDesniReket(lopta, lijeviReket)) {
                desniReket.dodajPoen();
                osvojioBod = 1;
                centrirano = true;
            }

            if (centrirano) {
                if (tasterPressed(off)) {
                    centrirano = false;
                    if (osvojioBod == 1) {
                        lopta.setDelta_x(-1);
                        lopta.setDelta_y(-1);
                    } else if (osvojioBod == 2) {
                        lopta.setDelta_x(1);
                        lopta.setDelta_y(1);
                    }
                }
            }
            lopta.promijeniKretanje(lijeviReket, desniReket);
            //     lopta.pomjeri(display.getDisplay());

            lijeviReket.setY(lijeviReket.getY() + pozicijaLijevi);
            desniReket.setY(desniReket.getY() + pozicijaDesni);

            //   lijeviReket.pomjeri(display.getDisplay(), mode);
            //  desniReket.pomjeri(display.getDisplay(), mode);
        }


    }

    void gameOverStanje()
    {
        //   display.GameOver(mode, pobjednik);
        wait(3);

        tokIgre = 1;                         // vrati na odabir
        mode = 1;                            // signle player
        tezina = 1;                          // easy / medium / hard
        zapocetiIgru = 1;                    // start/exit
        pobjednik = 0;
    }

    void izadjiStanje()
    {
//       display.getDisplay().clear();
    }


};


Ekran ekran(dp4, dp24, dp23, dp25, dp2, dp6, dp18);
//Pong igra(tokIgre, lopta, lijevi, desni, ekran);

























int main()
{

    enableLed = 1;                                                                  // gasimo ledice
    lijeviSW.mode(PullUp);
    desniSW.mode(PullUp);

    ekran.init();
    ekran.clear();

    int tokIgre = 0;
    Reket lijevi(1, Y_MAX / 2, 6, 2);                              // kreiramo desni, lijevi reket i loptu
    Reket desni(82, Y_MAX / 2, 6, 2);
    Lopta lopta(X_MAX / 2, Y_MAX / 2, 1, 0, 0);


    int biranjeOpcije = 1;

    ekran.InitialScreen(); // ----------------------------------------------------------------------------------------------------------------------------
    wait(3);


    pozicijaLijevi = Centar;
    pozicijaDesni = Centar;

    tokIgre++;

    bool pritisnutTaster = false;

    int mode = 1; // single player
    int winner = 1; // player1
    int difficulty = 1; // easy

    //bool poIelaIgra = false;

    while (1) {

        biranjeOpcije = 1; // start
        // Start/Exit
        while (tokIgre == 1) {

            pritisnutTaster = tasterPressed(off);
            pozicijaLijevi = Centar;
            pozicijaDesni = Centar;


            ekran.clear();
            ekran.StartMenu(biranjeOpcije);
            wait(0.5);

            ekran.refresh();


            azurirajPozicijuLijevi();

            if (pozicijaLijevi % 2 != 0) {

                if (biranjeOpcije == 1)
                    biranjeOpcije = 0;

                else biranjeOpcije = 1;

            }

            if (pritisnutTaster) {
                if (biranjeOpcije == 0) {
                    ekran.clear();
                    ekran.turnOff();
                    return 0;

                } else {

                    ++tokIgre;
                }

            }

        }


        biranjeOpcije = 1;
        // Singleplayer, Multiplayer
        while (tokIgre == 2) {

            ekran.clear();

            pritisnutTaster = tasterPressed(off);
            // single player
            pozicijaLijevi = Centar;
            pozicijaDesni = Centar;


            ekran.SelectMode(biranjeOpcije);
            ekran.refresh();
            wait(0.5);


            azurirajPozicijuLijevi();

            if (pozicijaLijevi % 2 != 0) {

                if (biranjeOpcije == 1)
                    biranjeOpcije = 0;
                else biranjeOpcije = 1;

            }


            if (pritisnutTaster) {
                if (biranjeOpcije == 0) mode = 2; // multiplayer
                else if (biranjeOpcije == 1) mode = 1; // single player


                ++tokIgre;
            }


        }


        biranjeOpcije = 0;
        // Easy - 1, Medium - 2, Hard - 3
        while (tokIgre == 3) {

            ekran.clear();
            pritisnutTaster = tasterPressed(off);

            pozicijaLijevi = Centar;
            pozicijaDesni = Centar;


            if (biranjeOpcije == 0) {
                ekran.SelectDifficulty(1);

            } else if (biranjeOpcije == 1) {
                ekran.SelectDifficulty(3);

            } else if (biranjeOpcije == 2) {
                ekran.SelectDifficulty(biranjeOpcije);

            }

            azurirajPozicijuLijevi(); // mozda na ticker...

            if (pozicijaLijevi == Dolje)
                biranjeOpcije++;

            else if (pozicijaLijevi == Gore)
                biranjeOpcije--;

            while (biranjeOpcije < 0)
                biranjeOpcije += 3;

            biranjeOpcije %= 3;


            if (pritisnutTaster) {
                //igra.setTezina(biranjeOpcije + 1);
                if (biranjeOpcije == 0) difficulty = 1;
                else if (biranjeOpcije == 1) difficulty = 3;
                else difficulty = 2;

                ekran.clear();
                ekran.refresh();
                ++tokIgre;
            }

            ekran.refresh();
            wait(0.5);


        }

        // pocetno stanje igre...
        if (tokIgre == 4) {

            ekran.clear();
            ekran.postaviCentrirano(lijevi, desni, lopta);
            ekran.ispisiBodove(lijevi, desni);
            ekran.refresh();

            wait(1.5);

            lopta.setDelta_x(2);
            lopta.setDelta_y(2);
        }


        while (tokIgre == 4) {
            

            if (mode == 1) {

                // Single player
                ekran.clear();
                if (lijevi.osvojioPoenLijeviReket(lopta, desni) || desni.osvojioPoenDesniReket(lopta, lijevi)) {

                    ekran.clear();
                    ekran.postaviCentrirano(lijevi, desni, lopta);
                    lopta.setDelta_x(2);
                    lopta.setDelta_y(2);
                

                    if (lijevi.osvojioPoenLijeviReket(lopta, desni))
                        lijevi.dodajPoen();

                    else desni.dodajPoen();

                    ekran.ispisiBodove(lijevi, desni);
                    ekran.refresh();

                    wait(1.5); // prikazujemo centrirano 1.5 sekundi nakon cega lopta pocinje kretanje prema nekom od igraca

                }


                if (lijevi.getBodovi() >= 11 || desni.getBodovi() >= 11)
                    ++tokIgre;

                if (tokIgre != 4)
                    break;

                azurirajPozicijuLijevi();

                int delta_lijevi = pozicijaLijevi * 2;
                int delta_desni;

                if (lopta.getY() > desni.getY())
                    delta_desni = 2; // idi dolje

                else if (lopta.getY() > desni.getY())
                    delta_desni = -2; // idi gore

                else delta_desni = 0;

                lopta.promijeniKretanje(lijevi, desni);


                lopta.neIzlaziVanOkvira();
                lijevi.neIzlaziVanOkvira();
                desni.neIzlaziVanOkvira();

                ekran.clear();

                lijevi.pomjeri(ekran.getDisplay(), delta_lijevi);
                desni.pomjeri(ekran.getDisplay(), delta_desni);
                lopta.pomjeri(ekran.getDisplay());
                ekran.ispisiBodove(lijevi, desni);

                if (difficulty == 2) {

                    lopta.setDelta_y(lopta.getDelta_y() * 1.5);
                    lopta.setDelta_x(lopta.getDelta_x() * 1.5);

                    delta_desni *= 1.5;
                } else if (difficulty == 3) {

                    lopta.setDelta_y(lopta.getDelta_y() * 2.5);
                    lopta.setDelta_x(lopta.getDelta_x() * 2.5);

                    delta_desni *= 2.5;

                }

                ekran.refresh();
                wait(0.2);

            } else {

                ekran.clear();


                if (lijevi.osvojioPoenLijeviReket(lopta, desni) || desni.osvojioPoenDesniReket(lopta, lijevi)) {

                    ekran.clear();
                    ekran.postaviCentrirano(lijevi, desni, lopta);
                    lopta.setDelta_x(2);
                    lopta.setDelta_y(2);

                    if (lijevi.osvojioPoenLijeviReket(lopta, desni))
                        lijevi.dodajPoen();

                    else desni.dodajPoen();

                    ekran.ispisiBodove(lijevi, desni);
                    ekran.refresh();

                    wait(1.5); // prikazujemo centrirano 1.5 sekundi nakon cega lopta pocinje kretanje prema nekom od igraca

                }


                if (lijevi.getBodovi() >= 11 || desni.getBodovi() >= 11)
                    ++tokIgre;

                if (tokIgre != 4)
                    break;


                azurirajPozicijuDesni();
                azurirajPozicijuLijevi();

                int delta_lijevi = pozicijaLijevi * 2;// neka funkcija od lijevog joysticka koja se poziva tickerom
                int delta_desni = pozicijaDesni * 2;// neka funkcija od desnog joysticka koja se poziva tickerom
                lopta.promijeniKretanje(lijevi, desni);

                lopta.neIzlaziVanOkvira();
                lijevi.neIzlaziVanOkvira();
                desni.neIzlaziVanOkvira();


                ekran.clear();

                lijevi.pomjeri(ekran.getDisplay(), delta_lijevi);
                desni.pomjeri(ekran.getDisplay(), delta_desni);
                lopta.pomjeri(ekran.getDisplay());
                ekran.ispisiBodove(lijevi, desni);

                ekran.refresh();

                if (difficulty == 2) {

                    lopta.setDelta_y(lopta.getDelta_y() * 1.5);
                    lopta.setDelta_x(lopta.getDelta_x() * 1.5);

                    delta_desni *= 1.5;
                    delta_lijevi *= 1.5;


                } else if (difficulty == 3) {

                    lopta.setDelta_y(lopta.getDelta_y() * 2.5);
                    lopta.setDelta_x(lopta.getDelta_x() * 2.5);

                    delta_desni *= 2.5;
                    delta_lijevi *= 2.5;


                }

                wait(0.3);

            }


        }
        biranjeOpcije = 1;
        // Game Over
        if (tokIgre == 5) {

            ekran.clear();
            if (lijevi.getBodovi() == 11)
                winner = 1;
            else winner = 2;

            ekran.GameOver(mode, winner);
            ekran.refresh();
            wait(3);
            tokIgre = 1;
        }


    } // kraj beskonacne while petlje

} // kraj main -a
