# PowerPC Emulator — Dangerbin Test Programs

These are small C programs compiled with Symantec C++ under Mac OS 9 into genuine PowerPC PEF executables.

**Compiler configuration:**
- Target: PowerPC
- No `PPCANSI.o` or `PPCRuntime.o`
- No `InterfaceLib` or `MathLib`
- No standard headers
- Each program is compiled separately
- Do not execute these bare executables in SheepShaver: their runtime termination is unsafe

Each program writes a known value to the global `result` variable, which the emulator can inspect after execution.

## danger01 — Addition

Expected result: **42**

```c
volatile int result;

void main(void)
{
	volatile int a = 12;
	volatile int b = 30;

	result = a + b;
}
```

Tests integer addition, stack allocation, loads and stores.

## danger02 — Multiplication

Expected result: **42**

```c
volatile int result;

void main(void)
{
	volatile int a = 7;
	volatile int b = 6;

	result = a * b;
}
```

Tests integer multiplication, likely using `mullw`.

## danger03 — Conditional branching

Expected result: **18**

```c
volatile int result;

void main(void)
{
	volatile int a = 12;
	volatile int b = 30;

	if (a < b)
		result = b - a;
	else
		result = a - b;
}
```

Tests comparisons, conditional branches and subtraction.

## danger04 — For loop

Expected result: **55**

```c
volatile int result;

void main(void)
{
	volatile int i;
	volatile int sum = 0;

	for (i = 1; i <= 10; i++)
		sum += i;

	result = sum;
}
```

Tests repeated conditional branching, loop counters and arithmetic.

## danger05 — Arrays

Expected result: **30**

This version uses explicit element assignments to avoid problems with local array initialisation in Symantec C++.

```c
volatile int result;
int values[5] = {2, 4, 6, 8, 10};

void main(void)
{
	int i;
	int sum = 0;

	for (i = 0; i < 5; i++)
		sum += values[i];

	result = sum;
}
```

Tests stack-based arrays, address calculations, indexed memory access and loops.

An alternative uses a
