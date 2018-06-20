package com.example.androidex;

import com.example.androidex.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;



public class MainActivity2 extends Activity implements View.OnClickListener{
	private static final String TAG = "tag";
	LinearLayout linear;
	EditText data;
	Button btn;
	OnClickListener ltn;
	static int row = 0;
	static int col = 0;
	int widthPixel;
	int heightPixel;
	MainActivity2 mainActiviy;;
	DisplayMetrics dm;
	
	int[] numberList;
	
	FpgaControl fpgaControl;
	
	public static boolean isStringInt(String s){
		try{
			Integer.parseInt(s);
			return true;
		} catch (NumberFormatException e){
			return false;
		}
	}
	
		@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main2);
		
		mainActiviy = this;
		dm = getApplicationContext().getResources().getDisplayMetrics();
		fpgaControl = new FpgaControl();
		
		
		linear = (LinearLayout)findViewById(R.id.container);
		data=(EditText)findViewById(R.id.editText1);
		Button btn=(Button)findViewById(R.id.button1);
		
		widthPixel = dm.widthPixels;
		heightPixel = dm.heightPixels - 150;
		
		ltn=new OnClickListener(){
			public void onClick(View v){
				
				
				String temp=data.getText().toString();
				String[] parseStr = temp.split(" ");
				if( parseStr.length != 2 )
					return ;
				if( isStringInt(parseStr[0]) == false || 
					isStringInt(parseStr[1]) == false )
					return ;
				
				clearButtons();

				row = Integer.parseInt(parseStr[0]);
				col = Integer.parseInt(parseStr[1]);
				if( row < 2 || col < 1 || row > 4 || col > 4){
					row = col = 0;
					return ;
				}
				numberList = makeRandomNumber(row, col);
				//linear.setOrientation(LinearLayout.VERTICAL);
				makeButtons();
				
			}
		};
		
		btn.setOnClickListener(ltn);
	}
	
	
	public int[] makeRandomNumber(int row, int col){
		int[] temp = new int[row*col];
		int zeroIndex = row*col-1;
		for(int i=0; i<temp.length-1; i++){
			temp[i] = i+1;
		}
		for(int i=0; i<1000; i++){
		
			int dir = (int)(Math.random()*4);
			//Log.v(TAG, String.valueOf(dir));
			int _row = zeroIndex/col;
			int _col = zeroIndex%col;
			int changeIndex = zeroIndex;
			
			if( dir == 0 ){
				if( _row-1 >= 0 ){
					changeIndex = (_row-1)*col+_col;
				}
			}
			else if( dir == 1 ){	
				if( _row+1 < row ){
					changeIndex = (_row+1)*col+_col;
				}
			}
			else if( dir == 2 ){	
				if( _col-1 >= 0 ){
					changeIndex = (_row)*col+_col-1;
				}
			}
			else if( dir == 3){		
				if( _col+1 < col ){
					changeIndex = (_row)*col+_col+1;
				}
			}
			
			int t = temp[changeIndex]; 
			temp[changeIndex] = 0;
			temp[zeroIndex] = t;
				
			zeroIndex = changeIndex;
			
		}	
		
		return temp;
	}
	
	public void onClick(View V){
		Button btnTag = (Button)V;
		int buttonNumber = Integer.parseInt((String)(btnTag.getText()));
		
		int index = 0;
		
		for(int i=0; i<numberList.length; i++){
			if( numberList[i] == buttonNumber ){
				index = i;
				break;
			}
		}
		
		int _row = index/col;
		int _col = index%col;
		
		if( _row-1 >= 0 ){
			if( numberList[(_row-1)*col + _col] == 0 ){
				numberList[(_row-1)*col + _col] = buttonNumber;
				numberList[_row*col + _col] = 0;
			}
		}
		if( _row+1 < row ){
			if( numberList[(_row+1)*col + _col] == 0 ){
				numberList[(_row+1)*col + _col] = buttonNumber;
				numberList[_row*col + _col] = 0;
			}
		}
		if( _col-1 >= 0){
			if( numberList[(_row)*col + _col-1] == 0 ){
				numberList[(_row)*col + _col-1] = buttonNumber;
				numberList[_row*col + _col] = 0;
			}
		}
		if( _col+1 < col){
			if( numberList[(_row)*col + _col+1] == 0 ){
				numberList[(_row)*col + _col+1] = buttonNumber;
				numberList[_row*col + _col] = 0;
			}
		}
		
		
		clearButtons();
		makeButtons();
		
		fpgaControl.increaseCount();
		
		checkAnswer();
	}
	
	public void clearButtons(){
		for(int i=0; i < row; i++){
			LinearLayout childView = (LinearLayout)linear.getChildAt(2);
			childView.removeAllViewsInLayout();
			linear.removeView(childView);
		}
	}
	
	public void makeButtons(){
		//linear.setOrientation(LinearLayout.VERTICAL);
		
		//Log.v(TAG, row+" "+col+" "+widthPixel+" "+heightPixel+" "+numberList[0]);
		for(int i=0; i<row; i++){
			LinearLayout rowLayout = new LinearLayout(this);
			rowLayout.setLayoutParams(new LayoutParams(widthPixel, heightPixel/row));
			rowLayout.setTag("Buttons");
			
			for(int j=0; j<col; j++){
				Button btnTag = new Button(this);
				btnTag.setLayoutParams(new LayoutParams(widthPixel/col, heightPixel/row));
				btnTag.setText( String.valueOf(numberList[i*col+j]) );
				//btnTag.setOnClickListener(this);
				if(numberList[i*col+j] == 0)
					btnTag.setBackgroundColor(Color.BLACK);
				else
					btnTag.setOnClickListener(this);
				rowLayout.addView(btnTag);
			}
			linear.addView(rowLayout);
		}
	}
	
	public void checkAnswer(){
		//AlertDialog.Builder dialog = new AlertDialog.Builder(this, android.R.style.Theme_DeviceDefault_Light_Dialog);
		boolean check = true;
		int count = 1;
		for(int i=0; i<numberList.length-1; i++){
			if( numberList[i] != count ){
				check = false;
				break;
			}
			count++;
		}
		
		if( check ){
			AlertDialog dlg = null;
			AlertDialog.Builder dialog = new AlertDialog.Builder(this);
			
			dialog.setMessage("You Complete!")
				.setTitle("20131612")
				.setPositiveButton("ok", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						// TODO Auto-generated method stub
						mainActiviy.finish();
						dialog.dismiss();
					}
				})
				.setCancelable(false);
				
			dlg = dialog.create();
			dlg.show();
			row = col = 0;
		}
	}
}

class FpgaControl{
	public native int open();
	public native void close(int fd);
	public native void write(int fd, int value);

	int fd;
	int count;
	
	public FpgaControl(){
		System.loadLibrary("ndk-exam");
		fd = open();
		Log.v("tag", String.valueOf(fd));
		count = 0;
	}
	
	public void increaseCount(){
		count++;
		write(fd, count);
	}
	
	@Override
	protected void finalize(){
		close(fd);
	}
}
