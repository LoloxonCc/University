package exceptions;

public class Main {
    public static void main(String[] args) {
        FIFO q = new FIFO();

        System.out.println("Dodawanie 0,1,...,9");
        for (int i = 0; i < 10; i++) {
            perform(q, "ADD", i, 0);
        }

        System.out.println("Dodawanie parzystych 0,2,...,18");
        for (int i = 0; i <= 18; i += 2) {
            perform(q, "ADD", i, 0);
        }

        System.out.println("Zamiana 3i -> 4i");
        for (int i = 0; i < 10; i++) {
            perform(q, "CHANGE", 3 * i, 4 * i);
        }

        System.out.println("Opróżnianie");
        while (true) {
            try {
                int val = q.remove();
                System.out.print(val + " ");
            } catch (RandomException e) {
                System.err.println("Błąd odczytu/usuwania, ponawiamy");
            } catch (UnexpectedBehavior e) {
                // Koniec kolejki lub błąd logiczny
                break;
            }
        }
        System.out.println();
    }

    private static void perform(FIFO q, String action, int v1, int v2) {
        boolean success = false;

        while (!success) {
            try {
                switch (action) {
                    case "ADD" -> q.add(v1);
                    case "CHANGE" -> q.change(v1, v2);
                }
                success = true;
            } catch (RandomException e) {
                System.err.println("[AWARIA POJEMNIKA] " + e.getMessage() + " - Próbujemy ponownie");
            } catch (UnexpectedBehavior e) {
                System.err.println("[NIEDOZWOLONA OPERACJA NA KOLEJCE] " + e.getMessage());
                success = true; // Przechodzimy do kolejnego elementu
            }
        }
    }
}
