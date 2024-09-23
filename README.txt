# Homework 4: PL/0 Compiler

## Description
A brief overview of your virtual machine implementation, explaining its purpose and functionality.

My PL/0 compiler program will take a text file, tokenize its content while checking for invalid characters, numbers, or identifiers, then check whether its grammar aligns
with the EBNF grammar rules specified in the HW4 document. If my program determines its input file contents to be adhering to the grammar and symbol rules of PL/0, it will then
generate code in the form of instructions to then output to an Executable Linkable File or ELF. The generated ELF is designed to be used with the vm created in HW1. Errors in the
provided text in the input file are thrown if there is an error in the scanning OR parsing step. The compiler has no "else" statement functionality.

## Compilation Instructions
Detailed commands to compile your code:

Ensure you have the "hw4compiler.h" file in the same directory as "hw4compiler.c"
```bash
gcc hw4compiler.c -o hw4compiler.out
```
This will compile the hw4compiler.c file into a file called "hw4compiler.out".

## Usage
How to run your compiled program, including any necessary arguments:
```bash
./hw4compiler.out [INPUT FILENAME]
```
Replace [INPUT FILENAME] with whatever the name of your input file is. If your input file contains code which follows the rules of PL/0, hw4compiler will
generate executable for it and output said code into a file named "elf.txt" which can be used with the vm in HW1 (see readme from HW1 for more details on using
the ELF).

## Example
Provide a simple example to illustrate how to use your program:

### Contents of input.txt
```
var a, b, answer;
procedure gcd;
	procedure run;
		var q, r, t;
		begin
			t := b;
			q := a / b;
			b := a - (q * b);
			a := t;
		end;
	begin
		while b <> 0 do call run;
		if b = 0 then answer := a fi;
	end;
begin
	read a;
	read b;
	if b > a then begin
		answer := a;
		a := b;
		b := answer;
		answer := 0;
	end fi;
	call gcd;
	write answer;
end.
```

```bash
./hw4compiler.out input.txt
```
```
### Output of command
Source Program:
var a, b, answer;
procedure gcd;
        procedure run;
                var q, r, t;
                begin
                        t := b;
                        q := a / b;
                        b := a - (q * b);
                        a := t;
                end;
        begin
                while b <> 0 do call run;
                if b = 0 then answer := a fi;
        end;
begin
        read a;
        read b;
        if b > a then begin
                answer := a;
                a := b;
                b := answer;
                answer := 0;
        end fi;
        call gcd;
        write answer;
end.


No errors, program is syntactically correct

Assembly Code:
Line    OP      L       M
0       JMP     0       109
1       JMP     0       67
2       JMP     0       19
3       INC     0       6
4       LOD     2       4
5       STO     0       5
6       LOD     2       3
7       LOD     2       4
8       DIV     2       4
9       STO     0       3
10      LOD     2       3
11      LOD     0       3
12      LOD     2       4
13      MUL     2       3
14      SUB     0       2
15      STO     2       4
16      LOD     0       5
17      STO     2       3
18      RTN     0       0
19      INC     0       6
20      LOD     1       4
21      LIT     0       0
22      NEQ     0       6
23      JPC     0       88
24      CAL     0       16
25      JMP     0       70
26      LOD     1       4
27      LIT     0       0
28      EQL     0       5
29      JPC     0       106
30      LOD     1       3
31      STO     1       5
32      RTN     0       0
33      INC     0       6
34      SYS     0       2
35      STO     0       3
36      SYS     0       2
37      STO     0       4
38      LOD     0       4
39      LOD     0       3
40      GTR     0       9
41      JPC     0       160
42      LOD     0       3
43      STO     0       5
44      LOD     0       4
45      STO     0       3
46      LOD     0       5
47      STO     0       4
48      LIT     0       0
49      STO     0       5
50      CAL     0       13
51      LOD     0       5
52      SYS     0       1
53      SYS     0       3
```
Since this example uses functional code, hw4compiler will also generate a "elf.txt" file which can be ran
using the vm from HW1 (see HW1 readme for instruction on use).
```bash
7 0 109
7 0 67
7 0 19
6 0 6
3 2 4
4 0 5
3 2 3
3 2 4
2 2 4
4 0 3
3 2 3
3 0 3
3 2 4
2 2 3
2 0 2
4 2 4
3 0 5
4 2 3
2 0 0
6 0 6
3 1 4
1 0 0
2 0 6
8 0 88
5 0 16
7 0 70
3 1 4
1 0 0
2 0 5
8 0 106
3 1 3
4 1 5
2 0 0
6 0 6
9 0 2
4 0 3
9 0 2
4 0 4
3 0 4
3 0 3
2 0 9
8 0 160
3 0 3
4 0 5
3 0 4
4 0 3
3 0 5
4 0 4
1 0 0
4 0 5
5 0 13
3 0 5
9 0 1
9 0 3
```

## Team Information (if applicable)
SOLO

## Contact Information
For any questions or issues, please contact:

Name: Tucker Carroll
email: tu512807@ucf.edu