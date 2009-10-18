/* Built-in files */

#include "types.h"

const unsigned char * const builtins[]=
{
	US "jupprc",
		US "-asis\n"
		"-assume_color\n"
		"-dopadding\n"
		"--force\n"
		"-keepup\n"
		"-mid\n"
		"-nobackups\n"
		"-noxon\n"
		"-pg 2\n"
		"-lmsg \\i%k%T%*\\b%n\\b%R\n"
		"-rmsg  R%r<%l C%c\\u%o|%O\\i\\b%a|%A\\b\\i\\u %u\n"
		"-guess_crlf\n"
		"-french\n"
		"-indentc 9\n"
		"-istep 1\n"
		"-guess_indent\n"
		"--autoindent\n"
		"-purify\n"
		"-highlight\n"
		"--linums\n"
		"-lmargin 1\n"
		"-rmargin 73\n"
		"--smarthome\n"
		"--indentfirst\n"
		"-smartbacks\n"
		"-tab 8\n"
		"--wordwrap\n"
		"\n"
		"*.py\n"
		"-encoding utf8\n"
		"--guess_indent\n"
		"-tab 4\n"
		"-indentc 32\n"
		"-istep 4\n"
		"-spaces\n"
		"\n"
		"*\n"
		"+#!\\+\\[	 ]\\+\\[a-z/]/python\n"
		"-encoding utf8\n"
		"--guess_indent\n"
		"-tab 4\n"
		"-indentc 32\n"
		"-istep 4\n"
		"-spaces\n"
		"\n"
		"*\n"
		"+#!\\+\\[	 ]\\+\\[a-z/]/env\\+\\[	 ]python\n"
		"-encoding utf8\n"
		"--guess_indent\n"
		"-tab 4\n"
		"-indentc 32\n"
		"-istep 4\n"
		"-spaces\n"
		"\n"
		"{Basic\n"
		"\\i   Help Screen    turn off with ^J     more help with ESC . (^[.)              \\i\n"
		"\\i \\i\\u\\bCURSOR\\b\\u           \\u\\bGOTO\\b\\u             \\u\\bBLOCK\\b\\u    \\u\\bDELETE\\b\\u    \\u\\bMISC\\b\\u         \\u\\bEXIT\\b\\u       \\i \\i\n"
		"\\i \\i^S left ^D right ^R  prev. screen ^KV move ^G  char  ^B  reformat ^KX save   \\i \\i\n"
		"\\i \\i^E up   ^X down  ^C  next screen  ^KC copy ^Y  line  ^V  overtype ^KQ abort  \\i \\i\n"
		"\\i \\i^A previous word ^QS beg. of line ^KY kill ^T  >word ^QL refresh  ^KZ shell  \\i \\i\n"
		"\\i \\i^F next word     ^QD end of line  ^K/ pipe ^QY >line ^O  options  \\u\\bFILE\\b\\u       \\i \\i\n"
		"\\i \\i\\u\\bSEARCH\\b\\u           ^QR top of file  ^KB begin          \\u\\bBUFFER\\b\\u       ^KE new    \\i \\i\n"
		"\\i \\i^QF find first   ^QC end of file  ^KK end  ^K] space ^U undo      ^KR import \\i \\i\n"
		"\\i \\i^L  find next    ^QI to line No.  ^KD reformat       ^^ redo      ^KW export \\i \\i\n"
		"}\n"
		"\n"
		"{Windows\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,    next screen ^[.     \\i\n"
		"\\i \\i^KO Split the window in half         ^KE Load file into window               \\i \\i\n"
		"\\i \\i^KG Make current window bigger       ^KT Make current window smaller         \\i \\i\n"
		"\\i \\i^KN Go to the window below           ^KP Go to the window above              \\i \\i\n"
		"\\i \\i^KQ Eliminate the current window     ^KI Show all windows / Show one window  \\i \\i\n"
		"\\i \\i\\u\\bSpecial help for XON/XOFF aware terminals\\b\\u                                    \\i \\i\n"
		"\\i \\i You can use \\b^[q\\b and \\b^[s\\b instead of \\b^Q\\b and \\b^S\\b to initiate a command.         \\i \\i\n"
		"}\n"
		"\n"
		"{Advanced\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,    next screen ^[.     \\i\n"
		"\\i \\i\\u\\bMACROS\\b\\u      \\u\\bMISC\\b\\u          \\u\\bSCROLL\\b\\u      \\u\\bSHELL\\b\\u       \\u\\bGOTO\\b\\u        \\u\\bI-SEARCH\\b\\u       \\i \\i\n"
		"\\i \\i^[( Record  ^Q? status    ^Q. Right   ^K' Window  ^QB to ^KB  ^[R Backwards  \\i \\i\n"
		"\\i \\i^[) Stop    ^QQ repeat    ^Q, Left    ^[! Command ^QK to ^KK  ^[S Forwards   \\i \\i\n"
		"\\i \\i^[ 0-9 Play ^QM Math      \\u\\bCharSEARCH\\b\\u  \\u\\bQUOTE\\b\\u       \\u\\bDELETE\\b\\u      \\u\\bBOOKMARKS\\b\\u      \\i \\i\n"
		"\\i \\i^[? Query   ^KA CenterLn. ^QH forwrd   ` Ctrl-    ^[Y yank    ^K 0-9 Set     \\i \\i\n"
		"\\i \\i^[D Dump    ^[H Message   ^QG backwd  ^P Meta-    ^[O word<   ^Q 0-9 Goto    \\i \\i\n"
		"}\n"
		"\n"
		"{Programs\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,    next screen ^[.     \\i\n"
		"\\i \\i\\u\\bGOTO\\b\\u                  \\u\\bINDENT\\b\\u         \\u\\bCOMPILING\\b\\u                   \\u\\bMACROS\\b\\u      \\i \\i\n"
		"\\i \\i^Q[ Matching brace    ^K. more       ^[C Compile & parse errors  ^N play #0  \\i \\i\n"
		"\\i \\i^QP Previous place    ^K, less       ^[E Parse errors            \\u\\bSLIDING\\b\\u     \\i \\i\n"
		"\\i \\i^K= Next place        \\u\\bSPECIAL\\b\\u        ^[M Goto next error         ^W up       \\i \\i\n"
		"\\i \\i^K; Split+Tag search  ^[- jexecmd    ^[N Goto previous error     ^Z down     \\i \\i\n"
		"}\n"
		"\n"
		"{Search\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,    next screen ^[.     \\i\n"
		"\\i \\i\\u\\bSpecial search sequences\\b\\u                                                     \\i \\i\n"
		"\\i \\i    \\\\^  \\\\$  matches beg./end of line       \\\\?     match any single char      \\i \\i\n"
		"\\i \\i    \\\\<  \\\\>  matches beg./end of word       \\\\*     match 0 or more chars      \\i \\i\n"
		"\\i \\i    \\\\c     matches balanced C expression   \\\\\\\\     matches a \\\\                \\i \\i\n"
		"\\i \\i    \\\\[..]  matches one of a set            \\\\n     matches a newline          \\i \\i\n"
		"\\i \\i    \\\\+     matches 0 or more of the character which follows the \\\\+           \\i \\i\n"
		"\\i \\i\\u\\bSpecial replace sequences\\b\\u                                                    \\i \\i\n"
		"\\i \\i    \\\\&     replaced with text which matched search string                    \\i \\i\n"
		"\\i \\i    \\\\0 - 9 replaced with text which matched \\bN\\bth \\\\*, \\\\?, \\\\c, \\\\+, or \\\\[..]     \\i \\i\n"
		"\\i \\i    \\\\\\\\     replaced with \\\\                 \\\\n     replaced with newline      \\i \\i\n"
		"}\n"
		"\n"
		"{Names\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,    next screen ^[.     \\i\n"
		"\\i \\i At file name prompts use the cursor up/down keys to access a history of     \\i \\i\n"
		"\\i \\i recently used files or the tab key to complete them.  \\bSpecial file names:\\b   \\i \\i\n"
		"\\i \\i      !command                 Pipe in/out of a shell command                \\i \\i\n"
		"\\i \\i      >>filename               Append to a file                              \\i \\i\n"
		"\\i \\i      -                        Read/Write to/from standard I/O               \\i \\i\n"
		"\\i \\i      filename,START,SIZE      Read/Write a part of a file/device            \\i \\i\n"
		"\\i \\i          Give START/SIZE in decimal (255), octal (0377) or hex (0xFF)       \\i \\i\n"
		"}\n"
		"\n"
		"{Joe\n"
		"\\i   Help Screen    turn off with ^J     prev. screen ^[,                        \\i\n"
		"\\i \\i \\bJUPP\\b is based upon JOE (Joe's Own Editor) 2.8/3.x \\d(GPL v1)\\d by Joe H. Allen; \\i \\i\n"
		"\\i \\i go to \\uhttp://sf.net/projects/joe-editor/\\u for upstream bug reports. JUPP 2.8 \\i \\i\n"
		"\\i \\i for DOS compiled by A. Totlis, packed with LHarc 2.13; JUPP 3.x for UNIX\\d(R)\\d \\i \\i\n"
		"\\i \\i at \\uhttp://mirbsd.de/jupp\\u and by \\bThorsten \"\\dmirabilos\\d\" Glaser <\\utg@mirbsd.org\\u>\\b \\i \\i\n"
		"\\i \\i @(#) blt_in 2009-10-18; autoCR-LF; UTF-8 tty by LC_CTYPE; per-file encoding \\i \\i\n"
		"}\n"
		"\n"
		":windows\n"
		"type		^@ TO �\n"
		"abort		^K Q\n"
		"abort		^K ^Q\n"
		"abort		^K q\n"
		"arg		^Q Q\n"
		"arg		^Q ^Q\n"
		"arg		^Q q\n"
		"arg		^[ q q\n"
		"explode		^K I\n"
		"explode		^K ^I\n"
		"explode		^K i\n"
		"help		^J\n"
		"help		^[ [ 1 1 ~\n"
		"hnext		^[ .\n"
		"hprev		^[ ,\n"
		"math		^Q M\n"
		"math		^Q ^M\n"
		"math		^Q m\n"
		"math		^[ q m\n"
		"msg		^[ H\n"
		"msg		^[ h\n"
		"nextw		^K N\n"
		"nextw		^K ^N\n"
		"nextw		^K n\n"
		"play		^[ 0 TO 9\n"
		"prevw		^K P\n"
		"prevw		^K ^P\n"
		"prevw		^K p\n"
		"query		^[ ?\n"
		"quote		`\n"
		"quote8		^P\n"
		"record		^[ (\n"
		"retype		^Q L\n"
		"retype		^Q ^L\n"
		"retype		^Q l\n"
		"retype		^[ q l\n"
		"rtn		^M\n"
		"shell		^K Z\n"
		"shell		^K ^Z\n"
		"shell		^K z\n"
		"stop		^[ )\n"
		"\n"
		":main\n"
		":inherit windows\n"
		"bof,qrepl,\"\\\\[\",quote,\"i\",quote,\"k\",quote,\"l\",quote,\"m ]\\\\+\\\\[\",quote,\"i\",quote,\"k\",quote,\"l\",quote,\"m ]\\\\$\",rtn,rtn,rtn,\"r\",eof	^K ]\n"
		"edit,rtn,filt,query,parserr	^[ C\n"
		"edit,rtn,filt,query,parserr	^[ c\n"
		"backs		^?\n"
		"backs		^H\n"
		"backw		^[ o\n"
		"bknd		^K '\n"
		"bkwdc		^Q G ^@ TO �\n"
		"bkwdc		^Q ^G ^@ TO �\n"
		"bkwdc		^Q g ^@ TO �\n"
		"bkwdc		^[ q g ^@ TO �\n"
		"blkcpy		^K C\n"
		"blkcpy		^K ^C\n"
		"blkcpy		^K c\n"
		"blkdel		^K Y\n"
		"blkdel		^K ^Y\n"
		"blkdel		^K y\n"
		"blkmove		^K V\n"
		"blkmove		^K ^V\n"
		"blkmove		^K v\n"
		"blksave		^K W\n"
		"blksave		^K ^W\n"
		"blksave		^K w\n"
		"bof		^Q R\n"
		"bof		^Q ^R\n"
		"bof		^Q r\n"
		"bof		^[ [ 1 ; 5 H\n"
		"bof		^[ q r\n"
		"bol		.kh\n"
		"bol		^Q S\n"
		"bol		^Q ^S\n"
		"bol		^Q s\n"
		"bol		^[ [ 1 ~\n"
		"bol		^[ [ 7 ~\n"
		"bol		^[ [ H\n"
		"bol		^[ q s\n"
		"bos		^Q X\n"
		"bos		^Q ^X\n"
		"bos		^Q x\n"
		"bos		^[ q x\n"
		"center		^K A\n"
		"center		^K ^A\n"
		"center		^K a\n"
		"crawll		^Q ,\n"
		"crawll		^[ q ,\n"
		"crawlr		^Q .\n"
		"crawlr		^[ q .\n"
		"delbol		^Q ^?\n"
		"delbol		^Q ^H\n"
		"delbol		^[ q ^?\n"
		"delbol		^[ q ^H\n"
		"delch		.kD\n"
		"delch		^G\n"
		"delch		^[ [ 3 ~\n"
		"deleol		^Q Y\n"
		"deleol		^Q ^Y\n"
		"deleol		^Q y\n"
		"deleol		^[ q y\n"
		"dellin		^Y\n"
		"delw		^T\n"
		"dnarw		.kd\n"
		"dnarw		^X\n"
		"dnarw		^[ O B\n"
		"dnarw		^[ [ B\n"
		"dnslide		^Z\n"
		"edit		^K E\n"
		"edit		^K ^E\n"
		"edit		^K e\n"
		"eof		^Q C\n"
		"eof		^Q ^C\n"
		"eof		^Q c\n"
		"eof		^[ [ 1 ; 5 F\n"
		"eof		^[ q c\n"
		"eol		.@7\n"
		"eol		.kH\n"
		"eol		^Q D\n"
		"eol		^Q ^D\n"
		"eol		^Q d\n"
		"eol		^[ [ 4 ~\n"
		"eol		^[ [ 8 ~\n"
		"eol		^[ [ F\n"
		"eol		^[ q d\n"
		"execmd		^[ -\n"
		"exsave		^K X\n"
		"exsave		^K ^X\n"
		"exsave		^K x\n"
		"ffirst		^Q F\n"
		"ffirst		^Q ^F\n"
		"ffirst		^Q f\n"
		"ffirst		^[ q f\n"
		"filt		^K /\n"
		"fmtblk		^K D\n"
		"fmtblk		^K ^D\n"
		"fmtblk		^K d\n"
		"fnext		.k3\n"
		"fnext		^L\n"
		"fnext		^[ [ 1 3 ~\n"
		"format		^B\n"
		"fwrdc		^Q H ^@ TO �\n"
		"fwrdc		^Q ^H ^@ TO �\n"
		"fwrdc		^Q h ^@ TO �\n"
		"fwrdc		^[ q h ^@ TO �\n"
		"gomark		^Q 0 TO 9\n"
		"gomark		^[ q 0 TO 9\n"
		"groww		^K G\n"
		"groww		^K ^G\n"
		"groww		^K g\n"
		"insf		^K R\n"
		"insf		^K ^R\n"
		"insf		^K r\n"
		"isrch		^[ S\n"
		"isrch		^[ s\n"
		"lindent		^K ,\n"
		"line		^Q I\n"
		"line		^Q ^I\n"
		"line		^Q i\n"
		"line		^[ q i\n"
		"ltarw		.kl\n"
		"ltarw		^S\n"
		"ltarw		^[ O D\n"
		"ltarw		^[ [ D\n"
		"ltarw		^[ s\n"
		"macros		^[ D\n"
		"macros		^[ d\n"
		"markb		^K B\n"
		"markb		^K ^B\n"
		"markb		^K b\n"
		"markk		^K K\n"
		"markk		^K ^K\n"
		"markk		^K k\n"
		"markl		^K L\n"
		"markl		^K ^L\n"
		"markl		^K l\n"
		"mode		^O\n"
		"mode,\"T\"	.kI\n"
		"mode,\"T\"	^V\n"
		"nextpos		^K =\n"
		"nextword	^F\n"
		"nextword	^[ [ 1 ; 5 C\n"
		"nmark		^K H\n"
		"nmark		^K ^H\n"
		"nmark		^K h\n"
		"nxterr		^[ M\n"
		"nxterr		^[ m\n"
		"open		^[ b\n"
		"parserr		^[ E\n"
		"parserr		^[ e\n"
		"pgdn		.kN\n"
		"pgdn		^C\n"
		"pgdn		^[ [ 6 ~\n"
		"pgup		.kP\n"
		"pgup		^R\n"
		"pgup		^[ [ 5 ~\n"
		"play,\"0\"	^N\n"
		"prevpos		^Q P\n"
		"prevpos		^Q ^P\n"
		"prevpos		^Q p\n"
		"prevpos		^[ q p\n"
		"prevword	^A\n"
		"prevword	^[ [ 1 ; 5 D\n"
		"prverr		^[ N\n"
		"prverr		^[ n\n"
		"qrepl		^Q A\n"
		"qrepl		^Q ^A\n"
		"qrepl		^Q a\n"
		"qrepl		^[ q a\n"
		"redo		^^\n"
		"rindent		^K .\n"
		"rsrch		^[ R\n"
		"rsrch		^[ r\n"
		"rtarw		.kr\n"
		"rtarw		^D\n"
		"rtarw		^[ O C\n"
		"rtarw		^[ [ C\n"
		"run		^[ !\n"
		"save		^K S\n"
		"save		^K ^S\n"
		"save		^K s\n"
		"setmark		^K 0 TO 9\n"
		"shrinkw		^K T\n"
		"shrinkw		^K ^T\n"
		"shrinkw		^K t\n"
		"splitw		^K O\n"
		"splitw		^K ^O\n"
		"splitw		^K o\n"
		"splitw,tag	^K ;\n"
		"stat		^Q ?\n"
		"stat		^[ q ?\n"
		"tomarkb		^Q B\n"
		"tomarkb		^Q ^B\n"
		"tomarkb		^Q b\n"
		"tomarkb		^[ q b\n"
		"tomarkk		^Q K\n"
		"tomarkk		^Q ^K\n"
		"tomarkk		^Q k\n"
		"tomarkk		^[ q k\n"
		"tomatch		^Q [\n"
		"tomatch		^Q ]\n"
		"tomatch		^Q ^[\n"
		"tomatch		^Q ^]\n"
		"tomatch		^[ q [\n"
		"tomatch		^[ q ]\n"
		"tos		^Q E\n"
		"tos		^Q ^E\n"
		"tos		^Q e\n"
		"tos		^[ q e\n"
		"undo		^U\n"
		"undo		^_\n"
		"uparw		.ku\n"
		"uparw		^E\n"
		"uparw		^[ O A\n"
		"uparw		^[ [ A\n"
		"upslide		^W\n"
		"yankpop		^[ Y\n"
		"yankpop		^[ y\n"
		"\n"
		":prompt\n"
		":inherit main\n"
		"abort		^C\n"
		"complete	^I\n"
		"\n"
		":menu\n"
		":inherit windows\n"
		"abort		^[ ^[\n"
		"backsmenu	^?\n"
		"backsmenu	^H\n"
		"bofmenu		^Q R\n"
		"bofmenu		^Q ^R\n"
		"bofmenu		^Q r\n"
		"bofmenu		^[ [ 1 ; 5 H\n"
		"bofmenu		^[ q r\n"
		"bolmenu		.kh\n"
		"bolmenu		^Q S\n"
		"bolmenu		^Q ^S\n"
		"bolmenu		^Q s\n"
		"bolmenu		^[ [ 1 ~\n"
		"bolmenu		^[ [ 7 ~\n"
		"bolmenu		^[ [ H\n"
		"bolmenu		^[ q s\n"
		"dnarwmenu	.kd\n"
		"dnarwmenu	^X\n"
		"dnarwmenu	^[ O B\n"
		"dnarwmenu	^[ [ B\n"
		"eof		^[ [ 1 ; 5 F\n"
		"eofmenu		^Q C\n"
		"eofmenu		^Q ^C\n"
		"eofmenu		^Q c\n"
		"eofmenu		^[ q c\n"
		"eolmenu		.@7\n"
		"eolmenu		.kH\n"
		"eolmenu		^Q D\n"
		"eolmenu		^Q ^D\n"
		"eolmenu		^Q d\n"
		"eolmenu		^[ [ 4 ~\n"
		"eolmenu		^[ [ 8 ~\n"
		"eolmenu		^[ [ F\n"
		"eolmenu		^[ q d\n"
		"ltarwmenu	.kl\n"
		"ltarwmenu	^S\n"
		"ltarwmenu	^[ O D\n"
		"ltarwmenu	^[ [ D\n"
		"ltarwmenu	^[ s\n"
		"rtarwmenu	.kr\n"
		"rtarwmenu	^D\n"
		"rtarwmenu	^[ O C\n"
		"rtarwmenu	^[ [ C\n"
		"rtn		SP\n"
		"rtn		^I\n"
		"rtn		^J\n"
		"uparwmenu	.ku\n"
		"uparwmenu	^E\n"
		"uparwmenu	^[ O A\n"
		"uparwmenu	^[ [ A\n"
		"\n"
		":query\n"
		":inherit windows\n"
		"\n"
		":querya\n"
		"type		^@ TO �\n"
		"\n"
		":querysr\n"
		"type		^@ TO �\n"
,	NULL
,	"@(#) $MirOS: contrib/code/jupp/builtins.c,v 1.2 2009/10/18 14:56:49 tg Exp $"
};
