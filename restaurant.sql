DROP DATABASE IF EXISTS Restaurant;
CREATE DATABASE Restaurant;
USE Restaurant;

CREATE TABLE Client (
    id_client INT PRIMARY KEY,
    nume VARCHAR(100),
    prenume VARCHAR(100),
    telefon VARCHAR(15)
);

CREATE TABLE Angajat (
    id_angajat INT PRIMARY KEY,
    nume VARCHAR(100),
    prenume VARCHAR(100),
    functie VARCHAR(50)
);

CREATE TABLE Comanda (
    id_comanda INT PRIMARY KEY,
    data_comanda DATE,
    id_client INT,
    id_angajat INT,
    FOREIGN KEY (id_client) REFERENCES Client(id_client),
    FOREIGN KEY (id_angajat) REFERENCES Angajat(id_angajat)
);

CREATE TABLE Plata (
    id_plata INT PRIMARY KEY,
    suma DECIMAL(8,2),
    metoda_plata VARCHAR(30),
    id_comanda INT,
    FOREIGN KEY (id_comanda) REFERENCES Comanda(id_comanda)
);

CREATE TABLE Produs (
    id_produs INT PRIMARY KEY,
    nume_produs VARCHAR(100),
    pret DECIMAL(8,2),
    categorie VARCHAR(50)
);

-- TABEL NOU: Utilizatori pentru autentificare
CREATE TABLE Utilizatori (
    id_utilizator INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    parola VARCHAR(100) NOT NULL,
    rol ENUM('admin', 'user') DEFAULT 'user'
);

-- Date de test
INSERT INTO Client VALUES
(1, 'Popescu', 'Ion', '069123456'),
(2, 'Ionescu', 'Maria', '068111222'),
(3, 'Rusu', 'Andrei', '079333444'),
(4, 'Munteanu', 'Elena', '060555666'),
(5, 'Ciobanu', 'Alex', '067777888');

INSERT INTO Angajat VALUES
(1, 'Popa', 'Mihai', 'Chelner'),
(2, 'Vasilescu', 'Laura', 'Chelner'),
(3, 'Ilie', 'George', 'Bucatar'),
(4, 'Petrescu', 'Ana', 'Casier'),
(5, 'Matei', 'Radu', 'Manager');

INSERT INTO Produs VALUES
(1, 'Pizza Margherita', 85.00, 'Pizza'),
(2, 'Paste Carbonara', 75.50, 'Paste'),
(3, 'Salata Caesar', 60.00, 'Salate'),
(4, 'Coca-Cola', 25.00, 'Bauturi'),
(5, 'Tiramisu', 45.00, 'Desert');

INSERT INTO Comanda VALUES
(1, '2025-02-01', 1, 1),
(2, '2025-02-02', 2, 2),
(3, '2025-02-03', 3, 1),
(4, '2025-02-04', 4, 3),
(5, '2025-02-05', 5, 4);

INSERT INTO Plata VALUES
(1, 150.00, 'Card', 1),
(2, 85.50, 'Cash', 2),
(3, 120.00, 'Card', 3),
(4, 60.00, 'Cash', 4),
(5, 200.00, 'Card', 5);

-- Utilizatori de test
INSERT INTO Utilizatori (username, parola, rol) VALUES
('admin', 'admin123', 'admin'),
('user', 'user123', 'user');