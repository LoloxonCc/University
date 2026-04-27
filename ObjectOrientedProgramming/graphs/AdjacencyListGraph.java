package graphs;

import java.util.LinkedList;
import java.util.List;

public class AdjacencyListGraph extends Graph {

    private List<List<Integer>> graph;

    public AdjacencyListGraph(int n) {
        super(n);

        graph = new LinkedList<>();

        for(int i = 0; i < n; i++)
            graph.add(new LinkedList<Integer>());
    }

    @Override
    public void add(int a, int b) {
        super.add(a, b);
        graph.get(a).add(b);
    }

    @Override
    public void remove(int a, int b) {
        super.remove(a, b);
        boolean removed = graph.get(a).remove(Integer.valueOf(b));
        assert removed : "This edge does not exist!";
    }

    @Override
    public List<Integer> getNeighbors(int a) {
        super.getNeighbors(a);
        return new LinkedList<>(graph.get(a));
    }
}