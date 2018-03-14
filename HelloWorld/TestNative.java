public class TestNative
{
    static {
        try {
            System.loadLibrary("HelloNative"); //没有lib前缀
        }
        catch(UnsatisfiedLinkError e) {
            System.out.println( "Cannot load hello library:\n " + e.toString() );
        }
    }
    public static void main(String[] args) {
        HelloNative test = new HelloNative();
        test.sayHello();
    }
}
