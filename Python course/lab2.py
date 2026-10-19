def sumuj_czas(aktywnosc):
    suma = 0
    for i in czas_aktywnosci[aktywnosc]:
        suma += i
    return suma

def dodaj_aktywnosc():
    aktywnosc = input("Podaj aktywnosc: ")
    czas = int(input("Podaj czas: "))
    if aktywnosc not in czas_aktywnosci:
        czas_aktywnosci[aktywnosc] = [czas]
    else:
        czas_aktywnosci[aktywnosc].append(czas)

def pokaz_czas():
    aktywnosc = input("Podaj aktywnosc: ")
    if aktywnosc not in czas_aktywnosci:
        print("Nie ma takiej aktywnosci")
        return
    else:
        print(aktywnosc, ":", sumuj_czas(aktywnosc))

def pokaz_top_aktywnosc():
    if not czas_aktywnosci:
        print("Brak zapisanych aktywności.")
        return

    lista_aktywnosci = []
    for aktywnosc in czas_aktywnosci:
        laczny_czas = sumuj_czas(aktywnosc)
        lista_aktywnosci.append((laczny_czas, aktywnosc))

    lista_aktywnosci.sort(reverse=True)

    print("Top aktywności:")
    for czas, aktywnosc in lista_aktywnosci[:3]:
        print(aktywnosc)

czas_aktywnosci = {}

print("Mozliwe akcje: ")
print("+ : dodaj aktywnosc")
print("= : pokaz czas wybranej aktywnosci")
print("* : pokaz top 3 aktywnosci")
print("x : zakoncz program")

while(True):
    akcja = input("Podaj akcje: ")
    if akcja == "x":
        break
    elif akcja == "+":
        dodaj_aktywnosc()
    elif akcja == "=":
        pokaz_czas()
    elif akcja == "*":
        pokaz_top_aktywnosc()
    else:
        print("Niepoprawna akcja!")
