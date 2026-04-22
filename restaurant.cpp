#include <iostream>
#include <iomanip>
#include <string>
#include <mysql/mysql.h>

using namespace std;

// ============================================================
// CONFIGURARE CONEXIUNE
// ============================================================
const char* HOST = "localhost";
const char* USER = "root";       // modifica daca e cazul
const char* PASS = "";           // modifica daca e cazul
const char* DB   = "Restaurant";

// ============================================================
// VARIABILE GLOBALE
// ============================================================
MYSQL* conn;
string currentUser;
string currentRole;

// ============================================================
// FUNCȚII UTILITARE
// ============================================================
bool connectDB() {
    conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, HOST, USER, PASS, DB, 0, nullptr, 0)) {
        cerr << "Eroare conectare BD: " << mysql_error(conn) << endl;
        return false;
    }
    cout << "Conectat la baza de date Restaurant!\n\n";
    return true;
}

void printLine(int cols, int widths[]) {
    for (int c = 0; c < cols; ++c) {
        cout << "+";
        for (int i = 0; i < widths[c] + 2; ++i) cout << "-";
    }
    cout << "+\n";
}

void executeSelect(const string& query) {
    if (mysql_query(conn, query.c_str())) {
        cerr << "Eroare interogare: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        cerr << "Eroare rezultat: " << mysql_error(conn) << endl;
        return;
    }

    int numFields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    int* widths = new int[numFields];

    // Calcul latime coloane
    for (int i = 0; i < numFields; ++i) {
        widths[i] = string(fields[i].name).length();
    }
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < numFields; ++i) {
            int len = row[i] ? string(row[i]).length() : 4;
            if (len > widths[i]) widths[i] = len;
        }
    }
    mysql_data_seek(res, 0);

    // Header
    printLine(numFields, widths);
    for (int i = 0; i < numFields; ++i) {
        cout << "| " << left << setw(widths[i]) << fields[i].name << " ";
    }
    cout << "|\n";
    printLine(numFields, widths);

    // Rows
    while ((row = mysql_fetch_row(res))) {
        for (int i = 0; i < numFields; ++i) {
            string val = row[i] ? row[i] : "NULL";
            cout << "| " << left << setw(widths[i]) << val << " ";
        }
        cout << "|\n";
    }
    printLine(numFields, widths);

    delete[] widths;
    mysql_free_result(res);
}

bool executeUpdate(const string& query) {
    if (mysql_query(conn, query.c_str())) {
        cerr << "Eroare: " << mysql_error(conn) << endl;
        return false;
    }
    return true;
}

// ============================================================
// AUTENTIFICARE
// ============================================================
bool login() {
    string user, pass;
    cout << "===== AUTENTIFICARE =====\n";
    cout << "Username: "; cin >> user;
    cout << "Parola: ";   cin >> pass;

    string query = "SELECT rol FROM Utilizatori WHERE username='" + user + "' AND parola='" + pass + "'";
    if (mysql_query(conn, query.c_str())) {
        cerr << "Eroare login!\n";
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return false;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        currentUser = user;
        currentRole = row[0];
        cout << "\nLogin reusit! Rol: " << currentRole << "\n\n";
        mysql_free_result(res);
        return true;
    }

    cout << "Username sau parola incorecta!\n\n";
    mysql_free_result(res);
    return false;
}

// ============================================================
// FUNCȚII VIZUALIZARE (USER & ADMIN)
// ============================================================
void viewClients() {
    cout << "\n--- CLIENTI ---\n";
    executeSelect("SELECT * FROM Client");
}
void viewEmployees() {
    cout << "\n--- ANGAJATI ---\n";
    executeSelect("SELECT * FROM Angajat");
}
void viewProducts() {
    cout << "\n--- PRODUSE ---\n";
    executeSelect("SELECT * FROM Produs");
}
void viewOrders() {
    cout << "\n--- COMENZI ---\n";
    executeSelect("SELECT c.id_comanda, c.data_comanda, cl.nume AS client, a.nume AS angajat FROM Comanda c JOIN Client cl ON c.id_client=cl.id_client JOIN Angajat a ON c.id_angajat=a.id_angajat");
}
void viewPayments() {
    cout << "\n--- PLATI ---\n";
    executeSelect("SELECT p.id_plata, p.suma, p.metoda_plata, c.id_comanda FROM Plata p JOIN Comanda c ON p.id_comanda=c.id_comanda");
}

// ============================================================
// FUNCȚII ADMIN - CRUD CLIENT
// ============================================================
void addClient() {
    int id; string nume, prenume, tel;
    cout << "ID Client: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, prenume);
    cout << "Telefon: "; getline(cin, tel);
    string q = "INSERT INTO Client VALUES (" + to_string(id) + ",'" + nume + "','" + prenume + "','" + tel + "')";
    if (executeUpdate(q)) cout << "Client adaugat!\n";
}
void editClient() {
    int id; string nume, prenume, tel;
    cout << "ID Client de modificat: "; cin >> id;
    cin.ignore();
    cout << "Nume nou: "; getline(cin, nume);
    cout << "Prenume nou: "; getline(cin, prenume);
    cout << "Telefon nou: "; getline(cin, tel);
    string q = "UPDATE Client SET nume='" + nume + "', prenume='" + prenume + "', telefon='" + tel + "' WHERE id_client=" + to_string(id);
    if (executeUpdate(q)) cout << "Client actualizat!\n";
}
void deleteClient() {
    int id;
    cout << "ID Client de sters: "; cin >> id;
    string q = "DELETE FROM Client WHERE id_client=" + to_string(id);
    if (executeUpdate(q)) cout << "Client sters!\n";
}

