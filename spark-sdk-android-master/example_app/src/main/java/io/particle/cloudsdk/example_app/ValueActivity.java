package io.particle.cloudsdk.example_app;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
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
    private EditText rpmVal;
    private EditText numStepsVal;
    private char[] intermediateMsg = new char[40];
    private String commandMsg;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_value);
        tv = (TextView) findViewById(R.id.value);
        tv.setText(String.valueOf(getIntent().getIntExtra(ARG_VALUE, 0)));
        rpmVal = (EditText) findViewById(R.id.rpm);
        numStepsVal = (EditText) findViewById(R.id.numSteps);


        findViewById(R.id.rotate_cw).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //...
                // Do network work on background thread
                Async.executeAsync(ParticleCloud.get(ValueActivity.this), new Async.ApiWork<ParticleCloud, Object>() {
                    @Override
                    public Object callApi(ParticleCloud ParticleCloud) throws ParticleCloudException, IOException {
                        ParticleDevice device = ParticleCloud.getDevice(getIntent().getStringExtra(ARG_DEVICEID));
                        Object variable;
                        int curIndex, splitIndex;
                        int speed = Integer.parseInt(rpmVal.getText().toString());
                        int steps = Integer.parseInt(numStepsVal.getText().toString());

                        intermediateMsg[0] = 'c';
                        intermediateMsg[1] = 'w';

                        for (curIndex = 2, splitIndex = 0; curIndex < 18 && splitIndex < ((Integer.toString(speed)).toCharArray()).length; curIndex++, splitIndex++){
                            intermediateMsg[curIndex] = ((Integer.toString(speed)).toCharArray())[splitIndex];
                        }
                        intermediateMsg[curIndex] = 'f';

                        for (curIndex = curIndex+1,splitIndex = 0; curIndex < 38 && splitIndex < ((Integer.toString(steps)).toCharArray()).length; curIndex++, splitIndex++){
                            intermediateMsg[curIndex] = ((Integer.toString(steps)).toCharArray())[splitIndex];
                        }
                        intermediateMsg[curIndex] = 'g';

                        commandMsg = new String(intermediateMsg);
                        Log.d("messagecheck",commandMsg);
                        try {
                            device.callFunction("movestepper", Py.list(commandMsg));
                            variable = "CW Movement Complete";
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

        findViewById(R.id.rotate_ccw).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //...
                // Do network work on background thread
                Async.executeAsync(ParticleCloud.get(ValueActivity.this), new Async.ApiWork<ParticleCloud, Object>() {
                    @Override
                    public Object callApi(ParticleCloud ParticleCloud) throws ParticleCloudException, IOException {
                        ParticleDevice device = ParticleCloud.getDevice(getIntent().getStringExtra(ARG_DEVICEID));
                        Object variable;
                        int curIndex, splitIndex;
                        int speed = Integer.parseInt(rpmVal.getText().toString());
                        int steps = Integer.parseInt(numStepsVal.getText().toString());

                        intermediateMsg[0] = 'c';
                        intermediateMsg[1] = 'c';

                        for (curIndex = 2, splitIndex = 0; curIndex < 18 && splitIndex < ((Integer.toString(speed)).toCharArray()).length; curIndex++, splitIndex++){
                            intermediateMsg[curIndex] = ((Integer.toString(speed)).toCharArray())[splitIndex];
                        }
                        intermediateMsg[curIndex] = 'f';

                        for (curIndex = curIndex+1,splitIndex = 0; curIndex < 38 && splitIndex < ((Integer.toString(steps)).toCharArray()).length; curIndex++, splitIndex++){
                            intermediateMsg[curIndex] = ((Integer.toString(steps)).toCharArray())[splitIndex];
                        }
                        intermediateMsg[curIndex] = 'g';

                        commandMsg = new String(intermediateMsg);
                        //Log.d("messagecheck",commandMsg);
                        try {
                            device.callFunction("movestepper", Py.list(commandMsg));
                            variable = "CCW Movement Complete";
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
