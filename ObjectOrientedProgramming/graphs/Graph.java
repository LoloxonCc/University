package graphs;

import java.util.List;

public class Graph {

    private int numberOfVertices;

    public Graph(int n) {
        numberOfVertices = n;
    }

    public void add(int a, int b) {
        assert a >= 0 && a < numberOfVertices : "The source vertex for this edge does not exist!";
        assert b >= 0 && b < numberOfVertices : "The destination vertex for this edge does not exist!";
    }

    public void remove(int a, int b) {
        assert a >= 0 && a < numberOfVertices : "The source vertex for this edge does not exist!";
        assert b >= 0 && b < numberOfVertices : "The destination vertex for this edge does not exist!";
    }

    public List<Integer> getNeighbors(int a) {
        assert a >= 0 && a < numberOfVertices : "Vertex does not exist!";
        return null;
    }

    public int getNumberOfVertices() {
        return numberOfVertices;
    }
}