// ============================================================
// FUNCȚII ADMIN - CRUD ANGAJAT
// ============================================================
void addEmployee() {
    int id; string nume, prenume, functie;
    cout << "ID Angajat: "; cin >> id;
    cin.ignore();
    cout << "Nume: "; getline(cin, nume);
    cout << "Prenume: "; getline(cin, prenume);
    cout << "Functie: "; getline(cin, functie);
    string q = "INSERT INTO Angajat VALUES (" + to_string(id) + ",'" + nume + "','" + prenume + "','" + functie + "')";
    if (executeUpdate(q)) cout << "Angajat adaugat!\n";
}
void editEmployee() {
    int id; string nume, prenume, functie;
    cout << "ID Angajat de modificat: "; cin >> id;
    cin.ignore();
    cout << "Nume nou: "; getline(cin, nume);
    cout << "Prenume nou: "; getline(cin, prenume);
    cout << "Functie noua: "; getline(cin, functie);
    string q = "UPDATE Angajat SET nume='" + nume + "', prenume='" + prenume + "', functie='" + functie + "' WHERE id_angajat=" + to_string(id);
    if (executeUpdate(q)) cout << "Angajat actualizat!\n";
}
void deleteEmployee() {
    int id;
    cout << "ID Angajat de sters: "; cin >> id;
    string q = "DELETE FROM Angajat WHERE id_angajat=" + to_string(id);
    if (executeUpdate(q)) cout << "Angajat sters!\n";
}

// ============================================================
// FUNCȚII ADMIN - CRUD PRODUS
// ============================================================
void addProduct() {
    int id; string nume, cat; double pret;
    cout << "ID Produs: "; cin >> id;
    cin.ignore();
    cout << "Nume produs: "; getline(cin, nume);
    cout << "Pret: "; cin >> pret;
    cin.ignore();
    cout << "Categorie: "; getline(cin, cat);
    string q = "INSERT INTO Produs VALUES (" + to_string(id) + ",'" + nume + "'," + to_string(pret) + ",'" + cat + "')";
    if (executeUpdate(q)) cout << "Produs adaugat!\n";
}
void editProduct() {
    int id; string nume, cat; double pret;
    cout << "ID Produs de modificat: "; cin >> id;
    cin.ignore();
    cout << "Nume nou: "; getline(cin, nume);
    cout << "Pret nou: "; cin >> pret;
    cin.ignore();
    cout << "Categorie noua: "; getline(cin, cat);
    string q = "UPDATE Produs SET nume_produs='" + nume + "', pret=" + to_string(pret) + ", categorie='" + cat + "' WHERE id_produs=" + to_string(id);
    if (executeUpdate(q)) cout << "Produs actualizat!\n";
}
void deleteProduct() {
    int id;
    cout << "ID Produs de sters: "; cin >> id;
    string q = "DELETE FROM Produs WHERE id_produs=" + to_string(id);
    if (executeUpdate(q)) cout << "Produs sters!\n";
}

// ============================================================
// FUNCȚII ADMIN - CRUD COMANDA
// ============================================================
void addOrder() {
    int id, idc, ida; string data;
    cout << "ID Comanda: "; cin >> id;
    cout << "ID Client: "; cin >> idc;
    cout << "ID Angajat: "; cin >> ida;
    cin.ignore();
    cout << "Data (YYYY-MM-DD): "; getline(cin, data);
    string q = "INSERT INTO Comanda VALUES (" + to_string(id) + ",'" + data + "'," + to_string(idc) + "," + to_string(ida) + ")";
    if (executeUpdate(q)) cout << "Comanda adaugata!\n";
}
void editOrder() {
    int id, idc, ida; string data;
    cout << "ID Comanda de modificat: "; cin >> id;
    cout << "ID Client nou: "; cin >> idc;
    cout << "ID Angajat nou: "; cin >> ida;
    cin.ignore();
    cout << "Data noua (YYYY-MM-DD): "; getline(cin, data);
    string q = "UPDATE Comanda SET data_comanda='" + data + "', id_client=" + to_string(idc) + ", id_angajat=" + to_string(ida) + " WHERE id_comanda=" + to_string(id);
    if (executeUpdate(q)) cout << "Comanda actualizata!\n";
}
void deleteOrder() {
    int id;
    cout << "ID Comanda de sters: "; cin >> id;
    string q = "DELETE FROM Comanda WHERE id_comanda=" + to_string(id);
    if (executeUpdate(q)) cout << "Comanda stearsa!\n";
}

