; SWIM.FRM
; This is the forms file for the SWIM application

!Form Main
Fredonia Swimming Association                {DateAndTime        }        `FORM]

Welcome to the Swimmer database.

Please make a selection with an "X":
   
    [A] Add a new swimmer
    [B] Search for a swimmer by:
             Last name: [LASTNAME            ]
        Swimmer number: [SWNum]
!EndForm
!Alias Add_Swimmer=A
!Alias Search=B

!Form SearchResults
Fredonia Swimming Association                {DateAndTime        }        `FORM]

More than one swimmer matched your criteria.

Please select a swimmer:

    Number Last Name              First Name    
[A] {N-1 } {LastName-1          } {FirstName-1    }
[B] {N-2 } {LastName-2          } {FirstName-2    }
[C] {N-3 } {LastName-3          } {FirstName-3    }
[D] {N-4 } {LastName-4          } {FirstName-4    }
[E] {N-5 } {LastName-5          } {FirstName-5    }
[F] {N-6 } {LastName-6          } {FirstName-6    }
[G] {N-7 } {LastName-7          } {FirstName-7    }
[H] {N-8 } {LastName-8          } {FirstName-8    }
[I] {N-9 } {LastName-9          } {FirstName-9    }
[J] {N-10} {LastName-10         } {FirstName-10   }
[K] {N-11} {LastName-11         } {FirstName-11   }
[L] {N-12} {LastName-12         } {FirstName-12   }
[M] {N-13} {LastName-13         } {FirstName-13   }
[N] {N-14} {LastName-14         } {FirstName-14   }
[O] {N-15} {LastName-15         } {FirstName-15   }

!EndForm
!Alias X-1=A
!Alias X-2=B
!Alias X-3=C
!Alias X-4=D
!Alias X-5=E
!Alias X-6=F
!Alias X-7=G
!Alias X-8=H
!Alias X-9=I
!Alias X-10=J
!Alias X-11=K
!Alias X-12=L
!Alias X-13=M
!Alias X-14=N
!Alias X-15=O

!Form EditSwimmer
Fredonia Swimming Association                {DateAndTime        }        `FORM]

{Operation                           }    "C" to Cancel, or blank to update: [X]

Swimmer ID: [SWNum]
 Last name: [LastName            ]
First name: [FirstName      ]
   Address: [Address                             ]
      City: [City                ]
     State: [St]
       Zip: [Zip  ]
     Phone: [Phone           ]
 Birthdate: Year: [Year]  Month: [MO]  Day: [DY]

!EndForm
!Alias State=St
