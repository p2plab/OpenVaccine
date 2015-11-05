package org.p2plab.openvaccine;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.app.Activity;
import android.content.IntentFilter;

public class SendMail extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static final String TAG = "SendMail";

    private static SendMail m_instance;

    public SendMail()
    {
        Log.v(TAG, "construtor called");
        m_instance = this;
    }

    public static void sendText(Activity context,String message) {
        Log.v(TAG, message);
        final String email = "openvaccine2015@gmail.com";
        final String[] mailto = new String[] { email };
        final String subject = "[OpenVaccine] Scan Report";

        try
        {
            Log.v(TAG, "try gmailer ... ");
            Intent gmail = new Intent(Intent.ACTION_VIEW);
            gmail.setClassName("com.google.android.gm", "com.google.android.gm.ComposeActivityGmail");
            gmail.putExtra(Intent.EXTRA_EMAIL, mailto );
            gmail.setData(Uri.parse(email));
            gmail.putExtra(Intent.EXTRA_SUBJECT, subject );
            gmail.setType("plain/text");
            gmail.putExtra(Intent.EXTRA_TEXT, message);

            Log.v(TAG, "startActivity gmailer ... ");
            context.startActivity(gmail);
        }

        catch (Exception e)
        {
            Log.v(TAG, "try email chooser ... :" + e.toString());
            Intent emailIntent = new Intent(Intent.ACTION_SEND);
            emailIntent.putExtra(Intent.EXTRA_EMAIL, mailto);
            emailIntent.putExtra(Intent.EXTRA_SUBJECT, subject );
            emailIntent.putExtra(Intent.EXTRA_TEXT, message);
            emailIntent.setType("message/rfc822");
            Log.v(TAG, "startActivity createChooser ... ");

            context.startActivity(Intent.createChooser(emailIntent, "Choose Messaging App:"));
        }

        Log.v(TAG, "mail test done!");
    }
}

