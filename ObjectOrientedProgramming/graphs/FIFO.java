package graphs;

public class FIFO extends Queue {

    public FIFO() {
        super();
    }

    public void add(Integer element) {
        elements.offer(element);
    }

    public Integer retrieve() {
        return elements.poll();
    }
}