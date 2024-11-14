import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;

public class rbtree {
	public static void main(String [] args) {
		System.out.println("Running rbtree java test");
		Map<Long, Long> treeMap = new TreeMap<>();
		int size = 10 * 1000;
		int count = 1000;

		long keys[] = new long[size];
		for(int j=0; j<size; j++)
			keys[j] = (long)Math.floor(Long.MAX_VALUE * Math.random());

		for(int i=0; i<count; i++) {
			for(int j=0; j<size; j++) {
				treeMap.put(keys[j], (long)j);
			}
			for(int j=0; j<size; j++) {
				treeMap.remove(keys[j]);
			}
		}
	}
}
