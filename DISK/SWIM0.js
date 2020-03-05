// SWIM.js - Implements simple swimmer database
// Mark Riordan   26 Sept 1999
var FormFileID = OpenFormFile("SWIM.FRM");
var FormIDMain = OpenForm(FormFileID,"MAIN");
var STOQ, ODTMsg;

var WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3;
var bRunning = true;

while(bRunning) {
   var which = Wait(9999,"SWIM",STOQ);
   switch(which) {
      case WAIT_TIMEOUT:
         break;
      case WAIT_ODT:
         Accept(ODTMsg);
         if("QUIT" = ODTMsg) bRunning = false;
         SPOMsg("Unrecognized command: " + ODTMsg);
         break;
      case WAIT_STOQ:
         retval = GetSTOQMsg("FLDTST", Header, InBuf);
         break;
   }
}