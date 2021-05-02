import java.lang.reflect.Array;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Edwin Ansari
 * 
 *         multithread consumer-producer system
 * 
 */

public class Tapis {

    public static Queue<String> buf;
    public static int cap;

    public Tapis(int cap) {
        Tapis.cap = cap;
    }

    public static synchronized int enqueue(String val) {
        if (buf.size() >= cap)
            return -1;

        buf.add(val);

        return 0;
    }

    public static synchronized String dequeue() {
        if (buf == null || buf.size() == 0)
            return null;

        return buf.poll();
    }

    class Producer implements Runnable {

        String name;
        int target;

        public Producer(String name, int target) {
            this.name = name;
            this.target = target;
        }

        @Override
        public void run() {
            int i = 0;

            while (i < target) {
                String s = name + " " + (i + 1);

                int r = enqueue(s);

                if (r < 0) {
                    try {
                        Thread.sleep(2000);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    i++;
                    System.out.printf("Produce: %s\n", s);
                }
            }

        }
    }

    class Consumer implements Runnable {

        int id;
        AtomicInteger counter;

        public Consumer(int id, AtomicInteger counter) {
            this.id = id;
            this.counter = counter;
        }

        @Override
        public void run() {
            while (counter.get() != 0) {
                String s = Tapis.dequeue();

                if (s == null) {
                    try {
                        Thread.sleep(2000);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    System.out.printf("C%d mange %s\n", id, s);
                    counter.decrementAndGet();
                }
            }

        }
    }

    public void initQueue(int n, int m, int target) {
        Thread[] producers = new Thread[n], consumers = new Thread[m];
        String[] names = { "Apple", "BlackBerry", "Snack", "Mango", "Orange" };
        if (n > 5) {
            names = new String[n];

            names = Arrays.stream(names).map(x -> "?").toArray(String[]::new);

        }

        buf = new LinkedList<String>();

        AtomicInteger counter = new AtomicInteger(n * target);

        for (int i = 0; i < n; i++) {
            producers[i] = new Thread(new Producer(names[i], target));
            producers[i].start();
        }

        for (int i = 0; i < m; i++) {
            consumers[i] = new Thread(new Consumer(i + 1, counter));
            consumers[i].start();
        }

        while (counter.get() != 0)
            ;

    }

    /*
        n = number of producers
        m = number of consumers
        target = number of objects to be produced
        cap = capacity of the queue
    */
    public static void main(String[] args) {
        int n = 3, m = 4, target = 3;
        if (args.length >= 3) {
            n = Integer.parseInt(args[0]);
            m = Integer.parseInt(args[1]);
            target = Integer.parseInt(args[2]);
        }

        Tapis t = new Tapis(5);
        t.initQueue(n, m, target);
    }
}