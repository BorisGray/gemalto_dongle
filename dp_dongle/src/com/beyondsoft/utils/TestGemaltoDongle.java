package com.beyondsoft.utils;

public class TestGemaltoDongle {

    public static void main(String[] args) {
        GemaltoDongle test = new GemaltoDongle();
        long dongle_id = test.getDongleId();
        String str = test.getDongleInfo(dongle_id);
        System.out.println(str);
    }

    static {
        try {
            System.loadLibrary("dp_dongle");
        } catch (UnsatisfiedLinkError var1) {
            System.out.println("Cannot load hello library:\n " + var1.toString());
        }

    }
}