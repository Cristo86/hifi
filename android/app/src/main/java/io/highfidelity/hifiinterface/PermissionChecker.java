package io.highfidelity.hifiinterface;

import android.Manifest;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;

import com.google.vr.ndk.base.DaydreamApi;
import com.google.vr.sdk.base.Constants;

import java.io.File;

public class PermissionChecker extends Activity {

    private static final int REQUEST_PERMISSIONS = 20;
    private static final int REQUEST_PERMISSIONS_VR = 21;
    private static final int REQUEST_EXIT_VR = 30;


    private static final String TAG = "Interface";
    private static final String PREFERENCE_ASKED_PERMISSIONS = "asked_permissions";
    private static final String EXTRA_BYPASS_PERMISSION_CHECK = "bypass_check";

    private boolean mIsDaydreamStarted;
    private DaydreamApi mDaydreamApi;
    private final String[] permissions = { Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE,
                                           Manifest.permission.RECORD_AUDIO, Manifest.permission.CAMERA };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (getIntent().getBooleanExtra(EXTRA_BYPASS_PERMISSION_CHECK, false)) {
            mIsDaydreamStarted = getIntent().getCategories() != null && getIntent().getCategories().contains(Constants.DAYDREAM_CATEGORY);
            launchActivityWithPermissions();
            return;
        }
        if (mDaydreamApi == null) {
            mDaydreamApi = DaydreamApi.create(this);
        }

        if (savedInstanceState == null) {

            onRequestPermissionsResult(REQUEST_PERMISSIONS_VR, new String[0], new int[0] );

            mIsDaydreamStarted = getIntent().getCategories() != null && getIntent().getCategories().contains(Constants.DAYDREAM_CATEGORY);
            Intent myIntent = new Intent(this, BreakpadUploaderService.class);
            startService(myIntent);

            File obbDir = getObbDir();
            if (!obbDir.exists()) {
                if (obbDir.mkdirs()) {
                    Log.d(TAG, "Obb dir created");
                }
            }

            if (arePermissionsGranted(permissions)) {
                launchActivityWithPermissions();
            } else if (!hasAskedPermissionsAnytime() || shouldShowRequestPermissionRationale(permissions)) {
                if (mIsDaydreamStarted) {
                    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                    mDaydreamApi.exitFromVr(this, PermissionChecker.REQUEST_EXIT_VR, null);
                } else {
                    requestPermissions(permissions, REQUEST_PERMISSIONS);
                }
            } else {
                // permissions are not granted but the user checked "Don't ask again"
                launchActivityWithPermissions();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mDaydreamApi != null) {
            mDaydreamApi.close();
            mDaydreamApi = null;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    public void onSaveInstanceState(Bundle savedInstanceState) {
        super.onSaveInstanceState(savedInstanceState);
        savedInstanceState.putBoolean(Constants.DAYDREAM_CATEGORY, mIsDaydreamStarted);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mIsDaydreamStarted = savedInstanceState.getBoolean(Constants.DAYDREAM_CATEGORY, false);
    }

    private boolean hasAskedPermissionsAnytime() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        return preferences.getBoolean(PREFERENCE_ASKED_PERMISSIONS, false);
    }

    private void setAskedPermission() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        preferences.edit().putBoolean(PREFERENCE_ASKED_PERMISSIONS, true).commit();
    }

    private boolean arePermissionsGranted(final String[] requestedPermissions) {
        int permissionCheck = PackageManager.PERMISSION_GRANTED;
        for (String permission : requestedPermissions) {
            permissionCheck = permissionCheck + checkSelfPermission(permission);
        }

        return permissionCheck == PackageManager.PERMISSION_GRANTED;
    }

    private boolean shouldShowRequestPermissionRationale(final String[] requestedPermissions) {
        boolean shouldShowRequestPermissionRationale = false;
        for (String permission : requestedPermissions) {
            shouldShowRequestPermissionRationale = shouldShowRequestPermissionRationale || shouldShowRequestPermissionRationale(permission);
        }
        return shouldShowRequestPermissionRationale;
    }

    private void launchActivityWithPermissions() {
        Intent i = new Intent(this, InterfaceActivity.class);
        if (mIsDaydreamStarted) {
            i.addCategory(Constants.DAYDREAM_CATEGORY);
        }
        startActivity(i);
        finish();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        setAskedPermission();
        int permissionCheck = PackageManager.PERMISSION_GRANTED;
        for (int permission : grantResults) {
            permissionCheck = permissionCheck + permission;
        }
        if ((grantResults.length > 0) && permissionCheck != PackageManager.PERMISSION_GRANTED) {
            System.out.println("User has deliberately denied Permissions. Launching anyways");
        }
        switch (requestCode) {
            case REQUEST_PERMISSIONS:
                launchActivityWithPermissions();
                break;
            case REQUEST_PERMISSIONS_VR:
                // show button
                setContentView(R.layout.back_to_vr);
                break;
            default:
                break;
        }
    }

    public void onBackToVrClicked(View view) {
        view.setVisibility(View.INVISIBLE);
        Intent i = mDaydreamApi.createVrIntent(new ComponentName(this, PermissionChecker.class));
        i.addCategory(Constants.DAYDREAM_CATEGORY);
        i.putExtra(EXTRA_BYPASS_PERMISSION_CHECK, true);
        mDaydreamApi.launchInVr(i);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case REQUEST_EXIT_VR:
                    requestPermissions(permissions, REQUEST_PERMISSIONS_VR);
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        View decorView = getWindow().getDecorView();
        // Hide the status bar.
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
    }
}
