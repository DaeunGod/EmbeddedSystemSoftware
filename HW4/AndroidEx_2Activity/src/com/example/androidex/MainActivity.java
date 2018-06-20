package com.example.androidex;

import com.example.androidex.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;



public class MainActivity extends Activity{
	private static final String TAG = "tag";
	//public native int add(int x, int y);
	//public native void testString(String str);
	
	LinearLayout linear;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		//System.loadLibrary("ndk-exam");
		
		setContentView(R.layout.activity_main);
		linear = (LinearLayout)findViewById(R.id.container);
		
		Button btn=(Button)findViewById(R.id.newactivity);
		OnClickListener listener=new OnClickListener(){
			public void onClick(View v){
				Intent intent=new Intent(MainActivity.this, MainActivity2.class);
				startActivity(intent);
			}
		};
		btn.setOnClickListener(listener);
	}

}
