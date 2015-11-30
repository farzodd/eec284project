package io.particle.cloudsdk.example_app;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.IOException;

import io.particle.android.sdk.cloud.ParticleCloud;
import io.particle.android.sdk.cloud.ParticleCloudException;
import io.particle.android.sdk.cloud.ParticleDevice;
import io.particle.android.sdk.utils.Async;
import io.particle.android.sdk.utils.Py;
import io.particle.android.sdk.utils.Toaster;

public class ValueActivity extends AppCompatActivity {

    private static final String ARG_VALUE = "ARG_VALUE";
    private static final String ARG_DEVICEID = "ARG_DEVICEID";

    private TextView tv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_value);
        tv = (TextView) findViewById(R.id.value);
        tv.setText(String.valueOf(getIntent().getIntExtra(ARG_VALUE, 0)));

        findViewById(R.id.refresh_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //...
                // Do network work on background thread
                Async.executeAsync(ParticleCloud.get(ValueActivity.this), new Async.ApiWork<ParticleCloud, Object>() {
                    @Override
                    public Object callApi(ParticleCloud ParticleCloud) throws ParticleCloudException, IOException {
                        ParticleDevice device = ParticleCloud.getDevice(getIntent().getStringExtra(ARG_DEVICEID));
                        Object variable;
                        int writeResponse;
                        try {
                            for (String funcName : device.getFunctions()) {
                                Log.i("SOME_TAG", "Device has function: " + funcName);
                            }
                            variable = device.getVariable("analogvalue");

                        } catch (ParticleDevice.VariableDoesNotExistException e) {
                            //Toaster.l(ValueActivity.this, e.getMessage());
                            variable = -1;
                        }

                        try {
                            Boolean isHigh;
                            int currentState = device.callFunction("digitalread", Py.list("D7"));
                            Log.d("StateOfPin","Current value: " + Integer.toString(currentState));
                            if (currentState != 0){ isHigh = Boolean.TRUE;}
                            else {isHigh = Boolean.FALSE;}

                            if (isHigh){
                                writeResponse = device.callFunction("digitalwrite", Py.list("D7", "LOW"));
                                variable = "Set to LOW";
                            }
                            else{
                                writeResponse = device.callFunction("digitalwrite", Py.list("D7", "HIGH"));
                                variable = "Set to HIGH";
                            }
                            //String tinkerResp = Integer.toString(writeResponse);
                            //Log.d("BANANA", "analogvalue: " + obj);
                            //Toaster.s(ValueActivity.this, tinkerResp);
                            //variable = "Set to HIGH";
                        } catch (ParticleDevice.FunctionDoesNotExistException e) {
                            Toaster.s(ValueActivity.this, "Function doesn't exist.");
                            variable = "Func not found";
                        }

                        return variable;
                    }

                    @Override
                    public void onSuccess(Object i) { // this goes on the main thread
                        tv.setText(i.toString());
                    }

                    @Override
                    public void onFailure(ParticleCloudException e) {
                        e.printStackTrace();
                    }
                });
            }
        });
    }

    public static Intent buildIntent(Context ctx, Integer value, String deviceid) {
        Intent intent = new Intent(ctx, ValueActivity.class);
        intent.putExtra(ARG_VALUE, value);
        intent.putExtra(ARG_DEVICEID, deviceid);

        return intent;
    }


}
