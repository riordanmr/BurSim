; FLIP.FRM
; This is the forms file for the SCRNDB test application
; Mark Riordan  13 Nov 2000

!Form Main
Screens database test               {DateAndTime        }              `FORM  ]

Use this program to test AND and OR conditions in the screens DB.

Enter X in one or more of the fields below:

[a] one
[b] two
[c] three


!EndForm
!Alias f1=a
!Alias f2=b
!Alias f3=c

!Form Result
Screens database test               {DateAndTime        }              `FORM  ]

This is the result form.  The rows below will have different
values depending in the input below.

{field1          }
{field2          }
{field3          }

!EndForm