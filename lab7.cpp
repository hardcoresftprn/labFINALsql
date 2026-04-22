#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <windows.h>
#include <conio.h>
#include <mysql.h>

using namespace std;

const char* DB_HOST = "localhost";
const char* DB_USER = "root";
const char* DB_PASS = "i41Sa52MM.T9";
const char* DB_NAME = "Restaurant";
const int DB_PORT = 3306;

void clearScreen() {
    system("cls");
}

void pauseScreen() {
    system("pause");
}

string escapeString(MYSQL* conn, const string& input) {
    if (input.empty()) return "";
    vector<char> buffer(input.length() * 2 + 1);
    mysql_real_escape_string(conn, buffer.data(), input.c_str(), input.length());
    return string(buffer.data());
}

string citesteParola() {
    string parola;
    char ch;
    cout << "Parola: ";
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!parola.empty()) {
                parola.pop_back();
                cout << "\b \b";
            }
        }
        else {
            parola.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return parola;
}

MYSQL* connectDB() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        cout << "Eroare la initializarea MySQL!" << endl;
        return nullptr;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, nullptr, 0)) {
        cout << "Eroare la conectare: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return nullptr;
    }

    mysql_set_character_set(conn, "utf8mb4");
    cout << "Conectat la baza de date cu succes!" << endl;
    return conn;
}

void afiseazaRezultate(MYSQL* conn, const string& query) {
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Eroare SQL: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        if (mysql_field_count(conn) == 0) {
            cout << "Query executat. Randuri afectate: " << mysql_affected_rows(conn) << endl;
        }
        else {
            cout << "Eroare la obtinerea rezultatelor: " << mysql_error(conn) << endl;
        }
        return;
    }

    int numFields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    vector<int> widths(numFields, 12);
    for (int i = 0; i < numFields; i++) {
        widths[i] = max(widths[i], (int)strlen(fields[i].name) + 2);
    }

    cout << "\n";
    for (int i = 0; i < numFields; i++) {
        cout << left << setw(widths[i]) << fields[i].name << " | ";
    }
    cout << "\n" << string(accumulate(widths.begin(), widths.end(), 0) + numFields * 3, '-') << "\n";

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < numFields; i++) {
            cout << left << setw(widths[i]) << (row[i] ? row[i] : "NULL") << " | ";
        }
        cout << "\n";
    }
    cout << endl;

    mysql_free_result(res);
}

string autentificare(MYSQL* conn, string& username) {
    clearScreen();
    cout << "========== AUTENTIFICARE ==========\n";
    cout << "Username: ";
    cin >> username;
    string parola = citesteParola();

    string userEsc = escapeString(conn, username);
    string passEsc = escapeString(conn, parola);

    string query = "SELECT rol FROM Utilizatori WHERE username='" + userEsc + "' AND parola='" + passEsc + "'";
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Eroare la autentificare: " << mysql_error(conn) << endl;
        return "";
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return "";

    string rol = "";
    if (mysql_num_rows(res) > 0) {
        MYSQL_ROW row = mysql_fetch_row(res);
        rol = row[0];
    }

    mysql_free_result(res);
    return rol;
}

void menuVizualizare(MYSQL* conn) {
    int opt;
    do {
        clearScreen();
        cout << "========== VIZUALIZARE DATE ==========\n";
        cout << "1. Clienti\n";
        cout << "2. Angajati\n";
        cout << "3. Produse\n";
        cout << "4. Comenzi\n";
        cout << "5. Plati\n";
        cout << "6. Utilizatori\n";
        cout << "0. Inapoi\n";
        cout << "Optiune: ";
        cin >> opt;

        switch (opt) {
        case 1: afiseazaRezultate(conn, "SELECT * FROM Client"); break;
        case 2: afiseazaRezultate(conn, "SELECT * FROM Angajat"); break;
        case 3: afiseazaRezultate(conn, "SELECT * FROM Produs"); break;
        case 4: afiseazaRezultate(conn,
            "SELECT co.id_comanda, co.data_comanda, "
            "cl.nume, cl.prenume, "
            "a.nume AS angajat_nume, a.prenume AS angajat_prenume "
            "FROM Comanda co "
            "JOIN Client cl ON co.id_client = cl.id_client "
            "JOIN Angajat a ON co.id_angajat = a.id_angajat"); break;
        case 5: afiseazaRezultate(conn,
            "SELECT p.id_plata, p.suma, p.metoda_plata, "
            "p.id_comanda, co.data_comanda "
            "FROM Plata p "
            "JOIN Comanda co ON p.id_comanda = co.id_comanda"); break;
        case 6: afiseazaRezultate(conn,
            "SELECT id_utilizator, username, rol FROM Utilizatori"); break;
        case 0: break;
        default: cout << "Optiune invalida!\n";
        }
        if (opt != 0) pauseScreen();
    } while (opt != 0);
}

