/* more.c */

/* A program to display the contents of a text file, one screenful at
   a time */

/* Copyright (C) 1994-2002 Jim Hall <jhall@freedos.org> */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  See the file 'COPYING' for details.

*/

/*
  new in version 4.0 by tom:
	
  fixed hang on CtrlBreak (ignored) (bug 1609)
  is now able to survive CTTY COM1
  handles screensize dynamically (bug #1639)  
  expands tabs (bug 1639)
  executable size reduced by factor 2
	
*/

#if 1
int Xprintf(const char * fmt, ...);
int Xsprintf(char *, const char * fmt, ...);
#define PRINTF Xprintf
#define SPRINTF Xsprintf
#else
#include <stdio.h>
#define PRINTF printf
#define SPRINTF sprintf
#endif
#include <stdlib.h>			/* for _splitpath, _makepath */
#include <dos.h>			/* for findfirst, findnext */
#include <bios.h>			/* for _bios_keybrd - see keypress() */
#include <fcntl.h>

#include "../kitten/kitten.h"		/* Cats message library */

#if defined(__TURBOC__)
#include <dir.h>			/* for findfirst, findnext */

#ifndef _MAX_DRIVE	/* TC 2.01, TC++ 1.0 */
#define _MAX_DRIVE MAXDRIVE
#define _MAX_DIR   MAXDIR
#define _MAX_FNAME MAXPATH
#define _MAX_EXT   MAXEXT
#define _MAX_PATH  MAXPATH
#define _makepath  fnmerge
#define _splitpath fnsplit
#endif

#else /* not Turbo C */
#include <unistd.h>
#include <conio.h> 			/* for getch - see keypress() */

/* redefine struct name */
#define ffblk find_t
/* rename one of the member of that struct */
#define ff_name name

#define findfirst(pattern,buf,attrib) _dos_findfirst((pattern), (attrib)	\
	, (struct find_t*)(buf))
#define findnext(buf) _dos_findnext((struct find_t*)(buf))
#endif

#if defined(__GNUC__)
#include <libi86/stdlib.h>
#define far __far
#endif

/* Symbolic constants */

#define CTRL_C 3			/* control-c */


unsigned LINES   = 25;			/* lines per screen */
unsigned COLS    = 80;			/* columns per screen */
unsigned TABSIZE = 4;			/* default tabsize for tom */

void GetScreenSize(void)
{
  unsigned char  bios_screencols = *(unsigned char  far *)MK_FP(0x40,0x4a);
  unsigned short bios_screensize = *(unsigned short far *)MK_FP(0x40,0x4c);
    
  if (bios_screencols != 0)
    {
      COLS = bios_screencols;
    	
      if (bios_screensize != 0)
	{
	  LINES= bios_screensize/bios_screencols/2;
	}
            
      /* PRINTF("BIOS screen size = %u/%u\n",LINES,COLS);    */
    }   

}


/* Function prototypes */

int more (int pfile, const char *descr, const char *prompt);
unsigned keypress (void);
void usage (nl_catd cat);

int prompt_for_more(char *filename);


int dos_dup(int handle)
{
  union REGS r;
  r.h.ah = 0x45;	/* dosdup */
  r.x.bx = handle;
  intdos(&r,&r);
  /* PRINTF("dos dup new %x - %x %x\n",r.x.ax,r.x.cflag); */
  
  if (r.x.cflag) 
    return -1;
    
  return r.x.ax;      
}

int dos_dup2(int handle,int duphandle)
{
  union REGS r;
  r.h.ah = 0x46;	/* dosdup2 */
  r.x.bx = handle;
  r.x.cx = duphandle;
  intdos(&r,&r);
  /* PRINTF("dos2 dup new %x - %x %x\n",r.x.ax,r.x.cflag); */
  
  if (r.x.cflag) 
    return r.x.ax;
    
  return 0;      
}


/* Main program */




