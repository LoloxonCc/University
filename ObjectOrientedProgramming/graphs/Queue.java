package graphs;

import java.util.Deque;
import java.util.LinkedList;

public abstract class Queue {

    protected Deque<Integer> elements = new LinkedList<>();

    public boolean isEmpty() {
        return elements.isEmpty();
    }

    public abstract void add(Integer element);
    public abstract Integer retrieve();
}