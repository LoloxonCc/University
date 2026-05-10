package wyjątki;

public class WadliwyInt{
    private int value;

    public WadliwyInt(int v) throws RandomException {
        if(Math.random() > 0.9){
            throw new RandomException("nie udało się stworzyć pojemnika :(");
        }
        value = v;
    }

    public int get() throws RandomException{
        if(Math.random() > 0.9){
            throw new RandomException("nie udało się odczytać wartości :(");
        }
        return value;
    }

    public void set(int v) throws RandomException {
        if(Math.random() > 0.9){
            throw new RandomException("nie udało się zmienić wartości :(");
        }
        value = v;
    }
}
