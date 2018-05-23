

### Target Board
HUINS Board

Dual core processor based on ARM Cortex-A9


### Step
> <pre><code>$sudo -s</code></pre>
> <pre><code>$source /root/.bashrc</code></pre>
> install the device driver in case of need

### modes
mode1 
-	init: show local time on FND, on LED(1) 
-	step 
- 		SW(1)-> blink LED(3) and LED(4), you can change time 
		SW(2)-> reset time 
 		SW(3)-> increase 1 hour 
 		SW(4)-> increase 1 min 

mode2 
-	init: show the number 0000 on FND 
-	require: ignore thousand digits 
-	step 
- 		SW(1)-> change number into decimal, octal, quaternion, binary in turn 
		SW(2)-> increase hundred digits 
		SW(3)-> increase ten digits 
		SW(4)-> increase one digits

mode3
- 	init: show empty string on LCD, 0 count on fnd, character input mode
- 	step
- 		SW(1)~SW(9)-> put a character or number in string and show on LCD
 		SW(2)+SW(3)-> clear
 		SW(5)+SW(6)-> change input mode, character or number, change what the dot matrix shows.
 		SW(8)+SW(9)-> put a white space in string.
- 	common
- 		if you click the switch button, then increase count

mode4
- 	init: cursor is on the left top side on the dot matrix and it is blinking. count is 0
- 	step
- 		SW(2),(4),(6),(8)-> Move cursor up, left, right, down
 		SW(5)-> Mark the position of cursor(Set)
 		SW(1)-> clear and reset
 		SW(3)-> cursor show on/off
 		SW(7)-> clear the data, remember cursor position
		SW(9)-> reverse every data on the dot matrix

mode5
-	init: empty
-	step
-	
					 
### Shared Memory
0 	- event key \
1 ~ 9 	- sw Buttons \
10 	- mode \
11 	- led value \
12 ~ 15 - FND Data \
16 ~ 47 - LCD String Data \
48 	- Dot matrix index \
49 ~ 58 - Dot Matrix Table
