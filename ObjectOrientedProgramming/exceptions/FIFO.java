package wyjątki;

import java.util.*;

public class FIFO {
    private final LinkedList<WadliwyInt> queue = new LinkedList<>();
    private final Set<Integer> values = new HashSet<>();

    public void add(int value) throws UnexpectedBehavior, RandomException {
        // Sprawdzenie unikalności (nie zmienia stanu)
        if (values.contains(value)) {
            throw new UnexpectedBehavior("Wartość " + value + " już jest w kolejce");
        }

        // Próba stworzenia pojemnika (może być RandomException)
        WadliwyInt nowy = new WadliwyInt(value);

        // Aktualizacja stanu
        queue.addLast(nowy);
        values.add(value);
    }

    public int remove() throws UnexpectedBehavior, RandomException {
        if (queue.isEmpty()) {
            throw new UnexpectedBehavior("Kolejka jest pusta");
        }

        // Pobieramy pierwszy element bez usuwania go jeszcze
        WadliwyInt first = queue.getFirst();

        // Próba odczytu (może być RandomException)
        int val = first.get();

        // Jeśli get() się udał usuwamy z kolejki
        queue.removeFirst();
        values.remove(val);

        return val;
    }

    public void change(int value1, int value2) throws UnexpectedBehavior, RandomException {
        if (!values.contains(value1)) {
            throw new UnexpectedBehavior("Nie znaleziono wartości " + value1);
        }
        if (value1 != value2 && values.contains(value2)) {
            throw new UnexpectedBehavior("Wartość " + value2 + " już jest w kolejce");
        }

        // Szukamy pojemnika (nie zmienia stanu)
        WadliwyInt target = null;
        for (WadliwyInt p : queue) {
            // get() może dać wyjątek, jeśli da, metoda zostanie przerwana, a stan 'values' i 'queue' pozostanie bez zmian.
            if (p.get() == value1) {
                target = p;
                break;
            }
        }

        if (target != null) {
            // Próba zmiany wartości (może dać RandomException)
            target.set(value2);

            // Jeśli set() się udał to aktualizujemy zbiór unikalności
            values.remove(value1);
            values.add(value2);
        }
    }
}