void adaugaClient(MYSQL* conn) {
    int id;
    string nume, pren, tel;
    cout << "ID Client: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, pren);
    cout << "Telefon: "; getline(cin, tel);

    string query = "INSERT INTO Client VALUES (" + to_string(id) + ", '" +
        escapeString(conn, nume) + "', '" + escapeString(conn, pren) + "', '" +
        escapeString(conn, tel) + "')";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Client adaugat!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void adaugaAngajat(MYSQL* conn) {
    int id;
    string nume, pren, functie;
    cout << "ID Angajat: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, pren);
    cout << "Functie: "; getline(cin, functie);

    string query = "INSERT INTO Angajat VALUES (" + to_string(id) + ", '" +
        escapeString(conn, nume) + "', '" + escapeString(conn, pren) + "', '" +
        escapeString(conn, functie) + "')";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Angajat adaugat!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void adaugaProdus(MYSQL* conn) {
    int id;
    string nume, categorie;
    double pret;
    cout << "ID Produs: "; cin >> id;
    cin.ignore();
    cout << "Nume produs: "; getline(cin, nume);
    cout << "Pret: "; cin >> pret;
    cin.ignore();
    cout << "Categorie: "; getline(cin, categorie);

    string query = "INSERT INTO Produs VALUES (" + to_string(id) + ", '" +
        escapeString(conn, nume) + "', " + to_string(pret) + ", '" +
        escapeString(conn, categorie) + "')";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Produs adaugat!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void adaugaComanda(MYSQL* conn) {
    int id, idCl, idAng;
    string data;
    cout << "ID Comanda: "; cin >> id;
    cin.ignore();
    cout << "Data (YYYY-MM-DD): "; getline(cin, data);
    cout << "ID Client: "; cin >> idCl;
    cout << "ID Angajat: "; cin >> idAng;

    string query = "INSERT INTO Comanda VALUES (" + to_string(id) + ", '" +
        escapeString(conn, data) + "', " + to_string(idCl) + ", " + to_string(idAng) + ")";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Comanda adaugata!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void adaugaPlata(MYSQL* conn) {
    int id, idCom;
    double suma;
    string metoda;
    cout << "ID Plata: "; cin >> id;
    cout << "Suma: "; cin >> suma;
    cin.ignore();
    cout << "Metoda plata (Card/Cash): "; getline(cin, metoda);
    cout << "ID Comanda: "; cin >> idCom;

    string query = "INSERT INTO Plata VALUES (" + to_string(id) + ", " +
        to_string(suma) + ", '" + escapeString(conn, metoda) + "', " + to_string(idCom) + ")";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Plata adaugata!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void adaugaUtilizator(MYSQL* conn) {
    string user, rol;
    cout << "Username: "; cin >> user;
    string pass1 = citesteParola();
    cout << "Confirma parola: ";
    string pass2 = citesteParola();
    if (pass1 != pass2) {
        cout << "Parolele nu coincid!\n";
        return;
    }
    cout << "Rol (admin/user): "; cin >> rol;

    string query = "INSERT INTO Utilizatori (username, parola, rol) VALUES ('" +
        escapeString(conn, user) + "', '" + escapeString(conn, pass1) + "', '" +
        escapeString(conn, rol) + "')";
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Utilizator adaugat!\n" : "Eroare: " + string(mysql_error(conn)) + "\n");
}

void menuAdaugare(MYSQL* conn) {
    int opt;
    do {
        clearScreen();
        cout << "========== ADAUGARE DATE ==========\n";
        cout << "1. Client\n";
        cout << "2. Angajat\n";
        cout << "3. Produs\n";
        cout << "4. Comanda\n";
        cout << "5. Plata\n";
        cout << "6. Utilizator\n";
        cout << "0. Inapoi\n";
        cout << "Optiune: ";
        cin >> opt;

        switch (opt) {
        case 1: adaugaClient(conn); break;
        case 2: adaugaAngajat(conn); break;
        case 3: adaugaProdus(conn); break;
        case 4: adaugaComanda(conn); break;
        case 5: adaugaPlata(conn); break;
        case 6: adaugaUtilizator(conn); break;
        case 0: break;
        default: cout << "Optiune invalida!\n";
        }
        if (opt != 0) pauseScreen();
    } while (opt != 0);
}

void editeazaClient(MYSQL* conn) {
    int id;
    string nume, pren, tel;
    cout << "ID Client de editat: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, pren);
    cout << "Telefon: "; getline(cin, tel);

    string query = "UPDATE Client SET nume='" + escapeString(conn, nume) +
        "', prenume='" + escapeString(conn, pren) +
        "', telefon='" + escapeString(conn, tel) +
        "' WHERE id_client=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Actualizat!\n" : "ID inexistent!\n");
}

void editeazaAngajat(MYSQL* conn) {
    int id;
    string nume, pren, functie;
    cout << "ID Angajat de editat: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, pren);
    cout << "Functie: "; getline(cin, functie);

    string query = "UPDATE Angajat SET nume='" + escapeString(conn, nume) +
        "', prenume='" + escapeString(conn, pren) +
        "', functie='" + escapeString(conn, functie) +
        "' WHERE id_angajat=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Actualizat!\n" : "ID inexistent!\n");
}

void editeazaProdus(MYSQL* conn) {
    int id;
    string nume, categorie;
    double pret;
    cout << "ID Produs de editat: "; cin >> id;
    cin.ignore();
    cout << "Nume produs: "; getline(cin, nume);
    cout << "Pret: "; cin >> pret;
    cin.ignore();
    cout << "Categorie: "; getline(cin, categorie);

    string query = "UPDATE Produs SET nume_produs='" + escapeString(conn, nume) +
        "', pret=" + to_string(pret) +
        ", categorie='" + escapeString(conn, categorie) +
        "' WHERE id_produs=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Actualizat!\n" : "ID inexistent!\n");
}

void editeazaComanda(MYSQL* conn) {
    int id, idCl, idAng;
    string data;
    cout << "ID Comanda de editat: "; cin >> id;
    cin.ignore();
    cout << "Data noua (YYYY-MM-DD): "; getline(cin, data);
    cout << "ID Client nou: "; cin >> idCl;
    cout << "ID Angajat nou: "; cin >> idAng;

    string query = "UPDATE Comanda SET data_comanda='" + escapeString(conn, data) +
        "', id_client=" + to_string(idCl) +
        ", id_angajat=" + to_string(idAng) +
        " WHERE id_comanda=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Actualizat!\n" : "ID inexistent!\n");
}

void editeazaPlata(MYSQL* conn) {
    int id, idCom;
    double suma;
    string metoda;
    cout << "ID Plata de editat: "; cin >> id;
    cout << "Suma noua: "; cin >> suma;
    cin.ignore();
    cout << "Metoda plata noua (Card/Cash): "; getline(cin, metoda);
    cout << "ID Comanda nou: "; cin >> idCom;

    string query = "UPDATE Plata SET suma=" + to_string(suma) +
        ", metoda_plata='" + escapeString(conn, metoda) +
        "', id_comanda=" + to_string(idCom) +
        " WHERE id_plata=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Actualizat!\n" : "ID inexistent!\n");
}

