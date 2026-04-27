package graphs;

public class LIFO extends Queue {

    public LIFO(){
        super();
    }

    public void add(Integer element) {
        elements.push(element);
    }

    public Integer retrieve() {
        if (elements.isEmpty()) return null;
        return elements.pop();
    }
}