
/*
    GEOS resource compiler

    by Maciej 'YTM/Elysium' Witkowiak

    see GEOSLib documentation for license info

*/

/*
 - make it work, then do it better
 - more or less comments? it was hard to code, should be even harder to
   understand =D
 - add loadable icons feature (binary - 63 bytes)
*/

/* - err, maybe free allocated memory, huh? (who cares, it's just a little prog...)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "grc.h"

/* common stuff */
//#include "cmdline.h"
#include "fname.h"
#include "abend.h"
#include "chartype.h"

void VLIRLinker(int argc, char *argv[]) {
FILE *outCVT, *input;
unsigned char buffer[BLOODY_BIG_BUFFER];
unsigned char vlirtabt[127];
unsigned char vlirtabs[127];
int i,j;
int bytes;
int blocks,rest;

	i=2;

	/* check if we know enough */

	if (argc<4)
    	    AbEnd("too few arguments, required [out] [cvthead] [vlir0] ...\n");

	/* first open and copy CVT header */

	outCVT = fopen(argv[i],"wb+");
	if (outCVT==NULL)
	    AbEnd("can't open output:%s\n",strerror(errno));

	++i;
	input = fopen(argv[i],"rb");
	if (input==NULL)
	    AbEnd("can't open input:%s\n",strerror(errno));

	bytes = fread(buffer,1,BLOODY_BIG_BUFFER,input);
	fclose(input);
	if (bytes!=508)
		AbEnd("%s is not a cvt header\n",argv[i]);

	fwrite(buffer,1,bytes,outCVT);

	/* now put 254 bytes of VLIR table, to update later */

	/* clear out things */
	memset(buffer,0,254);
	fwrite(buffer,1,254,outCVT);
	for (j=0;j!=126;j++) {
		vlirtabt[j]=0;
		vlirtabs[j]=0;
	}

	/* now read all VLIR chains, align to 254 bytes */

	++i;
	j=0;
	while (i!=argc) {
		if (strcmp(argv[i],"blank")==0) {
			vlirtabt[j]=0; vlirtabs[j]=0; }
		else if (strcmp(argv[i],"noexist")==0) {
			vlirtabt[j]=0; vlirtabs[j]=0xff; }
		else {
			memset(buffer,0,BLOODY_BIG_BUFFER);
                        input = fopen(argv[i],"rb");
			if (input==NULL)
				AbEnd("couldn't open %s:%s\n",argv[i],strerror(errno));
			bytes = fread(buffer,1,BLOODY_BIG_BUFFER,input);
			fclose(input);
			if (bytes==0)
				AbEnd("couldn't read %s:%s\n",argv[i],strerror(errno));
			blocks = bytes / 254;
			rest = bytes % 254;
			if (rest==0) {
			    rest = 255;
			    --blocks;
			} else {
			    ++rest;
			}
			vlirtabt[j]=blocks+1; vlirtabs[j]=rest;
			fwrite(buffer,1,(blocks+1)*254,outCVT);
		}
		++j;
		++i;
	}

	/* now rewind and update VLIR table */

	fflush(outCVT);
	fseek(outCVT,508,SEEK_SET);
	for (i=0;i!=127;i++) {
		fputc(vlirtabt[i],outCVT);
		fputc(vlirtabs[i],outCVT);
	}
	fclose(outCVT);
	exit(EXIT_SUCCESS);
}

void printCHeader (void) {

    fprintf(outputCFile, "\n/*\n\tThis file was generated by GEOS Resource Compiler\n"
	    "\n\tDO NOT EDIT! Any changes will be lost!\n"
	    "\n\tEdit proper resource file instead\n"
	    "\n*/\n\n");
}

void printSHeader (void) {

    fprintf(outputSFile, "\n;\n;\tThis file was generated by GEOS Resource Compiler\n;"
	    "\n;\tDO NOT EDIT! Any changes will be lost!\n;"
	    "\n;\tEdit proper resource file instead\n;"
	    "\n;\n\n");
}

