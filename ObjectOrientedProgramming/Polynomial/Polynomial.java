/*
Author: Karol Synowiec, Date: April 9, 2026,
Polynomial calculator with a maximum degree of 10.
 */
public class Polynomial {

    private float[] coefficients = new float[11];
    private int degree;

    private void updateDegree() {
        int i = 10;
        while (i >= 0 && Math.abs(coefficients[i]) < 0.00001f) i--;
        degree = i;
    }

    public Polynomial(int[] coefficients) {
        assert coefficients.length <= 11 : "Degree too high!";
        this.degree = -1;
        for (int i = 0; i < coefficients.length; i++)
            this.coefficients[i] = coefficients[i];
        for (int i = coefficients.length; i < 11; i++)
            this.coefficients[i] = 0;
        updateDegree();
    }

    public Polynomial(float[] coefficients) {
        assert coefficients.length <= 11 : "Degree too high!";
        this.degree = -1;
        for (int i = 0; i < coefficients.length; i++)
            this.coefficients[i] = coefficients[i];
        for (int i = coefficients.length; i < 11; i++)
            this.coefficients[i] = 0.0f;
        updateDegree();
    }

    public Polynomial() {
        for (int i = 0; i < 11; i++)
            coefficients[i] = 0;
        degree = -1;
    }

    public void add(Polynomial p) {
        for (int i = 0; i <= p.degree; i++)
            coefficients[i] += p.coefficients[i];

        updateDegree();
    }

    public void subtract(Polynomial p) {
        for (int i = 0; i <= p.degree; i++)
            coefficients[i] -= p.coefficients[i];
        updateDegree();
    }

    public void multiply(Polynomial p) {
        if (degree == -1 || p.degree == -1) {
            for (int i = 0; i < 11; i++) coefficients[i] = 0;
            degree = -1;
            return;
        }
        assert degree + p.degree <= 10 : "Multiplication result would exceed maximum degree!";

        float[] result = new float[11];
        for (int i = 0; i <= degree; i++)
            for (int j = 0; j <= p.degree; j++)
                result[i + j] += coefficients[i] * p.coefficients[j];

        coefficients = result.clone();
        updateDegree();
    }

    public void divide(Polynomial p) {
        assert p.degree != -1 : "Cannot divide by zero!";

        float[] remainder = coefficients.clone();
        float[] quotient = new float[11];
        int remainderDegree = degree;

        while (remainderDegree >= p.degree && remainderDegree != -1) {
            int power = remainderDegree - p.degree;
            float factor = remainder[remainderDegree] / p.coefficients[p.degree];
            quotient[power] = factor;

            for (int i = 0; i <= p.degree; i++)
                remainder[i + power] -= p.coefficients[i] * factor;

            int i = 10;
            while (i >= 0 && Math.abs(remainder[i]) < 0.00001f) {
                remainder[i] = 0;
                i--;
            }
            remainderDegree = i;
        }
        assert remainderDegree == -1 : "Polynomials must be divisible without a remainder!";
        coefficients = quotient;
        updateDegree();
    }

    public float evaluate(float x) {
        float result = 0;

        for (int i = 0; i < 11; i++)
            result += coefficients[i] * (float) Math.pow(x, i);

        return result;
    }

    public Polynomial indefiniteIntegral() {
        assert degree <= 9 : "Integral degree would exceed maximum limit!";
        float[] integral = new float[11];
        integral[0] = 0;

        for (int i = 0; i <= degree; i++)
            integral[i + 1] = coefficients[i] / (i + 1.0f);

        return new Polynomial(integral);
    }

    public Polynomial derivative() {
        float[] derivative = new float[11];

        for (int i = 1; i <= degree; i++) {
            derivative[i - 1] = i * coefficients[i];
        }

        return new Polynomial(derivative);
    }

    @Override
    public String toString() {
        if (degree == -1) return "0";
        String output = "";
        float epsilon = 0.00001f; // Rounding threshold

        for (int i = degree; i >= 0; i--) {
            if (coefficients[i] == 0) continue;

            if (i < degree) {
                if (coefficients[i] > 0) output += " + ";
                else output += " - ";
            } else {
                if (coefficients[i] < 0) output += "-";
            }

            float absValue = Math.abs(coefficients[i]);
            String valueStr;

            if (Math.abs(absValue - Math.round(absValue)) < epsilon) {
                valueStr = String.valueOf(Math.round(absValue));
            } else {
                valueStr = String.valueOf(absValue);
            }

            boolean isOne = Math.abs(absValue - 1.0f) < epsilon;

            if (!isOne || i == 0) {
                output += valueStr;
                if (i > 0) output += " ";
            }

            if (i > 0) {
                output += "x";
                if (i > 1) {
                    output += "^" + i;
                }
            }
        }
        return output;
    }
}
