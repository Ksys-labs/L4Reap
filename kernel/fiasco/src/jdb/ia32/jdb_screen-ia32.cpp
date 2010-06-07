IMPLEMENTATION [ia32,ux]:

const char* const Jdb_screen::Reg_names[] 	= { "EAX", "EBX", "ECX", "EDX", 
						    "EBP", "ESI", "EDI", "EIP", 
						    "ESP", "EFL" };
const char Jdb_screen::Reg_prefix 		= 'E';

const char* Jdb_screen::Root_page_table		= "pdir: ";

//----------------------------------------------------------------------------
IMPLEMENTATION [amd64]:

const char * const Jdb_screen::Reg_names[] 	= { "RAX", "RBX", "RCX", "RDX", 						    "RBP", "RSI", "RDI", "R8",
    						    "R9", "R10", "R11", "R12",
						    "R13", "R14", "R15", "RIP",
						    "RSP", "RFL" };
const char  Jdb_screen::Reg_prefix		 = 'R';

const char* Jdb_screen::Root_page_table		 = "pml4: ";