void printVHeader (void) {
    fprintf(outputVFile, "\n#\n#\tThis file was generated by GEOS Resource Compiler\n#"
	    "\n#\tDO NOT EDIT! Any changes will be lost!\n#"
	    "\n#\tEdit proper resource file instead\n#"
	    "\n#\tLook at end of this file to find commandline that must be used\n"
	      "#\tto invoke ld65 and grc (as VLIR linker)\n#"
	    "\n#\n\n");
}

void openCFile (void) {
    if ((CFnum==0) && (forceFlag==0)) {
	/* test if file exists already and no forcing*/
	    if ((outputCFile = fopen (outputCName,"r"))!=0)
	        AbEnd("file %s already exists, aborting\n", outputCName);
	}
    if ((outputCFile = fopen (outputCName,outputCMode))==0)
	    AbEnd("can't open file %s for writing: %s\n",outputCName,strerror (errno));
    if (CFnum==0) {
		outputCMode[0]='a';
		printCHeader();
		CFnum++;
	}
}

void openSFile (void) {
    if ((SFnum==0) && (forceFlag==0)) {
	/* test if file exists already and no forcing*/
	    if ((outputSFile = fopen (outputSName,"r"))!=0)
	        AbEnd("file %s already exists, aborting\n", outputSName);
	}
    if ((outputSFile = fopen (outputSName,outputSMode))==0)
	     AbEnd("can't open file %s for writing: %s\n",outputSName,strerror (errno));
    if (SFnum==0) {
		outputSMode[0]='a';
		printSHeader();
		SFnum++;
	}
}

void openVFile (void) {
    if ((VFnum==0) && (forceFlag==0)) {
	/* test if file exists already and no forcing*/
	    if ((outputVFile = fopen (outputVName,"r"))!=0)
	        AbEnd("file %s already exists, aborting\n", outputVName);
	}
    if ((outputVFile = fopen (outputVName,outputVMode))==0)
	     AbEnd("can't open file %s for writting: %s\n",outputVName,strerror (errno));
    if (VFnum==0) {
		outputVMode[0]='a';
		printVHeader();
		VFnum++;
	}
}

void printUsage (void) {
    fprintf(stderr, "Usage: %s [options] file\n"
	    "Options:\n"
	    "\t-h, -?\t\tthis help\n"
	    "\t-f\t\tforce writting files\n"
	    "\t-o name\t\tname C output file\n"
	    "\t-s name\t\tname asm output file\n"
	    "\t-l name\t\tname ld65 config output file (for vlir)\n"
	    "Or as VLIR linker: %s -vlir output.cvt header [vlir0] ... [blank] ... [vlir_n]\n",
	    ProgName,ProgName);
}

int findToken (const char **tokenTbl, const char *token) {
/* takes as input table of tokens and token, returns position in table or -1 if not found */
int a=0;

    while (strlen(tokenTbl[a])!=0) {
		if (strcmp(tokenTbl[a],token)==0) break;
		a++;
    }
    if (strlen(tokenTbl[a])==0) a=-1;
    return a;
}

char *nextPhrase() {
    return strtok(NULL, "\"");
    }

char *nextWord() {
    return strtok(NULL, " ");
    }

void setLen (char *name, unsigned len) {
    if (strlen(name)>len)
    	name[len]='\0';
}

void fillOut (char *name, int len, char *filler) {
int a;
    setLen (name, len);
    fprintf(outputSFile, ".byte \"%s\"\n\t\t", name);
    a = len - strlen(name);
    if (a!=0) {
	fprintf(outputSFile, ".byte %s", filler);
        while (--a!=0) fprintf(outputSFile, ", %s", filler);
	fprintf(outputSFile, "\n\t\t");
	}
}

//char *bintos(unsigned char a, char *out) {
char *bintos(unsigned char a, char out[7]) {
int i=0;
    for (;i<8;i++) {
    out[7-i] = ((a & 1)==0) ? '0' : '1';
    a = a >> 1; };
    out[i]='\0';
return out;
}