int
main (int argc, char **argv)
{
  char *s, *prompt;
  char drive[_MAX_DRIVE];		/* for _splitpath, _makepath */
  char dir[_MAX_DIR];			/* for _splitpath, _makepath */
  char file[_MAX_FNAME];		/* for _splitpath, _makepath */
  char ext[_MAX_EXT];			/* for _splitpath, _makepath */
  char fullpath[_MAX_PATH];		/* the full path after _makepath */

  int i;
  int next;				/* return value of more() function */
  int done;				/* flag for findfirst, findnext */
  int exitval;				/* exit value */

  int pfile;				/* file handle */
  struct ffblk ffblk;			/* findfirst, findnext block */
  nl_catd cat;				/* message catalog */
  int stdinhandle;
  int someFileFound;
  int more_prompt_needed = 0;


  GetScreenSize();

  /* Open the message catalog */

  cat = catopen ("more", 0);
  prompt = catgets (cat, 2, 0, "More");

  /* Evaluate the files on the command line */


  /* we want to use DOS to get characters from STDIN.
     STDIN MAY have been redirected, which is not acceptable to more
     
     however, we want to be able to work with CTTY COM1
     so using the BIOS isn't an option as well.
     
     so we duplicate STDIN to a new handle,
     and duplicate STDERR (possibly COM1) to STDIN
     
     then STDIN is available for character input again
  */

  stdinhandle = dos_dup(0);
  if (stdinhandle < 0)
    {
      PRINTF("??\n");
      exit(1);
    }
	
  close(0);
	
  if (dos_dup2(2,0))
    {
      PRINTF("???\n");
      exit(1);   
    }
	    
  for (i = 1; i < argc; i++)
    {
      /* Check if we are asking for help */
	
      if (argv[i][0] != '/')
	{
	  continue;
	}
      /* This is a flag character */

      /* Add any code here that deals with special flag chars */

      switch (argv[i][1])
	{
	case '?':
	case 'H':
	case 'h':
	  /* print usage and quit */

	  s = catgets (cat, 0, 0, "Display the contents of a text file one screen at a time");
	  PRINTF ("MORE: %s\n", s);
	  usage (cat);
	  catclose (cat);
	  exit (0);
	  break;
	      
	case 't':
	case 'T':
	  if ( argv[i][2] < '1' || argv[i][2] > '9')
	    {
	      PRINTF("MORE:%s\n",
		     catgets(cat,1,3,"option /Tabs must be /T1..9 (default 4)\n"));
	    			
	      exit(1);	
	    }
	  TABSIZE = argv[i][2] - '0';
	  break;	    		
	      

	default:
	  /* Not a recognized option */

	  s = catgets (cat, 1, 0, "Not a recognized option");
	  PRINTF ("MORE: %s: %s\n", argv[i], s);
	  usage (cat);
	  catclose (cat);
	  exit (1);
	  break;
	} /* switch */
    } 
    
    
  someFileFound = 0;	
  for (i = 1; i < argc; i++)
    {
      /* Check if we are asking for help */

      if (argv[i][0] == '/')	/* arguments have been handled already */
      	{
	  continue;
      	}
      
      someFileFound = 1;	

      /* Assume a file.  Open it and display it. */

      /* Note that findfirst, findnext only return the base file
         name in ff_name, and not the path.  The search still
         works with a path, but you don't get it back.  So we
         have to preserve the full path to the file using
         _splitpath and _makepath. */

      _splitpath (argv[i], drive, dir, file, ext);
      done = findfirst (argv[i], &ffblk, 0);

      if (done)
	{
	  /* We were not able to find a file. Display a message and
	     set the exit value. */
	
	  s = catgets (cat, 1, 1, "No such file");
	  PRINTF ( "MORE: %s: %s\n", argv[i], s);
	  exit(1);
	}

      while (!done)
	{
	  /* We have found a file, so open it and display it.  Set
	     the exit value to 'successful' */
	     
	  if (more_prompt_needed)
	    {
	      next = prompt_for_more(fullpath);
	      /* Find next file */
	      /* but only if user did not press 'Q' during more() */
	
	      if (next == 0)
		{
		  exit(0);
		}
	    }  

	  _makepath (fullpath, drive, dir, ffblk.ff_name, "");
	  pfile = open (fullpath, O_RDONLY);

	  if (pfile < 0)
	    {
	      s = catgets (cat, 1, 2, "Cannot open file");
	      PRINTF ( "MORE: %s: %s\n", ffblk.ff_name, s);
	      exit(1);
	    }
	 
	  more_prompt_needed = 1; 
	  
	  more (pfile, ffblk.ff_name, prompt);
	  close (pfile);
	  exitval = 0;


	  done = findnext (&ffblk);
	} /* while */
    } /* for */
    

  if (!someFileFound)		/* nothing done - use STDIN */
    {
      /* Process the standard input.  Return from more() is not used. */

      s = catgets (cat, 2, 1, "<STDIN>");
      more (stdinhandle, s, prompt);
      exitval = 0;
    }
    

  /* Done */

  catclose (cat);

  return exitval;
}

/* Since we can show more than one file at a time, we need to
   display a prompt when we are done with a file. */

