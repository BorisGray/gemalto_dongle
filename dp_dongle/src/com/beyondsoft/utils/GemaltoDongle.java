package com.beyondsoft.utils;
public class GemaltoDongle
 {
    public native long getDongleId();
    public native String getDongleInfo(long dongle_id);

//     static{
//         System.loadLibrary("dp_dongle");
//     }
//
//     public static void main(String[] args) throws InterruptedException {
//         GemaltoDongle test = new GemaltoDongle();
//         long dongle_id =  test.getDongleId();
//         Thread.sleep(5000);
//         test.getDongleInfo(dongle_id);
//     }

//     public static class TestGemaltoDongle {
//
//         static {
//               try {
//                   System.loadLibrary("dp_dongle");
//               }
//               catch(UnsatisfiedLinkError e) {
//                   System.out.println( "Cannot load hello library:\n " + e.toString() );
//               }
//           }
//           public static void main(String[] args) {
//
//               GemaltoDongle test = new GemaltoDongle();
//
//                 long dongle_id =  test.getDongleId();
//
//               test.getDongleInfo(dongle_id);
//           }
//
//     }
 }
