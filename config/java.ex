[ExtensionFile]
EXTENSIONTYPE: Java
EXTENSIONS: *.jar
SEPS: !%&/()=?+-.,:;*^~><\\[]{}|\t\"'´` 
STRING: "
CHAR: '
LITERAL: \
LINECONTINUATION: \
BRACES: {,}:[,]:(,):/*,*/
BLOCKBEGIN: {
BLOCKEND: }
LINECOMMENT: //
COMMENTBEGIN: /*
COMMENTEND: */
COMMENTWHOLEWORD: 0
COMMENTFIRSTWORD: 0
PREPROCESSOR: #
IGNORECASE: 0
TabSize: 0
Indent: 0
Numbers: 0[xX]+[0-9a-fA-F]+[lLuU]*|0[1234567]*[lLuU]*|[0-9]+[uUlL]+|([0-9]+(\.)*[0-9]*|[0-9]*(\.)+[0-9]+)+([eE]+[-+]*[0-9]*)*
TabPos: 4 7 10 13 16 19 22
PreProcWords: define pragma if error include ifdef elif ifndef endif undef else line
[COLOR]
MarkText: 0x000000
MarkBackGround: 0xA0A0A0
TextText: 0x000000
TextBackGround: 0xFFFFFF
SeperatorsText: 0xFF0000
SeperatorsBackGround: 0xFFFFFF
PreProcessorText: 0x00A000
PreProcessorBackGround: 0xFFFFFF
CommentsText: 0xA0A0A0
CommentsBackGround: 0xFFFFFF
StringsText: 0x909000
StringsBackGround: 0xFFFFFF
NumbersText: 0xFF00FF
NumbersBackGround: 0xFFFFFF
TagsText: 0x800000
TagsBackGround: 0xFFFFFF
Keyword0Text: 0x0000E6
Keyword0BackGround: 0xFFFFFF
Keyword1Text: 0x0000D2
Keyword1BackGround: 0xFFFFFF
Keyword2Text: 0x0000BE
Keyword2BackGround: 0xFFFFFF
Keyword3Text: 0x0000AA
Keyword3BackGround: 0xFFFFFF
Keyword4Text: 0x000096
Keyword4BackGround: 0xFFFFFF
[TAGS]
Program: CTAGS.EXE
Options: --c-types=+px-n
File: $T$\$FFE$.tag
UseTagColor: 1
[Templates]
!*!fhead
/******************************************************************************\n * Description:\n *   \c\n * Parameters:\n *\n *****************************************************************************/
!*!if
if(\c)\n{\n}
!*!inc
#include ".\\..\\src\\\c.h"
!*!Inc
#include <\c.h>
!*!com
/* \c */
!*!ca
case \c:\n  break;  
!*!usi
unsigned int \c
!*!usc
unsigned char \c
!*!uss
unsigned short \c
!*!ss
signed short \c
!*!mod
/*\t\t\tModified\nBy: \U\c\n\T\S\D */
!*!todo
/* EGPRS TODO \U */
!*!for
for(\c;;)\n{\n}
!*!{
{\n\i\c\n}
!*!fhead
/******************************************************************************\n * Description:\n *   \c\n * Parameters:\n *\n *****************************************************************************/
!*!if
if(\c)\n{\n}
!*!inc
#include ".\\..\\src\\\c.h"
!*!Inc
#include <\c.h>
!*!com
/* \c */
!*!ca
case \c:\n  break;  
!*!usi
unsigned int \c
!*!usc
unsigned char \c
!*!uss
unsigned short \c
!*!ss
signed short \c
!*!mod
/*\t\t\tModified\nBy: \U\c\n\T\S\D */
!*!todo
/* EGPRS TODO \U */
!*!for
for(\c;;)\n{\n}
!*!{
{\n\i\c\n}
[Keyword 0]
break
case
char
const
continue
default
do
double
else
enum
extern
float
for
goto
if
int
long
return
short
signed
sizeof
static
struct
switch
typedef
union
unsigned
void
volatile
while
[Keyword 1]
BOOL
FALSE
NULL
TRUE
UINT
[Keyword 2]
CHAR_BIT
CHAR_MAX
CHAR_MIN
EOF
EXIT_FAILURE
EXIT_SUCCESS
FILE
INT_MAX
INT_MIN
RAND_MAX
SCHAR_MAX
SCHAR_MIN
SEEK_CUR
SEEK_END
SEEK_SET
SHRT_MAX
SHRT_MIN
UCHAR_MAX
UINT_MAX
USHRT_MAX
abs
acos
asin
assert
atan
atexit
bsearch
calloc
clock
clock_t
cos
ctime
difftime
div
eof
errno
exit
exp
fclose
feof
fflush
fgetc
fgets
fopen
fprintf
fputc
fputs
fread
free
fseek
ftell
fwrite
getc
getchar
getenv
isalnum
isalpha
isascii
iscntrl
isdigit
islower
isprint
ispunct
isspace
isupper
isxdigit
labs
ldiv
localtime
log
longjmp
malloc
memcmp
memcpy
memmove
memset
offsetof
pow
printf
putc
puts
qsort
rand
realloc
remove
rename
setjmp
sin
size_t
sprintf
sqrt
srand
stderr
stdin
stdout
strcat
strchr
strcmp
strcpy
strcspn
strftime
strlen
strncat
strncmp
strncpy
strrchr
strstr
strtod
strtok
strtol
system
tan
time
time_t
tolower
toupper
va_arg
va_end
va_list
va_start
vfprintf
vprintf
vsprintf
[Keyword 3]
begin
bool
c_str
catch
cerr
cin
class
clear
copy
cout
delete
empty
end
endl
erase
false
find
find_first_of
first
friend
getline
ifstream
inline
insert
is_open
iterator
list
lower_bound
map
new
npos
ofstream
open
operator
ostream
pop_front
pos_type
private
protected
public
push_back
resize
second
size
size_type
sort
std
string
stringstream
substr
template
this
true
try
typename
vector
virtual
[Keyword 4]
int16_t
int32_t
int8_t
uint16_t
uint32_t
uint8_t
