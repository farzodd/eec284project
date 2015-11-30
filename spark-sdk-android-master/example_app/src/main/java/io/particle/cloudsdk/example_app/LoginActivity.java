package io.particle.cloudsdk.example_app;

import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import java.io.IOException;
import java.util.List;

import io.particle.android.sdk.cloud.ParticleCloud;
import io.particle.android.sdk.cloud.ParticleCloudException;
import io.particle.android.sdk.cloud.ParticleDevice;
import io.particle.android.sdk.utils.Async;
import io.particle.android.sdk.utils.Py;
import io.particle.android.sdk.utils.Toaster;

public class LoginActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        final String ARG_DEVICEID = "ARG_DEVICEID";



        findViewById(R.id.login_button).setOnClickListener(
                new View.OnClickListener() {

                    @Override
                    public void onClick(final View v) {
                        final String email = ((EditText) findViewById(R.id.email)).getText().toString();
                        final String password = ((EditText) findViewById(R.id.password)).getText().toString();
                        //ParticleDevice particleDevice = ParticleCloud.getDevice(getIntent().getStringExtra(ARG_DEVICEID));

                        // Don't:
                        AsyncTask task = new AsyncTask() {
                            @Override
                            protected Object doInBackground(Object[] params) {
                                try {
                                    ParticleCloud.get(LoginActivity.this).logIn(email, password);

                                } catch (final ParticleCloudException e) {
                                    Runnable mainThread = new Runnable() {
                                        @Override
                                        public void run() {
                                            Toaster.l(LoginActivity.this, e.getBestMessage());
                                            e.printStackTrace();
                                            Log.d("info", e.getBestMessage());
//                                            Log.d("info", e.getCause().toString());
                                        }
                                    };
                                    runOnUiThread(mainThread);

                                }

                                return null;
                            }

                        };
//                        task.execute();

                        //-------

                        // DO!

                        Async.executeAsync(ParticleCloud.get(v.getContext()), new Async.ApiWork<ParticleCloud, Object>() {

                            private ParticleDevice mDevice;


                            @Override
                            public Object callApi(ParticleCloud sparkCloud) throws ParticleCloudException, IOException {
                                sparkCloud.logIn(email, password);
                                sparkCloud.getDevices();
                                mDevice = sparkCloud.getDevice("50ff71065067545632430687");
                                int returnVal;


                                try {
                                    returnVal = mDevice.callFunction("digitalwrite", Py.list("D7","1"));
                                    Log.d("BANANA", "analogvalue: " + returnVal);
                                } catch (ParticleDevice.FunctionDoesNotExistException e) {
                                    Toaster.s(LoginActivity.this, "Function doesn't exist.");
                                    returnVal = 500;
                                }
                                /*
                                try {
                                    String strVariable = mDevice.getStringVariable("stringvalue");
                                    Log.d("BANANA", "stringvalue: " + strVariable);
                                } catch (ParticleDevice.VariableDoesNotExistException e) {
                                    Toaster.s(LoginActivity.this, "Error reading variable1");
                                }

                                try {
                                    double dVariable = mDevice.getDoubleVariable("doublevalue");
                                    Log.d("BANANA", "doublevalue: " + dVariable);
                                } catch (ParticleDevice.VariableDoesNotExistException e) {
                                    Toaster.s(LoginActivity.this, "Error reading variable2");
                                }

                                try {
                                    int intVariable = mDevice.getIntVariable("analogvalue");
                                    Log.d("BANANA", "int analogvalue: " + intVariable);
                                } catch (ParticleDevice.VariableDoesNotExistException e) {
                                    Toaster.s(LoginActivity.this, "Error reading variable3");
                                }
                                */


                                return 1;

                            }

                            @Override
                            public void onSuccess(Object value) {
                                Toaster.l(LoginActivity.this, "Logged in");
                                Intent intent = ValueActivity.buildIntent(LoginActivity.this, 123, mDevice.getID());
                                startActivity(intent);
                            }

                            @Override
                            public void onFailure(ParticleCloudException e) {
                                Toaster.l(LoginActivity.this, e.getBestMessage());
                                e.printStackTrace();
                                Log.d("info", e.getBestMessage());
                            }
                        });


                    }
                }

        );
    }

}
