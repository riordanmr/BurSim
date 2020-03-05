; SNISWIM.FRM
; This is the forms file for the SNISWIM application

!Form Main
         ~LOGON - Menu-Assisted Resource Control            {Time  }            
                                                                                
                                                                               
                                                                                
                                                                                
                                 Welcome.                                       
                                                                                
                                                                                
                                                                                
              Please enter your usercode  [Usercode           ]                 
                                                                                
                    ...and your password [Password           ]                 
                                                                                
                                                                                
                                                                                
                                                                                
          
          
                                                                
                                                                                
                                                                                
                                                                               
 {BottomMsg1                                                                  }
 {BottomMsg2                                                                  }
!EndForm

!Form FirstMenu
~                        SNI Swimming Association           {Time  }    `FORM  ]

 Welcome to the SNI Swimming Association swimmer database.

 This program allows you to query, update, and add swimmer information.

 Please select from one of the following with an X:

         [A] Query swimmer info
         [B] Update a swimmer record
         [X] Exit




          ___                      ___                    ___     
         (   \                    (   \                  (   \
          \     \                  \     \                \     \
           \       \                \       \              \       \
____________)         \______________)         \____________)         \_______

!EndForm
!Alias Query=A
!Alias Update=B
!Alias Exit=X

!Form QueryMenu
~                        SNI Swimming Association           {Time  }    `FORM  ]

 Swimmer Query Screen

 Please select an option with an X, and a swimmer to query, if applicable.

             [A] Query by Swimmer ID 
                 Swimmer ID: [SwmID]

             [P] Exit to previous screen


!EndForm
!Alias QueryByID=A
!Alias Previous=P

!Form QueryResults
~                        SNI Swimming Association           {Time  }    `FORM  ]

 Swimmer Query Results

 SwimmerID: {SwmID}
 Last name: {LastName            }   First: {FirstName    }
   Address: {Address1                                   }
            {Address2                                   }
      City: {City               }    State: {ST}   Zip: {ZipCode }
     Phone: {Phone       }
 Birthdate: {BirthDt } (MM/DD/YYYY)


~{Results                                                                   }



             [P] Exit to previous screen

!EndForm
!Alias Previous=P

!Form UpdateMenu
~                        SNI Swimming Association           {Time  }    `FORM  ]

 Swimmer Update Screen

 Please select an option with an X, and a swimmer to update, if applicable.

             [A] Search by Swimmer ID 
                 Swimmer ID: [SwmID]

             [P] Exit to previous screen


!EndForm
!Alias QueryByID=A
!Alias Previous=P

!Form UpdateInput
~                        SNI Swimming Association           {Time  }    `FORM  ]

 Swimmer Query Results

 SwimmerID: `SwmID]
 Last name: [LastName            ]   First: [FirstName    ]
   Address: [Address1                                   ]
            [Address2                                   ]
      City: [City               ]    State: [ST]   Zip: [ZipCode   ]
     Phone: [Phone       ]
 Birthdate: [BirthDt   ] (MM/DD/YYYY)


~{Results                                                                   }



             [P] Exit to previous screen

!EndForm
!Alias Previous=P

