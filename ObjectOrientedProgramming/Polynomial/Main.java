/*
Author: Karol Synowiec, Date: April 9, 2026,
Demonstration of the Polynomial class usage.
 */
public class Main {
    public static void main(String[] args) {
        // Initializing Polynomials:
        Polynomial w1 = new Polynomial(new float[]{4, 3, 2, 2, 12});
        System.out.println("W1: " + w1);

        Polynomial w2 = new Polynomial(new int[]{1, 1});
        System.out.println("W2: " + w2);

        Polynomial w3 = new Polynomial(new float[]{3.0f, 4.0f, 2.24f, 3.14f, 4, 12.67f});
        System.out.println("W3: " + w3);

        Polynomial w4 = new Polynomial(new float[]{0.51f, 0.15f, 101, 4.567f, 3.1415f, 0, 0, 0, 0, 16, 4.05f});
        System.out.println("W4: " + w4);

        // Addition
        w1.add(w2);
        System.out.println("W1 + W2: " + w1);

        // Subtraction
        w4.subtract(w3);
        System.out.println("W4 - W3: " + w4);

        // Multiplication
        Polynomial a = new Polynomial(new int[]{1, 1});
        Polynomial b = new Polynomial(new int[]{-1, 1});
        a.multiply(b);
        System.out.println("(x+1)(x-1) = " + a);
        w3.multiply(w2);
        System.out.println("W3 * W2 = " +  w3);

        // Division
        Polynomial c = new Polynomial(new float[]{-1, 0, 1});
        Polynomial d = new Polynomial(new float[]{1, 1});
        c.divide(d);
        System.out.println("(x^2-1) / (x+1) = " + c);

        // Derivative and Integral
        Polynomial p = new Polynomial(new float[]{0, 2, 3, 0, 0.45f});
        Polynomial q = new Polynomial(new int[]{1, 0, 2});
        System.out.println("Derivative of 0.45x^4 + 3x^2 + 2x: " + p.derivative());
        System.out.println("Indefinite integral of 0.45x^4 + 3x^2 + 2x: " + p.indefiniteIntegral());
        System.out.println("Derivative of 1 + 2x^2: " + q.derivative());
        System.out.println("Indefinite integral of 1 + 2x^2: " + q.indefiniteIntegral());

        // Value at a point
        System.out.println("Value of 0.45x^4 + 3x^2 + 2x for x=2: " + p.evaluate(2));
        System.out.println("Value of 1 + 2x^2 for x=2: " + q.evaluate(2));

        // INVALID OPERATIONS:

        //Polynomial zero = new Polynomial();
        //w1.divide(zero); // Division by zero

        //Polynomial large = new Polynomial(new float[]{0,0,0,0,0,0,0,0,0,0,1}); // x^10
        //large.multiply(new Polynomial(new int[]{0, 1})); // x^11 - degree limit exceeded

        //Polynomial indivisible = new Polynomial(new int[]{1, 1}); // x+1
        //indivisible.divide(new Polynomial(new int[]{0, 1})); // (x+1)/x - division with remainder
    }
}
