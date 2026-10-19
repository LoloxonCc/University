package lab03.assignments;

import java.util.ArrayList;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.function.IntBinaryOperator;

public class VectorStream {
    private static final int STREAM_LENGTH = 10;
    private static final int VECTOR_LENGTH = 100;

    /**
     * Function that defines how vectors are computed: the i-th element depends on
     * the previous sum and the index i.
     * The sum of elements in the previous vector is initially given as zero.
     */
    private final static IntBinaryOperator vectorDefinition = (previousSum, i) -> {
        int a = 2 * i + 1;
        return (previousSum / VECTOR_LENGTH + 1) * (a % 4 - 2) * a + 1;
    };

    private static void computeVectorStreamSequentially() {
        int[] vector = new int[VECTOR_LENGTH];
        int sum = 0;
        for (int vectorNo = 0; vectorNo < STREAM_LENGTH; ++vectorNo) {
            for (int i = 0; i < VECTOR_LENGTH; ++i) {
                vector[i] = vectorDefinition.applyAsInt(sum, i);
            }
            sum = 0;
            for (int x : vector) {
                sum += x;
            }
            System.out.println(vectorNo + " -> " + sum);
        }
    }

    private static class SharedSum {
        int sum = 0;

        public int getSum() {
            return sum;
        }
        
        public void setSum(int sum) {
            this.sum = sum;
        }
    }

    private static class ElementCalculator implements Runnable {
        private final int i;
        private final int[] vector;
        private final CyclicBarrier barrier;
        private final SharedSum sum;

        public ElementCalculator(int i, int[] vector, CyclicBarrier barrier, SharedSum sum) {
            this.i = i;
            this.vector = vector;
            this.barrier = barrier;
            this.sum = sum;
        }

        public void run() {
            try {
                for(int vectorNo = 0; vectorNo < STREAM_LENGTH; ++vectorNo) {
                    vector[i] = vectorDefinition.applyAsInt(sum.getSum(), i);
                    barrier.await();
                }
            } catch (InterruptedException | BrokenBarrierException e) {
                Thread.currentThread().interrupt();
                System.err.println(Thread.currentThread().getName() +  " interrupted.");
            }
        }
    }

    private static class SumCalculator implements Runnable {
        private final int[] vector;
        private final SharedSum sum;
        private int vectorNo = 0;

        public SumCalculator(int[] vector, SharedSum sum) {
            this.vector = vector;
            this.sum = sum;
        }

        public void run() {
            int newSum = 0;
            for(int x : vector) {
                newSum += x;
            }

            sum.setSum(newSum);
            System.out.println(vectorNo + " -> " + sum.getSum());
            vectorNo++;
        }
    }

    private static void computeVectorStreamInParallel() throws InterruptedException {
        int[] vector = new int[VECTOR_LENGTH];
        SharedSum sum = new SharedSum();

        CyclicBarrier barrier = new CyclicBarrier(VECTOR_LENGTH, new SumCalculator(vector, sum));
        ArrayList<Thread> threads = new ArrayList<>();

        for(int i = 0; i < VECTOR_LENGTH; i++)
            threads.add(new Thread(new ElementCalculator(i, vector, barrier, sum)));

        for(Thread t : threads)
            t.start();

        for(Thread t : threads)
            t.join();
    }

    public static void main(String[] args) {
        try {
            System.out.println("-- Sequentially --");
            computeVectorStreamSequentially();
            System.out.println("-- Parallel --");
            computeVectorStreamInParallel();
            System.out.println("-- End --");
        } catch (InterruptedException e) {
            System.err.println("Main interrupted.");
        }
    }
}