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
				if( row < 1 || col < 1 || row > 4 || col > 4){
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
		
		for(int i=0; i<temp.length-1; i++){
			temp[i] = (int)(Math.random()*(row*col-1))+1;
			for(int j=i-1; j>=0; j--){
				if(temp[i] == temp[j]){
					i--;
					break;
				}
			}
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
				btnTag.setOnClickListener(this);
				if(numberList[i*col+j] == 0)
					btnTag.setBackgroundColor(Color.BLACK);
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
		}
	}
}