// ============================================================
// FUNCȚII ADMIN - CRUD PLATA
// ============================================================
void addPayment() {
    int id, idc; double suma; string metoda;
    cout << "ID Plata: "; cin >> id;
    cout << "Suma: "; cin >> suma;
    cin.ignore();
    cout << "Metoda (Card/Cash): "; getline(cin, metoda);
    cout << "ID Comanda: "; cin >> idc;
    string q = "INSERT INTO Plata VALUES (" + to_string(id) + "," + to_string(suma) + ",'" + metoda + "'," + to_string(idc) + ")";
    if (executeUpdate(q)) cout << "Plata adaugata!\n";
}
void editPayment() {
    int id, idc; double suma; string metoda;
    cout << "ID Plata de modificat: "; cin >> id;
    cout << "Suma noua: "; cin >> suma;
    cin.ignore();
    cout << "Metoda noua: "; getline(cin, metoda);
    cout << "ID Comanda nou: "; cin >> idc;
    string q = "UPDATE Plata SET suma=" + to_string(suma) + ", metoda_plata='" + metoda + "', id_comanda=" + to_string(idc) + " WHERE id_plata=" + to_string(id);
    if (executeUpdate(q)) cout << "Plata actualizata!\n";
}
void deletePayment() {
    int id;
    cout << "ID Plata de sters: "; cin >> id;
    string q = "DELETE FROM Plata WHERE id_plata=" + to_string(id);
    if (executeUpdate(q)) cout << "Plata stearsa!\n";
}

// ============================================================
// MENIU ADMIN
// ============================================================
void menuAdmin() {
    int opt;
    do {
        cout << "\n========== MENIU ADMINISTRATOR ==========\n";
        cout << "1.  Vizualizare Clienti\n";
        cout << "2.  Adaugare Client\n";
        cout << "3.  Editare Client\n";
        cout << "4.  Stergere Client\n";
        cout << "5.  Vizualizare Angajati\n";
        cout << "6.  Adaugare Angajat\n";
        cout << "7.  Editare Angajat\n";
        cout << "8.  Stergere Angajat\n";
        cout << "9.  Vizualizare Produse\n";
        cout << "10. Adaugare Produs\n";
        cout << "11. Editare Produs\n";
        cout << "12. Stergere Produs\n";
        cout << "13. Vizualizare Comenzi\n";
        cout << "14. Adaugare Comanda\n";
        cout << "15. Editare Comanda\n";
        cout << "16. Stergere Comanda\n";
        cout << "17. Vizualizare Plati\n";
        cout << "18. Adaugare Plata\n";
        cout << "19. Editare Plata\n";
        cout << "20. Stergere Plata\n";
        cout << "0.  Deconectare\n";
        cout << "Optiune: "; cin >> opt;

        switch (opt) {
            case 1: viewClients(); break;
            case 2: addClient(); break;
            case 3: editClient(); break;
            case 4: deleteClient(); break;
            case 5: viewEmployees(); break;
            case 6: addEmployee(); break;
            case 7: editEmployee(); break;
            case 8: deleteEmployee(); break;
            case 9: viewProducts(); break;
            case 10: addProduct(); break;
            case 11: editProduct(); break;
            case 12: deleteProduct(); break;
            case 13: viewOrders(); break;
            case 14: addOrder(); break;
            case 15: editOrder(); break;
            case 16: deleteOrder(); break;
            case 17: viewPayments(); break;
            case 18: addPayment(); break;
            case 19: editPayment(); break;
            case 20: deletePayment(); break;
            case 0: cout << "Deconectare...\n"; break;
            default: cout << "Optiune invalida!\n";
        }
    } while (opt != 0);
}

// ============================================================
// MENIU USER
// ============================================================
void menuUser() {
    int opt;
    do {
        cout << "\n========== MENIU UTILIZATOR (Read-Only) ==========\n";
        cout << "1. Vizualizare Clienti\n";
        cout << "2. Vizualizare Angajati\n";
        cout << "3. Vizualizare Produse\n";
        cout << "4. Vizualizare Comenzi\n";
        cout << "5. Vizualizare Plati\n";
        cout << "0. Deconectare\n";
        cout << "Optiune: "; cin >> opt;

        switch (opt) {
            case 1: viewClients(); break;
            case 2: viewEmployees(); break;
            case 3: viewProducts(); break;
            case 4: viewOrders(); break;
            case 5: viewPayments(); break;
            case 0: cout << "Deconectare...\n"; break;
            default: cout << "Optiune invalida!\n";
        }
    } while (opt != 0);
}

// ============================================================
// MAIN
// ============================================================
int main() {
    if (!connectDB()) return 1;

    int opt;
    do {
        cout << "========== RESTAURANT DB APP ==========\n";
        cout << "1. Login\n";
        cout << "0. Iesire\n";
        cout << "Optiune: "; cin >> opt;

        if (opt == 1) {
            if (login()) {
                if (currentRole == "admin")
                    menuAdmin();
                else
                    menuUser();
            }
        }
    } while (opt != 0);

    mysql_close(conn);
    cout << "La revedere!\n";
    return 0;
}