void menuEditare(MYSQL* conn) {
    int opt;
    do {
        clearScreen();
        cout << "========== EDITARE DATE ==========\n";
        cout << "1. Client\n";
        cout << "2. Angajat\n";
        cout << "3. Produs\n";
        cout << "4. Comanda\n";
        cout << "5. Plata\n";
        cout << "0. Inapoi\n";
        cout << "Optiune: ";
        cin >> opt;

        switch (opt) {
        case 1: editeazaClient(conn); break;
        case 2: editeazaAngajat(conn); break;
        case 3: editeazaProdus(conn); break;
        case 4: editeazaComanda(conn); break;
        case 5: editeazaPlata(conn); break;
        case 0: break;
        default: cout << "Optiune invalida!\n";
        }
        if (opt != 0) pauseScreen();
    } while (opt != 0);
}

void stergeDinTabel(MYSQL* conn, const string& tabel, const string& coloanaID, int id) {
    string query = "DELETE FROM " + tabel + " WHERE " + coloanaID + "=" + to_string(id);
    mysql_query(conn, query.c_str());
    cout << (mysql_affected_rows(conn) > 0 ? "Sters cu succes!\n" : "ID inexistent sau constrangere de integritate!\n");
}

void menuStergere(MYSQL* conn) {
    int opt, id;
    do {
        clearScreen();
        cout << "========== STERGERE DATE ==========\n";
        cout << "1. Client\n";
        cout << "2. Angajat\n";
        cout << "3. Produs\n";
        cout << "4. Comanda\n";
        cout << "5. Plata\n";
        cout << "6. Utilizator\n";
        cout << "0. Inapoi\n";
        cout << "Optiune: ";
        cin >> opt;

        if (opt >= 1 && opt <= 6) {
            cout << "ID de sters: "; cin >> id;
        }

        switch (opt) {
        case 1: stergeDinTabel(conn, "Client", "id_client", id); break;
        case 2: stergeDinTabel(conn, "Angajat", "id_angajat", id); break;
        case 3: stergeDinTabel(conn, "Produs", "id_produs", id); break;
        case 4: stergeDinTabel(conn, "Comanda", "id_comanda", id); break;
        case 5: stergeDinTabel(conn, "Plata", "id_plata", id); break;
        case 6: stergeDinTabel(conn, "Utilizatori", "id_utilizator", id); break;
        case 0: break;
        default: cout << "Optiune invalida!\n";
        }
        if (opt != 0) pauseScreen();
    } while (opt != 0);
}

