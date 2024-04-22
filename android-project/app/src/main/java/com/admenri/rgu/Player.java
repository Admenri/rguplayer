package com.admenri.rgu;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;

import org.libsdl.app.SDLActivity;

public class Player extends SDLActivity {
    private static final String TAG = "RGUActivity";
    private static String GAME_PATH;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // External storage dirs
        GAME_PATH = getContext().getExternalFilesDir("").toString();
        Log.i(TAG, GAME_PATH);
    }
}
