       Hello World with delayed increments and find cell loop;
	      Initially; set and print the first letter;
     then create cells containing all the other letters and print
			    them in one go

Set H in Cell 2
H 72
> >
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++
. [-]

Now set the rest of the characters in cell 1 to 12 in one go
<
e 101
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
+
>

l 108
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++
>

l 108
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++
>

o 111
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++
+
>

SP 32
++++++++++ ++++++++++ ++++++++++ ++
>

W 87
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ +++++++
>

o 111
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ +
>

r 114
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++
>

l 108
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++
>

d 100
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
++++++++++ ++++++++++ ++++++++++ ++++++++++ ++++++++++
>

! 33
++++++++++ ++++++++++ ++++++++++ +++

Throw in a copy loop for good measure
copy the current cell to the next two adjecent cells
[ - > + > + << ]
Move the second cell over to the one copied from
>> [ - << + >> ]

Set the next character by subtracting from the first cell
copied to

LF 10
<
---------- ---------- ---

Go to cell 0 with a find zero cell loop and print all the filled cells
[<] > [.>]