int getNameSize (const char *word) {
/* count length of a word using BSW 9 font table */
int a=0, i=0;

    while (word[i]!='\0') {
	a+=(BSWTab[word[i]-31] - BSWTab[word[i]-32]); i++; }

    return a;
}

void DoMenu (void) {

int a, size, tmpsize, item=0;
char *token;
char namebuff[255]="";
struct menu myMenu;
struct menuitem *curItem, *newItem;

    openCFile();

    myMenu.name=nextWord();
    myMenu.left=atoi(nextWord());
    myMenu.top=atoi(nextWord());
    myMenu.type=nextWord();

    if (strcmp(nextWord(),"{")!=0) {
	AbEnd ("menu '%s' description has no opening bracket!\n", myMenu.name);
	};
    curItem=malloc(sizeof(struct menuitem));
    myMenu.item=curItem;
    do {
	token = nextWord();
	if (strcmp(token,"}")==0) break;
	if (token[strlen(token)-1]!='"') {
	    strcpy (namebuff, token);
	    do {
		token = nextWord();
		strcat (namebuff, " ");
		strcat (namebuff, token);
		} while (token[strlen(token)-1]!='"');
	    token = malloc(strlen(namebuff));
	    strcpy (token, namebuff);
	}
	curItem->name=token;
	curItem->type=nextWord();
	curItem->target=nextWord();
	newItem=malloc(sizeof(struct menuitem));
	curItem->next=newItem;
	curItem=newItem;
	item++;
	} while (strcmp(token,"}")!=0);
    if (item==0) AbEnd ("menu '%s' has 0 items!\n", myMenu.name);
    if (item>31) AbEnd ("menu '%s' has too many items!\n", myMenu.name);

    curItem->next=NULL;

    /* Count menu sizes */

    size=0;
    curItem=myMenu.item;
    if (strstr(myMenu.type,"HORIZONTAL")!=NULL) {
	/* menu is HORIZONTAL, ysize=15, sum xsize of all items +~8?*/
	    myMenu.bot=myMenu.top+15;
	    for (a=0;a!=item;a++) {
		size+=getNameSize(curItem->name);
		curItem=curItem->next;
		};
	} else {
	/* menu is VERTICAL, ysize=item*15, count largest xsize of all items +~8? */
	    myMenu.bot=myMenu.top+(14*item);
	    for (a=0;a!=item;a++) {
		tmpsize=getNameSize(curItem->name);
		size = (size > tmpsize) ? size : tmpsize;
		curItem=curItem->next;
		};
	};
    myMenu.right=myMenu.left+size-1;

    curItem=myMenu.item;
    for (a=0;a!=item;a++) {
	/* print prototype only if MENU_ACTION or DYN_SUB_MENU are present in type */
	if ((strstr(curItem->type, "MENU_ACTION")!=NULL) || (strstr(curItem->type, "DYN_SUB_MENU")!=NULL))
	    fprintf(outputCFile, "void %s (void);\n", curItem->target);
	curItem=curItem->next;
	}

    fprintf(outputCFile, "\nconst void %s = {\n\t(char)%i, (char)%i,\n\t(int)%i, (int)%i,\n\t"
	    "(char)(%i | %s),\n", myMenu.name, myMenu.top, myMenu.bot, myMenu.left,
	    myMenu.right, item, myMenu.type);

    curItem=myMenu.item;
    for (a=0;a!=item;a++) {
	fprintf(outputCFile, "\t%s, (char)%s, (int)", curItem->name, curItem->type);
	if ((strstr(curItem->type, "SUB_MENU")!=NULL) && (strstr(curItem->type, "DYN_SUB_MENU")==NULL))
	    fprintf(outputCFile, "&");
	fprintf(outputCFile, "%s,\n", curItem->target);
	curItem=curItem->next;
	}

    fprintf(outputCFile, "\t};\n\n");

    if (fclose (outputCFile)!=0)
	AbEnd("error closing %s: %s\n",outputCName,strerror (errno));
}

