//
//  InterfaceActivity.java
//  android/app/src/main/java
//
//  Created by Stephen Birarda on 1/26/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

package io.highfidelity.hifiinterface;

import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Point;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Vibrator;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.SlidingDrawer;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.cardboard.DisplayUtils;
import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.DaydreamApi;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.Constants;

import org.qtproject.qt5.android.QtLayout;
import org.qtproject.qt5.android.QtSurface;
import org.qtproject.qt5.android.bindings.QtActivity;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import io.highfidelity.hifiinterface.fragment.WebViewFragment;
import io.highfidelity.hifiinterface.receiver.HeadsetStateReceiver;


public class InterfaceActivity extends QtActivity implements WebViewFragment.OnWebViewInteractionListener {

    public static final String DOMAIN_URL = "url";
    public static final String EXTRA_GOTO_USERNAME = "gotousername";
    private static final String TAG = "Interface";
    public static final String OPEN_VR = "VR";

    private static final int WEB_DRAWER_RIGHT_MARGIN = 262;
    private static final int WEB_DRAWER_BOTTOM_MARGIN = 150;
    private static final int NORMAL_DPI = 160;
    private static final int MARGIN_GVR_BUTTONS_DP = 10;
    private static final int GVR_SPLIT_WIDTH = 2;
    private static final int GVR_SPLIT_HEIGHT = 55;

    private native void saveRealScreenSize(int width, int height);

    private Vibrator mVibrator;
    private HeadsetStateReceiver headsetStateReceiver;
    private DaydreamApi mDaydreamApi;

    //public static native void handleHifiURL(String hifiURLString);
    private native long nativeOnCreate(InterfaceActivity instance, AssetManager assetManager);
    private native long nativeOnCreateGvrSetup(InterfaceActivity instance, long gvrContextPtr);
    private native void nativeOnDestroy();
    private native void nativeGotoUrl(String url);
    private native void nativeGoToUser(String username);
    private native void nativeBeforeEnterBackground();
    private native void nativeEnterBackground();
    private native void nativeEnterForeground();
    private native long nativeOnExitVr();
    private native void nativeInitAfterAppLoaded();
    private native void nativeInitDaydream();

    private AssetManager assetManager;

    private boolean mIsDaydreamStarted;
    private boolean mSwitchToDaydreamPending;
    private static boolean inVrMode;

    private boolean nativeEnterBackgroundCallEnqueued = false;
    private SlidingDrawer mWebSlidingDrawer;
    private GvrApi gvrApi;
    private DisplaySynchronizer displaySynchronizer;
    private ImageView vrCloseButton;
    private ImageView vrSettingsButton;
    private View vrVerticalSplitLine;

    // Opaque native pointer to the Application C++ object.
    // This object is owned by the InterfaceActivity instance and passed to the native methods.
    //private long nativeGvrApi;
    
