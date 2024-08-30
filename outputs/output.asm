LABEL   main
MOV 42  x
MOV 3.14  y
MOV 'a'  z
MOV "Hello, World!"  hello
> x y t0
!= z 'b' t1
&& t0 t1 t2
IF_FALSE t2  L1
+ x 1 t3
MOV t3  y
GOTO   L2
LABEL   L1
- x 1 t4
MOV t4  y
LABEL   L2
RET 0  