void menuAdmin(MYSQL* conn, const string& username) {
    int opt;
    do {
        clearScreen();
        cout << "========== MENIU ADMINISTRATOR ==========\n";
        cout << "Utilizator: " << username << "\n\n";
        cout << "1. Vizualizare date\n";
        cout << "2. Adaugare date\n";
        cout << "3. Editare date\n";
        cout << "4. Stergere date\n";
        cout << "0. Deconectare\n";
        cout << "Optiune: ";
        cin >> opt;

        switch (opt) {
        case 1: menuVizualizare(conn); break;
        case 2: menuAdaugare(conn); break;
        case 3: menuEditare(conn); break;
        case 4: menuStergere(conn); break;
        case 0: cout << "Deconectare...\n"; break;
        default: cout << "Optiune invalida!\n"; pauseScreen();
        }
    } while (opt != 0);
}

void menuUser(MYSQL* conn, const string& username) {
    int opt;
    do {
        clearScreen();
        cout << "========== MENIU UTILIZATOR ==========\n";
        cout << "Utilizator: " << username << " (drepturi de citire)\n\n";
        cout << "1. Vizualizare Clienti\n";
        cout << "2. Vizualizare Angajati\n";
        cout << "3. Vizualizare Produse\n";
        cout << "4. Vizualizare Comenzi\n";
        cout << "5. Vizualizare Plati\n";
        cout << "0. Deconectare\n";
        cout << "Optiune: ";
        cin >> opt;

        switch (opt) {
        case 1: afiseazaRezultate(conn, "SELECT * FROM Client"); break;
        case 2: afiseazaRezultate(conn, "SELECT * FROM Angajat"); break;
        case 3: afiseazaRezultate(conn, "SELECT * FROM Produs"); break;
        case 4: afiseazaRezultate(conn,
            "SELECT co.id_comanda, co.data_comanda, "
            "cl.nume, cl.prenume, "
            "a.nume AS angajat_nume, a.prenume AS angajat_prenume "
            "FROM Comanda co "
            "JOIN Client cl ON co.id_client = cl.id_client "
            "JOIN Angajat a ON co.id_angajat = a.id_angajat"); break;
        case 5: afiseazaRezultate(conn,
            "SELECT p.id_plata, p.suma, p.metoda_plata, "
            "p.id_comanda, co.data_comanda "
            "FROM Plata p "
            "JOIN Comanda co ON p.id_comanda = co.id_comanda"); break;
        case 0: break;
        default: cout << "Optiune invalida!\n";
        }
        if (opt != 0) pauseScreen();
    } while (opt != 0);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    MYSQL* conn = connectDB();
    if (!conn) {
        pauseScreen();
        return 1;
    }
    pauseScreen();

    bool running = true;
    while (running) {
        string username;
        string rol = autentificare(conn, username);

        if (rol == "admin") {
            menuAdmin(conn, username);
        }
        else if (rol == "user") {
            menuUser(conn, username);
        }
        else {
            cout << "Autentificare esuata!\n";
            pauseScreen();
            continue;
        }

        char cont;
        cout << "\nDoresti sa te autentifici din nou? (d/n): ";
        cin >> cont;
        if (cont != 'd' && cont != 'D') running = false;
    }

    mysql_close(conn);
    cout << "Aplicatie incheiata.\n";
    return 0;
}