void DoHeader (void) {

time_t t;
struct tm *my_tm;

struct appheader myHead;
char *token;
char i1[9], i2[9], i3[9];
int a, b;

    openSFile();

    token = nextWord();

    a = findToken (hdrFTypes, token);

    if (a>1)
	   AbEnd("filetype '%s' is not supported yet\n", token);

    switch (a) {
	case 0: myHead.geostype = 6; break;
	case 1: myHead.geostype = 14; break;
	}

    myHead.dosname = nextPhrase();
    nextPhrase();
    myHead.classname = nextPhrase();
    nextPhrase();
    myHead.version = nextPhrase();

    /* put default values into myHead here */
    myHead.author = "cc65";
    myHead.info = "Program compiled with cc65 and GEOSLib.";
    myHead.dostype = 128+3;
    myHead.structure = 0;
    myHead.mode = 0;

    t = time(NULL);
    my_tm = localtime (&t);

    myHead.year = my_tm->tm_year;
    myHead.month = my_tm->tm_mon+1;
    myHead.day = my_tm->tm_mday;
    myHead.hour = my_tm->tm_hour;
    myHead.min = my_tm->tm_min;

    if (strcmp(nextWord(),"{")!=0)
		AbEnd ("header '%s' has no opening bracket!\n", myHead.dosname);

    do {
	token=nextWord();
	if (strcmp(token, "}")==0) break;
        switch (a = findToken (hdrFields, token)) {
		case -1:
		    AbEnd ("unknown field '%s' in header '%s'\n", token, myHead.dosname);
		    break;
		case 0: /* author */
		    myHead.author = nextPhrase(); break;
		case 1: /* info */
		    myHead.info = nextPhrase(); break;
		case 2:	/* date */
		    myHead.year = atoi(nextWord());
		    myHead.month = atoi(nextWord());
		    myHead.day = atoi(nextWord());
		    myHead.hour = atoi(nextWord());
		    myHead.min = atoi(nextWord());
		    break;
		case 3:	/* dostype */
		    switch (b = findToken (hdrDOSTp, nextWord())) {
			case -1:
			    AbEnd ("unknown dostype in header '%s'\n", myHead.dosname);
			    break;
			default:
			    myHead.dostype = b/2 + 128 + 1;
			    break;
		    }
		    break;
		case 4:	/* mode */
		    switch (b = findToken (hdrModes, nextWord())) {
			case -1:
			    AbEnd ("unknown mode in header '%s'\n", myHead.dosname);
			case 0:
			    myHead.mode = 0x40; break;
			case 1:
			    myHead.mode = 0x00; break;
			case 2:
			    myHead.mode = 0xc0; break;
			case 3:
			    myHead.mode = 0x80; break;
		    }
		    break;
		case 5: /* structure */
		    switch (b = findToken(hdrStructTp, nextWord())) {
			case -1:
			    AbEnd ("unknown structure type in header '%s'\n", myHead.dosname);
			case 0:
			case 1:
			    myHead.structure = 0; break;
			case 2:
			case 3:
			    myHead.structure = 1; break;
		    }
		    break;
        }

    } while (strcmp(token, "}")!=0);

    /* OK, all information is gathered, do flushout */

    fprintf(outputSFile,
	 "\t\t\t.segment \"HEADER\"\n\n\t\t.byte %i\n\t\t.word 0\n\t\t", myHead.dostype);

    fillOut(myHead.dosname,16,"$a0");

    fprintf(outputSFile,
	".word 0\n\t\t.byte %i\n\t\t.byte %i\n\t\t.byte %i, %i, %i, %i, %i\n\n\t\t"
	".word 0\n\t\t.byte \"PRG formatted GEOS file V1.0\"\n\n\t\t.res $c4\n\n\t\t"
	".byte 3, 21, 63 | $80\n\t\t",
	myHead.structure, myHead.geostype, myHead.year, myHead.month, myHead.day,
	myHead.hour, myHead.min);

    for (a=0;a!=63;a=a+3) {
	fprintf(outputSFile,
	     ".byte %%%s, %%%s, %%%s\n\t\t",
	     bintos(icon1[a], i1), bintos(icon1[a+1], i2), bintos(icon1[a+2], i3)); };

    fprintf(outputSFile,
	    "\n\t\t.byte %i, %i, %i\n\t\t.word $0400, $0400-1, $0400\n\n\t\t",
	    myHead.dostype, myHead.geostype, myHead.structure);

    fillOut(myHead.classname,12,"$20");

    fillOut(myHead.version,4,"0");

    fprintf(outputSFile,
	    ".byte 0, 0, 0\n\t\t.byte %i\n\n\t\t", myHead.mode);

    setLen(myHead.author,62);
    fprintf(outputSFile,
	    ".byte \"%s\"\n\t\t.byte 0\n\t\t.res (63-%i)\n\n\t\t",
	    myHead.author, (int) (strlen(myHead.author)+1));

    setLen(myHead.info, 95);
    fprintf(outputSFile,
	    ".byte \"%s\"\n\t\t.byte 0\n\t\t.res (96-%i)\n\n",
	    myHead.info, (int) (strlen(myHead.info)+1));

    if (fclose (outputSFile)!=0)
		AbEnd("error closing %s: %s\n",outputSName,strerror (errno));

}

