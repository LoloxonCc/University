package graphs;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Scanner;

public class Main {
    public static void traverseGraph(Graph graph, Queue queue, int startVertex) {
        boolean[] visited = new boolean[graph.getNumberOfVertices()];

        queue.add(startVertex);
        visited[startVertex] = true;

        while(!queue.isEmpty()) {
            int v = queue.retrieve();
            System.out.print(v + " ");

            List<Integer> neighborList = new ArrayList<>(graph.getNeighbors(v));

            // to print IDs in ascending order
            if (queue instanceof LIFO) {
                neighborList.sort(Collections.reverseOrder());
            }
            else {
                Collections.sort(neighborList);
            }

            for (int neighbor : neighborList) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.add(neighbor);
                }
            }
        }

        System.out.println();
    }

    public static void main(String[] args) {
        Scanner scn = new Scanner(System.in);
        int n, m;
        n = scn.nextInt();
        m = scn.nextInt();
        AdjacencyMatrixGraph amg = new AdjacencyMatrixGraph(n);
        AdjacencyListGraph alg = new AdjacencyListGraph(n);

        for(int i = 0; i < m; i++) {
            int a, b;
            a = scn.nextInt();
            b = scn.nextInt();
            amg.add(a, b);
            alg.add(a, b);
        }

        System.out.print("Adjacency Matrix Graph - DFS: ");
        traverseGraph(amg, new LIFO(), 0);
        System.out.print("Adjacency Matrix Graph - BFS: ");
        traverseGraph(amg, new FIFO(), 0);

        System.out.print("Adjacency List Graph - DFS: ");
        traverseGraph(alg, new LIFO(), 0);
        System.out.print("Adjacency List Graph - BFS: ");
        traverseGraph(alg, new FIFO(), 0);

        scn.close();
    }
}
