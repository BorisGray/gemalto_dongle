/*
import java.security.*;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;
*/

public class GemaltoDongle
 {
	public native String getDongleInfo();
/*	public native String login_scope();
	public native String logout();
	public native String get_info();
	public native String get_session_info();

	static {
        System.loadLibrary("DPGemaltoDongle");
    }
*/
/*	public static void main(String[] args) {
		System.out.println("**************dpDongleNative*************");
		GemaltoDongle test = new GemaltoDongle();
		test.getDongleInfo();
		System.out.println("getDongleInfo!");
*/
	/*	String md5 = test.getDongleMD5("12345678901234567890123456789012");
		System.out.println("java md5 len="+md5.length());
		System.out.println("java md5="+md5);

		String data = test.getDongleData();
		System.out.println("java data len="+data.length());
		System.out.println("java data="+data);

		System.out.println("test dongle end");

		String str = "b3d230c7a55c530a0fec3694459e267f12345678901234567890123456789012";
		byte[] key = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
		byte[] result = desCrypto(str.getBytes(), key);

		printHexString(result);

		String startDate = test.getStartDate();
		System.out.println("java startDate="+startDate);

		String endDate = test.getEndDate();
		System.out.println("java endDate="+endDate);
	}
  */
}
