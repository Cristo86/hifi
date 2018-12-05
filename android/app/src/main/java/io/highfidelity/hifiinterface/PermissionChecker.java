package io.highfidelity.hifiinterface;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
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
    private boolean mIsDaydreamStarted;
    private final String[] permissions = { Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE,
                                           Manifest.permission.RECORD_AUDIO, Manifest.permission.CAMERA };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (savedInstanceState == null) {
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
            } else if (mIsDaydreamStarted) {
                DaydreamApi daydreamApi;
                daydreamApi = DaydreamApi.create(this);
                PermissionChecker permissionCheckerActivity = new PermissionChecker();
                setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                daydreamApi.exitFromVr(this, PermissionChecker.REQUEST_EXIT_VR, null);
                daydreamApi.close();
            } else {
                requestPermissions(permissions, REQUEST_PERMISSIONS);
            }
        }
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

    private boolean arePermissionsGranted(final String[] requestedPermissions) {
        int permissionCheck = PackageManager.PERMISSION_GRANTED;
        boolean shouldShowRequestPermissionRationale = false;
        for (String permission : requestedPermissions) {
            permissionCheck = permissionCheck + checkSelfPermission(permission);
            shouldShowRequestPermissionRationale = shouldShowRequestPermissionRationale || shouldShowRequestPermissionRationale(permission);
        }

        return permissionCheck == PackageManager.PERMISSION_GRANTED || !shouldShowRequestPermissionRationale;
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
        launchActivityWithPermissions();
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
