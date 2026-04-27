package graphs;

import java.util.ArrayList;
import java.util.List;

public class AdjacencyMatrixGraph extends Graph {

    private int[][] graph;

    public AdjacencyMatrixGraph(int n) {
        super(n);

        graph = new int[n][n];
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                graph[i][j] = 0;
    }

    @Override
    public void add(int a, int b) {
        super.add(a, b);
        graph[a][b] = 1;
    }

    @Override
    public void remove(int a, int b) {
        super.remove(a, b);
        assert graph[a][b] == 1 : "Given edge does not exist!";

        graph[a][b] = 0;
    }

    @Override
    public List<Integer> getNeighbors(int a) {
        super.getNeighbors(a);

        List<Integer> neighbors = new ArrayList<>();
        for(int i = 0; i < graph.length; i++)
            if(graph[a][i] == 1)
                neighbors.add(i);
        return neighbors;
    }
}