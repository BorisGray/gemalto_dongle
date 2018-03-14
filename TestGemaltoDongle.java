public class TestGemaltoDongle
{
    static {
        try {
            System.loadLibrary("GemaltoDongle");
        }
        catch(UnsatisfiedLinkError e) {
            System.out.println( "Cannot load hello library:\n " + e.toString() );
        }
    }
    public static void main(String[] args) {
        GemaltoDongle test = new GemaltoDongle();
        test.getDongleInfo();
    }
}
