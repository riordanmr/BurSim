short __cdecl screen_first_field(struct telnet_screen *)
{ return 0;}
int __cdecl ibm_next_unprot_field_with_wrap(struct telnet_screen *,int)
{ return 0;}
void __cdecl ug_erase_to_end_of_page_tel_screen(struct telnet_screen *)
{ }
int __cdecl ug_delete_line_tel_screen(int,struct telnet_screen *)
{ return 0;}
int __cdecl Init3270Module(void)
{ return 0;}
void __cdecl init_ibm_screen(int)
{ }
void __cdecl perform_ibm_key(int,int)
{ }
int __cdecl UGSendScreen(struct struct_conn *)
{ return 0;}
int ug_blank_screen(struct struct_conn *)
{ return 0;}
int ibm_screen_current_unprot_field(int conn_index,int mypos)
{ return 0;}