void DoVLIR (void) {

char *token;
char *headname;
int i,numchains,vlirbase;
struct vlirentry {
    char *chainname;
    int exist;
};

struct vlirentry vlirtable[127];

    openVFile();

    headname = nextWord();

    vlirbase = strtol(nextWord(),NULL,0);

    if (strcmp(nextWord(),"{")!=0)
		AbEnd ("VLIR description has no opening bracket!\n");

    numchains=0;

    do {
	token=nextWord();
	if (strcmp(token, "}")==0) break;
	numchains++;
	if (numchains>127) {
	    AbEnd("Too many VLIR chains!\n");
	}
	vlirtable[numchains].chainname=token;

	/* for first chain - name header */
	if (numchains==1) {
	  fprintf(outputVFile,"MEMORY {\n\tHEADER: start = $204, size = 508, file = \"%s\";\n"
	    "\tVLIR0: start = $0400, size = $5C00, file = \"%s\";\n",headname,token);
	} else {
	/* for all other - segment */
	  /* ignore non-existing segments */
	  vlirtable[numchains].exist=1;
	  if ( (strcmp(token,"blank")==0) || (strcmp(token,"noexist")==0) ) {
	    vlirtable[numchains].exist=0;
	    fprintf(outputVFile,"#");
	  }
	  fprintf(outputVFile,"\tVLIR%i: start = $%x, size = $%x, file = \"%s\";\n",
	    numchains-1,vlirbase,0x5c00-vlirbase,token);
	}

    } while (strcmp(token, "}")!=0);
    fprintf(outputVFile,"}\n\n");

    if (numchains==0) {
	AbEnd("There must be at least one VLIR chain.\n");
    };

    /* now put segments info */
    fprintf(outputVFile,"SEGMENTS {\n\tHEADER: load = HEADER, type = ro;\n"
	"\tCODE: load = VLIR0, type = ro;\n"
	"\tRODATA: load = VLIR0, type = ro;\n"
	"\tDATA: load = VLIR0, type = rw;\n"
	"\tBSS: load = VLIR0, type = bss, define = yes;\n\n");

    for (i=2;i<=numchains;i++) {
	if (vlirtable[i].exist==0) {
	    fprintf(outputVFile,"#");
	}
	fprintf(outputVFile,"\tVLIR%i: load = VLIR%i, type = rw, define = yes;\n",i-1,i-1);
    }
    fprintf(outputVFile,"}\n");

    /* now put usage info */
    fprintf(outputVFile,"\n# ld65 -o output.cvt -C %s file1.o file2.o ...",outputVName);
    fprintf(outputVFile,"\n# grc -vlir outputname %s",headname);
    for (i=1;i<=numchains;i++) {
	fprintf(outputVFile," %s",vlirtable[i].chainname);
    }
    fprintf(outputVFile,"\n");

    if (fclose (outputVFile)!=0)
		AbEnd("error closing %s: %s\n",outputVName,strerror (errno));
}

