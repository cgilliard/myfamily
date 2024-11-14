import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;

public class rbtree {
	public static void main(String [] args) {
		System.out.println("Running rbtree java test");
		Map<Long, Long> treeMap = new TreeMap<>();
		long size = 10 * 1000 * 1000;
		for(long i=0; i<size; i++) {
			long key = (long)Math.floor(Long.MAX_VALUE * Math.random());
			treeMap.put(key, i);
		}
	}
}