    public void enterVr() {
        inVrMode = true;
        // Enable VR Mode.
        AndroidCompat.setVrModeEnabled(this, true);
        runOnUiThread( () -> {
            setGvrButtonsVisibility(true);
            hideWebDrawer();
        } );
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.isLoading = true;
        mIsDaydreamStarted = getIntent().getCategories() != null && getIntent().getCategories().contains(Constants.DAYDREAM_CATEGORY);

        if (mDaydreamApi == null) {
            mDaydreamApi = DaydreamApi.create(this);
        }

        Intent intent = getIntent();
        if (intent.hasExtra(DOMAIN_URL) && !intent.getStringExtra(DOMAIN_URL).isEmpty()) {
            intent.putExtra("applicationArguments", "--url " + intent.getStringExtra(DOMAIN_URL));
        }
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (intent.getAction() == Intent.ACTION_VIEW) {
            Uri data = intent.getData();

//            if (data.getScheme().equals("hifi")) {
//                handleHifiURL(data.toString());
//            }
        }
        
        displaySynchronizer = new DisplaySynchronizer(this, DisplayUtils.getDefaultDisplay(this));
        gvrApi = new GvrApi(this, displaySynchronizer);

//        Log.d("GVR", "gvrApi.toString(): " + gvrApi.toString());

        assetManager = getResources().getAssets();

        Point size = new Point();
        getWindowManager().getDefaultDisplay().getRealSize(size);
        saveRealScreenSize(size.x, size.y);
        
        nativeOnCreateGvrSetup(this, gvrApi.getNativeGvrContext());
        nativeOnCreate(this, assetManager);

        final FrameLayout rootView = getWindow().getDecorView().findViewById(android.R.id.content);

        // This is a workaround to hide the menu bar when the virtual keyboard is shown from Qt
        rootView.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            if (getActionBar() != null && getActionBar().isShowing()) {
                getActionBar().hide();
            }
        });

        Intent splashIntent = new Intent(this, SplashActivity.class);
        splashIntent.putExtra(SplashActivity.EXTRA_DAYDREAM_START, mIsDaydreamStarted);
        startActivity(splashIntent);
        mVibrator = (Vibrator) this.getSystemService(VIBRATOR_SERVICE);
        headsetStateReceiver = new HeadsetStateReceiver();
        addGvrButtons(rootView);
    }

    private void addGvrButtons(FrameLayout mainLayout) {
        float density = Resources.getSystem().getDisplayMetrics().density;
        vrCloseButton = new ImageView(this);
        vrCloseButton.setImageResource(R.drawable.ic_gvr_close);
        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.TOP | Gravity.LEFT;
        lp.topMargin = ((int)(density * MARGIN_GVR_BUTTONS_DP));
        lp.leftMargin = ((int)(density * MARGIN_GVR_BUTTONS_DP));
        vrCloseButton.setLayoutParams(lp);
        vrCloseButton.setOnClickListener(view -> onGvrCloseButtonClick());
        vrCloseButton.setVisibility(View.GONE);
        mainLayout.addView(vrCloseButton);
        vrSettingsButton = new ImageView(this);
        vrSettingsButton.setImageResource(R.drawable.ic_gvr_settings);
        lp = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.TOP | Gravity.RIGHT;
        lp.topMargin = ((int)(density * MARGIN_GVR_BUTTONS_DP));
        lp.rightMargin = ((int)(density * MARGIN_GVR_BUTTONS_DP));
        vrSettingsButton.setLayoutParams(lp);
        vrSettingsButton.setOnClickListener(view -> onGvrSettingsButtonClick());
        vrSettingsButton.setVisibility(View.GONE);
        mainLayout.addView(vrSettingsButton);

        vrVerticalSplitLine = new View(this);
        vrVerticalSplitLine.setBackgroundColor(Color.parseColor("#FF515151"));
        lp = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        lp.width = (int) (density * GVR_SPLIT_WIDTH);
        lp.height = (int) (density * GVR_SPLIT_HEIGHT);
        vrVerticalSplitLine.setLayoutParams(lp);
        vrVerticalSplitLine.setVisibility(View.GONE);
        mainLayout.addView(vrVerticalSplitLine);
    }
    private void onGvrCloseButtonClick() {
        assert inVrMode;
        inVrMode = false;
        setGvrButtonsVisibility(false);
        nativeOnExitVr();
    }
    private void onGvrSettingsButtonClick() {
        openAndroidActivity(MainActivity.FRAGMENT_SETTINGS, true);
    }

    @Override
    protected void onPause() {
        if (super.isLoading) {
            nativeEnterBackgroundCallEnqueued = true;
        } else {
            nativeEnterBackground();
        }
        unregisterReceiver(headsetStateReceiver);
        gvrApi.pauseTracking();
        displaySynchronizer.onPause();
        super.onPause();
    }

    @Override
    protected void onStart() {
        super.onStart();
        nativeEnterBackgroundCallEnqueued = false;
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeEnterForeground();
        surfacesWorkaround();
        keepInterfaceRunning = false;
        registerReceiver(headsetStateReceiver, new IntentFilter(Intent.ACTION_HEADSET_PLUG));
        gvrApi.resumeTracking();
        displaySynchronizer.onResume();
        if (mSwitchToDaydreamPending) {
            runOnUiThread(() -> nativeInitDaydream());
            mSwitchToDaydreamPending = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        nativeOnDestroy();
        if (mDaydreamApi != null) {
            mDaydreamApi.close();
            mDaydreamApi = null;
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Checks the orientation of the screen
        if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT){
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
        surfacesWorkaround();
    }

    private void surfacesWorkaround() {
        FrameLayout fl = findViewById(android.R.id.content);
        if (fl.getChildCount() > 0 && fl.getChildAt(0) instanceof QtLayout) {
            QtLayout qtLayout = (QtLayout) fl.getChildAt(0);
            List<QtSurface> surfaces = new ArrayList<>();
            for (int i = 0; i < qtLayout.getChildCount(); i++) {
                Object ch = qtLayout.getChildAt(i);
                if (ch instanceof QtSurface) {
                    surfaces.add((QtSurface) ch);
                }
            }
            if (surfaces.size() > 1) {
                QtSurface s1 = surfaces.get(0);
                QtSurface s2 = surfaces.get(1);
                Integer subLayer1 = 0;
                Integer subLayer2 = 0;
                try {
                    String field;
                    if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        field = "mSubLayer";
                    } else {
                        field = "mWindowType";
                    }
                    Field f = s1.getClass().getSuperclass().getDeclaredField(field);
                    f.setAccessible(true);
                    subLayer1 = (Integer) f.get(s1);
                    subLayer2 = (Integer) f.get(s2);
                    if (subLayer1 < subLayer2) {
                        s1.setVisibility(View.VISIBLE);
                        s2.setVisibility(View.INVISIBLE);
                    } else {
                        s1.setVisibility(View.INVISIBLE);
                        s2.setVisibility(View.VISIBLE);
                    }
                } catch (ReflectiveOperationException e) {
                    Log.e(TAG, "Workaround failed");
                }
            }
        } else {
            Log.w(TAG, "Could not apply surfaces workaround");
        }
    }

    public void openUrlInAndroidWebView(String urlString) {
        Log.d("openUrl", "Received in open " + urlString);
        Intent openUrlIntent = new Intent(this, WebViewActivity.class);
        openUrlIntent.putExtra(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL, urlString);
        startActivity(openUrlIntent);
    }

    /**
     * Called when view focus is changed
     */
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            final int uiOptions = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

            getWindow().getDecorView().setSystemUiVisibility(uiOptions);
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        if (intent.hasExtra(DOMAIN_URL)) {
            hideWebDrawer();
            nativeGotoUrl(intent.getStringExtra(DOMAIN_URL));
        } else if (intent.hasExtra(EXTRA_GOTO_USERNAME)) {
            hideWebDrawer();
            nativeGoToUser(intent.getStringExtra(EXTRA_GOTO_USERNAME));
        } else if (intent.hasCategory(Constants.DAYDREAM_CATEGORY)) {
            mSwitchToDaydreamPending = true;
        }
    }

    private void hideWebDrawer() {
        if (mWebSlidingDrawer != null) {
            mWebSlidingDrawer.setVisibility(View.GONE);
        }
    }

    public void showWebDrawer() {
        if (mWebSlidingDrawer == null) {
            FrameLayout mainLayout = findViewById(android.R.id.content);
            LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            QtLayout qtLayout = (QtLayout) mainLayout.getChildAt(0);
            mWebSlidingDrawer = (SlidingDrawer) inflater.inflate(R.layout.web_drawer, mainLayout, false);

            QtLayout.LayoutParams layoutParams = new QtLayout.LayoutParams(mWebSlidingDrawer.getLayoutParams());
            mWebSlidingDrawer.setOnDrawerCloseListener(() -> {
                WebViewFragment webViewFragment = (WebViewFragment) getFragmentManager().findFragmentByTag("webViewFragment");
                webViewFragment.close();
            });

            Point size = new Point();
            getWindowManager().getDefaultDisplay().getRealSize(size);
            int widthPx = Math.max(size.x, size.y);
            int heightPx = Math.min(size.x, size.y);

            layoutParams.x = (int) (widthPx - WEB_DRAWER_RIGHT_MARGIN * getResources().getDisplayMetrics().xdpi / NORMAL_DPI);
            layoutParams.y = (int) (heightPx - WEB_DRAWER_BOTTOM_MARGIN * getResources().getDisplayMetrics().ydpi / NORMAL_DPI);

            layoutParams.resolveLayoutDirection(View.LAYOUT_DIRECTION_RTL);
            qtLayout.addView(mWebSlidingDrawer, layoutParams);
        }
        mWebSlidingDrawer.setVisibility(View.VISIBLE);
    }

    public void openAndroidActivity(String activityName, boolean backToScene) {
        openAndroidActivity(activityName, backToScene, null);
    }

    public void openAndroidActivity(String activityName, boolean backToScene, HashMap args) {
        switch (activityName) {
            case MainActivity.FRAGMENT_HOME:
            case MainActivity.FRAGMENT_PRIVACY_POLICY:
                nativeBeforeEnterBackground();
                Intent intent = new Intent(this, MainActivity.class);
                intent.putExtra(MainActivity.EXTRA_FRAGMENT, activityName);
                intent.putExtra(MainActivity.EXTRA_BACK_TO_SCENE, backToScene);
                startActivity(intent);
                break;
            case MainActivity.FRAGMENT_SETTINGS:
                nativeBeforeEnterBackground();
                try {
                    Intent settingsIntent = Intent.makeMainActivity(new ComponentName("com.google.vr.vrcore", "com.google.vr.vrcore.settings.VrSettingsActivity"));
                    startActivity(settingsIntent);
                } catch (ActivityNotFoundException e) {
                    Intent intentMain = new Intent(this, MainActivity.class);
                    intentMain.putExtra(MainActivity.EXTRA_FRAGMENT, activityName);
                    intentMain.putExtra(MainActivity.EXTRA_BACK_TO_SCENE, backToScene);
                    startActivity(intentMain);
                }
                break;
            case MainActivity.FRAGMENT_LOGIN:
                nativeBeforeEnterBackground();
                Intent loginIntent = new Intent(this, LoginMenuActivity.class);
                loginIntent.putExtra(LoginMenuActivity.EXTRA_BACK_TO_SCENE, backToScene);
                loginIntent.putExtra(LoginMenuActivity.EXTRA_BACK_ON_SKIP, true);
                if (args != null && args.containsKey(DOMAIN_URL)) {
                    loginIntent.putExtra(LoginMenuActivity.EXTRA_DOMAIN_URL, (String) args.get(DOMAIN_URL));
                }
                startActivity(loginIntent);
                break;
            case OPEN_VR:
                Intent i = mDaydreamApi.createVrIntent(new ComponentName(this, PermissionChecker.class));
                i.addCategory(Constants.DAYDREAM_CATEGORY);
                i.putExtra(PermissionChecker.EXTRA_BYPASS_PERMISSION_CHECK, true);
                i.putExtra(PermissionChecker.EXTRA_SINGLE_INTERFACE_ACTIVITY, true);
                mDaydreamApi.launchInVr(i);
                break;
            case "WebView":
                runOnUiThread(() -> {
                    showWebDrawer();
                    if (!mWebSlidingDrawer.isOpened()) {
                        mWebSlidingDrawer.animateOpen();
                    }
                    if (args != null && args.containsKey(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL)) {
                        WebViewFragment webViewFragment = (WebViewFragment) getFragmentManager().findFragmentByTag("webViewFragment");
                        webViewFragment.loadUrl((String) args.get(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL), true);
                        webViewFragment.setToolbarVisible(true);
                        webViewFragment.setCloseAction(() -> {
                            if (mWebSlidingDrawer.isOpened()) {
                                mWebSlidingDrawer.animateClose();
                            }
                            hideWebDrawer();
                        });
                    }
                });
                break;
            default: {
                Log.w(TAG, "Could not open activity by name " + activityName);
                break;
            }
        }
    }

    public void onAppLoadedComplete() {
        super.isLoading = false;
        if (nativeEnterBackgroundCallEnqueued) {
            nativeEnterBackground();
        }
        runOnUiThread(() -> {
            nativeInitAfterAppLoaded();
            if (mIsDaydreamStarted) {
                mSwitchToDaydreamPending = true;
            }
        });
    }

    public void performHapticFeedback(int duration) {
        if (duration > 0) {
            mVibrator.vibrate(duration);
        }
    }

    @Override
    public void onBackPressed() {
        openAndroidActivity("Home", false);
    }

    @Override
    public void processURL(String url) { }

    @Override
    public void onWebLoaded(String url, WebViewFragment.SafenessLevel safenessLevel) { }

    @Override
    public void onTitleReceived(String title) { }

    @Override
    public void onExpand() {
        keepInterfaceRunning = true;
    }

    @Override
    public void onOAuthAuthorizeCallback(Uri uri) { }

    public void setGvrButtonsVisibility(boolean visible) {
        if (visible) {
            vrCloseButton.setVisibility(View.VISIBLE);
            vrSettingsButton.setVisibility(View.VISIBLE);
            vrVerticalSplitLine.setVisibility(View.VISIBLE);
        } else {
            vrCloseButton.setVisibility(View.GONE);
            vrSettingsButton.setVisibility(View.GONE);
            vrVerticalSplitLine.setVisibility(View.GONE);
        }
    }
}
