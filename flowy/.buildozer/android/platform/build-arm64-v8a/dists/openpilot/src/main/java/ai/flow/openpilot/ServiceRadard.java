package ai.flow.openpilot;

import java.io.File;

import android.os.Build;
import android.content.Intent;
import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import org.kivy.android.PythonService;
import org.kivy.android.PythonUtil;

public class ServiceRadard extends PythonService {

    private static final String TAG = "PythonService";

    

    @Override
    protected int getServiceId() {
        return 8;
    }

    public static void prepare(Context ctx) {
        String appRoot = PythonUtil.getAppRoot(ctx);
        Log.v(TAG, "Ready to unpack");
        File app_root_file = new File(appRoot);
        PythonUtil.unpackAsset(ctx, "private", app_root_file, true);
        PythonUtil.unpackPyBundle(ctx, ctx.getApplicationInfo().nativeLibraryDir + "/" + "libpybundle", app_root_file, false);
    }
	
    static private void _start(Context ctx, String smallIconName,
			     String contentTitle,
			     String contentText,
			     String pythonServiceArgument) {
        Intent intent = getDefaultIntent(ctx, smallIconName, contentTitle,
                                         contentText, pythonServiceArgument);
        //foreground: False
        
        ctx.startService(intent);
        
    }

    public static void start(Context ctx, String pythonServiceArgument) {
	_start(ctx,  "", "Openpilot", "Radard", pythonServiceArgument);
    }
    
    static public void start(Context ctx, String smallIconName,
			     String contentTitle,
			     String contentText,
			     String pythonServiceArgument) {
	_start(ctx, smallIconName, contentTitle, contentText, pythonServiceArgument);
    }

    static public Intent getDefaultIntent(Context ctx, String smallIconName,
					  String contentTitle,
					  String contentText,
					  String pythonServiceArgument) {
        String appRoot = PythonUtil.getAppRoot(ctx);
        Intent intent = new Intent(ctx, ServiceRadard.class);
        intent.putExtra("androidPrivate", appRoot);
        intent.putExtra("androidArgument", appRoot);
        intent.putExtra("serviceEntrypoint", "radard.py");
        intent.putExtra("serviceTitle", "Radard");
        intent.putExtra("pythonName", "Radard");
        intent.putExtra("serviceStartAsForeground", "false");
        intent.putExtra("pythonHome", appRoot);
        intent.putExtra("androidUnpack", appRoot);
        intent.putExtra("pythonPath", appRoot + ":" + appRoot + "/lib");
        intent.putExtra("pythonServiceArgument", pythonServiceArgument);
        intent.putExtra("smallIconName", smallIconName);
        intent.putExtra("contentTitle", contentTitle);
        intent.putExtra("contentText", contentText);
        return intent;
    }

    @Override
    protected Intent getThisDefaultIntent(Context ctx, String pythonServiceArgument) {
        return ServiceRadard.getDefaultIntent(ctx,  "", "", "",
                                                             pythonServiceArgument);
    }



    static public void stop(Context ctx) {
        Intent intent = new Intent(ctx, ServiceRadard.class);
        ctx.stopService(intent);
    }

}