int
prompt_for_more(char *descr)
{
  int key; 
  
  PRINTF( "-- %s (EOF) --", descr);
  key = keypress();
  PRINTF("\n");

  /* Do we quit or move to next file? */

  switch (key)
    {
    case CTRL_C:
      PRINTF( "^C");
    case 'q':
    case 'Q':
      return (0);

    case 'n':
    case 'N':
      return (1);
    } /* switch */
  return 1; /* next file !! */
}


/* A function to display the contents of a text file, one screenful
   at a time. */

/* Returns: 0=quit program immediately, 1=next file
   -1(reserved)=previous file (not currently used) */

int
more (int pfile, const char *descr, const char *prompt)
{
  char ch;				/* the character read from file    */
  int key;				/* the key pressed by user         */
  int nchars = 0;			/* no. of chars printed per line   */
  int nlines = 0;			/* no. of lines printed per screen */
  unsigned linecount = 0;

  while (read (pfile,&ch,1) > 0)
    {
      if (ch != '\t')
	{
	  PRINTF("%c",ch);
	  nchars++;
        }
      else
	do {                /* tab expansion */
	  PRINTF(" ");
	  nchars++;
	} while ( nchars < COLS && nchars % TABSIZE );


      /* Determine if we have caused a new line to be printed.
         Printing a CR/LF will do it, as will a screen wrap. */
         

      if (ch == '\n')
	linecount++;

      if ((ch == '\n') || (nchars >= COLS))
	{
	  nlines++; 
	  nchars = 0;

	  if (nlines == (LINES-1))
	    {
	      /* We have met the printable screen boundary.  Display
	         a prompt, and wait for a keypress before we show more. */

	      /* Since we don't necessarily know if the user is
	         redirecting to a file, always display on stderr. */

	      PRINTF( "-- %s -- %s (%u)--", prompt, descr, linecount);
	      key = keypress();
	      PRINTF("%40s\r","");
	      nlines = 0;

#if 0 /* debugging */
	      PRINTF( "key=%d,%c\n", key, key);
#endif /* debugging */

	      /* Do we quit or move to next file? */

	      switch (key)
		{
		case CTRL_C:
		  PRINTF( "^C");
		case 'q':
		case 'Q':
		  return (0);

		case 'n':
		case 'N':
		  return (1);
		} /* switch */
	    } /* if NLINES */
	} /* if COLS */
    } /* while */


  return 0;
}

/* Retrieve the next keypress */


void idle(void)
{
  union REGS r;
  r.x.ax = 0x1680;		/* application idle */
  int86(0x2f,&r,&r);			
}


/* Retrieve the next keypress */


#ifdef __TURBOC__
unsigned
keypress (void)
{
#define ZERO_FLAG 0x40
  union REGS r;
  
  for (;;)
    {
  
      r.h.ah = 0x06;	/* direct console input */
      r.h.dl = 0xff;
      intdos(&r,&r);
      /*	PRINTF("al %x flags %x\n",r.h.al,r.x.flags);*/
      if ((r.x.flags & ZERO_FLAG) == 0) /* character available */
	{
	  return (int)r.h.al;
	}
      idle();
    }
}

#else /* not Turbo C */

/* Retrieve the next keypress */

unsigned
keypress(void)
{
  for(;;)
  {
    if(kbhit())
      {
        return getch();
      }

    idle(); /* idle in environments that support it */
  }
}

#endif

/* A function to display the program's usage */

void
usage (nl_catd cat)
{
  char *s;
  
  (void)cat;

  /* Show version, copyright, and GNU GPL */

  PRINTF ("MORE 4.0, Copyright (C) 1994-2002 Jim Hall <jhall@freedos.org>\n");
  PRINTF ("  This is free software, and you are welcome to redistribute it\n");
  PRINTF ("  under the GNU GPL; see the COPYING file for details.\n");

  /* Show usage */

  s = catgets (cat, 0, 1, "Usage");
  PRINTF ("\n%s:\n", s);

  s = catgets (cat, 0, 2, "command");
  PRINTF ("  %s | MORE [/T4] \n", s);

  s = catgets (cat, 0, 3, "file");
  PRINTF ("  MORE [/T4] %s..\n", s);
  PRINTF ("  MORE [/T4] < %s\n", s);

  /* Show available keys, while viewing a file */

  s = catgets (cat, 0, 4, "Available keys");
  PRINTF ("\n%s:\n", s);

  s = catgets (cat, 0, 9, "Space");
  PRINTF ("  %s = ", s);
  s = catgets (cat, 0, 10, "Next page");
  PRINTF ("%s\n", s);

  s = catgets (cat, 0, 6, "Next file");
  PRINTF ("  N n = %s\n", s);

  s = catgets (cat, 0, 8, "Quit program");
  PRINTF ("  Q q = %s\n", s);
}