char *filterInput (FILE *F, char *tbl) {
/* loads file into buffer filtering it out */
int a, prevchar=-1, i=0, bracket=0, quote=1;

    while (1) {
	a = getc(F);
	if ((a=='\n')||(a=='\015')) a = ' ';
	if (a==',') a = ' ';
	if (a=='\042') quote=!quote;
	if (quote) {
	    if ((a=='{')||(a=='(')) bracket++;
	    if ((a=='}')||(a==')')) bracket--;
	}
	if (a==EOF) { tbl[i]='\0'; realloc(tbl, i+1); break; };
	if (IsSpace(a)) {
	    if ((prevchar!=' ') && (prevchar!=-1)) { tbl[i++]=' '; prevchar=' '; }
	} else {
	    if (a==';' && quote) { do { a = getc (F); } while (a!='\n'); fseek(F,-1,SEEK_CUR); }
		else {
		    tbl[i++]=a; prevchar=a; }
	}
    }

    if (bracket!=0) AbEnd("there are unclosed brackets!\n");

    return tbl;
}

void processFile (const char *filename) {

FILE *F;

char *str;
char *token;

int head=0;	/* number of processed HEADER sections */
int vlir=0;	/* number of processed VLIR sections */

    if ((F = fopen (filename,"r"))==0)
		AbEnd("can't open file %s for reading: %s\n",filename,strerror (errno));

    str=filterInput(F, malloc(BLOODY_BIG_BUFFER));

    token = strtok (str," ");

    do {
        if (str!=NULL) {
	    switch (findToken (mainToken, token)) {

	    case 0: DoMenu(); break;
	    case 1:
		if (++head!=1) {
			AbEnd ("more than one HEADER section, aborting.\n");
		    } else {
			DoHeader();
		    }
		break;
	    case 2: break;	/* icon not implemented yet */
	    case 3: break;	/* dialog not implemented yet */
	    case 4:
		if (++vlir!=1) {
			AbEnd ("more than one VLIR section, aborting.\n");
		} else {
			DoVLIR();
		}
		break;
	    default: AbEnd ("unknown section %s.\n",token); break;
	    }
	}
	token = nextWord();
    } while (token!=NULL);
}

int main(int argc, char *argv[]) {

int ffile=0, i=1;

    ProgName = argv[0];
    while (i < argc) {
	const char *arg = argv[i];
	if (arg[0] == '-') {
	    switch (arg[1]) {
		case 'f':
		    forceFlag=1;
		    break;
		case 'o':
		    outputCName=argv[++i];
		    break;
		case 's':
		    outputSName=argv[++i];
		    break;
		case 'l':
		    outputVName=argv[++i];
		    break;
		case 'h':
		case '?':
		    printUsage();
		    exit (EXIT_SUCCESS);
		    break;
		case 'v':
		    if (strcmp(arg,"-vlir")==0) {
			VLIRLinker(argc,argv);
			exit (EXIT_SUCCESS);
			break;
		    } else {
			AbEnd("unknown option %s\n",arg);
			break;
		    }
		default: AbEnd("unknown option %s\n",arg);
	    }
	} else {
	    ffile++;

		if (outputCName==NULL)
			outputCName = MakeFilename(arg,".h");
		if (outputSName==NULL)
			outputSName = MakeFilename(arg,".s");
		if (outputVName==NULL)
			outputVName = MakeFilename(arg,".cfg");

	    processFile(arg);

	    }
	i++;
	}
    if (ffile==0) AbEnd("no input file\n");

    return EXIT_SUCCESS;
}
