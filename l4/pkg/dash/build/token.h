#define TEOF 0
#define TNL 1
#define TSEMI 2
#define TBACKGND 3
#define TAND 4
#define TOR 5
#define TPIPE 6
#define TLP 7
#define TRP 8
#define TENDCASE 9
#define TENDBQUOTE 10
#define TREDIR 11
#define TWORD 12
#define TNOT 13
#define TCASE 14
#define TDO 15
#define TDONE 16
#define TELIF 17
#define TELSE 18
#define TESAC 19
#define TFI 20
#define TFOR 21
#define TIF 22
#define TIN 23
#define TTHEN 24
#define TUNTIL 25
#define TWHILE 26
#define TBEGIN 27
#define TEND 28

/* Array indicating which tokens mark the end of a list */
const char tokendlist[] = {
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	1,
};

const char *const tokname[] = {
	"end of file",
	"newline",
	"\";\"",
	"\"&\"",
	"\"&&\"",
	"\"||\"",
	"\"|\"",
	"\"(\"",
	"\")\"",
	"\";;\"",
	"\"`\"",
	"redirection",
	"word",
	"\"!\"",
	"\"case\"",
	"\"do\"",
	"\"done\"",
	"\"elif\"",
	"\"else\"",
	"\"esac\"",
	"\"fi\"",
	"\"for\"",
	"\"if\"",
	"\"in\"",
	"\"then\"",
	"\"until\"",
	"\"while\"",
	"\"{\"",
	"\"}\"",
};

#define KWDOFFSET 13

STATIC const char *const parsekwd[] = {
	"!",
	"case",
	"do",
	"done",
	"elif",
	"else",
	"esac",
	"fi",
	"for",
	"if",
	"in",
	"then",
	"until",
	"while",
	"{",
	"}"
};
