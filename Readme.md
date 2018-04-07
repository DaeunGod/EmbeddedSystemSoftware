

### Target Board
HUINS Board

Dual core processor based on ARM Cortex-A9


### Step
> <pre><code>$sudo -s</code></pre>
> <pre><code>$source /root/.bashrc</code></pre>
> install the device driver in case of need

### mode
mode1 \
	init: show local time on FND, on LED(1) \
	step: SW(1)-> blink LED(3) and LED(4), you can change time \
				SW(2)-> reset time \
				SW(3)-> increase 1 hour \
				SW(4)-> increase 1 min \
\
mode2 \
	init: show the number 0000 on FND \
	require: ignore thousand digits \
	step: SW(1)-> change number into decimal, octal, quaternion, binary in turn \
				SW(2)-> increase hundred digits \
				SW(3)-> increase ten digits \
				SW(4)-> increase one digits
					 
### Shared Memory
0 - event key \
1 ~ 9 - sw Buttons \
10 - mode \
11 - led stat check \
12 ~ 15 - FND Data \
16 ~ 47 - LCD String Data \
48 - Dot matrix Data
