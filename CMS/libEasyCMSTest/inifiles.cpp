/*******************************************************

inifiles.c
ini file read/write functions
used by linux programs

********************************************************/
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#define bzero(a, b) memset((a), 0, (b))
#endif

#include <stdlib.h>

#include "inifiles.h"
#define  MAXSTRLEN 1024

struct stringlist
{
  char string[MAXSTRLEN];
  struct stringlist *next;
};

string IniReadString (const char *section, const char *indent, const char *defaultresult, const char *inifilename)
{
  FILE *fp;
  struct stringlist *head, *node, *p, *t;
  char tempstr[MAXSTRLEN];
  char sectionstr[MAXSTRLEN];
  char result[MAXSTRLEN];
  int i, n, len;
  int sectionfinded;
  bzero (sectionstr, MAXSTRLEN);
  sprintf (sectionstr, "[%s]", section);
  fp = fopen (inifilename, "rt");
  if (fp == NULL)
    {
      strcpy (result, defaultresult);
      return string(result);
    }
  head = (struct stringlist *) malloc (sizeof (struct stringlist));
  p = head;
  bzero (tempstr, MAXSTRLEN);
  while (fgets (tempstr, MAXSTRLEN, fp) != NULL)
    {
      node = (struct stringlist *) malloc (sizeof (struct stringlist));
      node->next = NULL;
      bzero (node->string, MAXSTRLEN);
      strcpy (node->string, tempstr);
      p->next = node;
      p = p->next;
      bzero (tempstr, MAXSTRLEN);
    }
  fclose (fp);
  p = head;
  while (p->next != NULL)
    {
      t = p->next;
      len = strlen (t->string);
      bzero (tempstr, MAXSTRLEN);
      n = 0;
      for (i = 0; i < len; i++)
	{
	  if ((t->string[i] != '\r') && (t->string[i] != '\n')
	      && (t->string[i] != ' '))
	    {
	      tempstr[n] = t->string[i];
	      n++;
	    }
	}
      if (strlen (tempstr) == 0)
	{
	  p->next = t->next;
	  free (t);
	}
      else
	{
	  bzero (t->string, MAXSTRLEN);
	  strcpy (t->string, tempstr);
	  p = p->next;
	}
    }
  p = head;
  sectionfinded = 0;
  while (p->next != NULL)
    {
      p = p->next;
      if (sectionfinded == 0)
	{
	  if (strcmp (p->string, sectionstr) == 0)
	    sectionfinded = 1;
	}
      else
	{
	  if (p->string[0] == '[')
	    {
	      strcpy (result, defaultresult);
	      goto end;
	    }
	  else
	    {
	      if (strncmp (p->string, indent, strlen (indent)) == 0)
		{
		  if (p->string[strlen (indent)] == '=')
		    {
		      strncpy (result, p->string + strlen (indent) + 1,
			       MAXSTRLEN);
		      goto end;
		    }
		}
	    }
	}
    }
  strcpy (result, defaultresult);
end:while (head->next != NULL)
    {
      t = head->next;
      head->next = t->next;
      free (t);
    }
  free (head);
  return string(result);
}

int
IniWriteString (const char *section, const char *indent, const char *value,
		const char *inifilename)
{
  FILE *fp;
  struct stringlist *head, *node, *p, *t;
  char tempstr[MAXSTRLEN];
  char sectionstr[MAXSTRLEN];
  int i, n, len;
  int sectionfinded;
  bzero (sectionstr, MAXSTRLEN);
  sprintf (sectionstr, "[%s]", section);
  fp = fopen (inifilename, "rt");
  if (fp == NULL)
    {
      fp = fopen (inifilename, "wt");
      fputs (sectionstr, fp);
      fputs ("\r\n", fp);
      bzero (tempstr, MAXSTRLEN);
      sprintf (tempstr, "%s=%s", indent, value);
      fputs (tempstr, fp);
      fputs ("\r\n", fp);
      fclose (fp);
      return 0;
    }
  head = (struct stringlist *) malloc (sizeof (struct stringlist));
  p = head;
  while (!feof (fp))
    {
      node = (struct stringlist *) malloc (sizeof (struct stringlist));
      node->next = NULL;
      bzero (node->string, MAXSTRLEN);
      fgets (node->string, MAXSTRLEN, fp);
      p->next = node;
      p = p->next;
    }
  fclose (fp);
  p = head;
  while (p->next != NULL)
    {
      t = p->next;
      len = strlen (t->string);
      bzero (tempstr, MAXSTRLEN);
      n = 0;
      for (i = 0; i < len; i++)
	{
	  if ((t->string[i] != '\r') && (t->string[i] != '\n')
	      && (t->string[i] != ' '))
	    {
	      tempstr[n] = t->string[i];
	      n++;
	    }
	}
      if (strlen (tempstr) == 0)
	{
	  p->next = t->next;
	  free (t);
	}
      else
	{
	  bzero (t->string, MAXSTRLEN);
	  strcpy (t->string, tempstr);
	  p = p->next;
	}
    }
  p = head;
  sectionfinded = 0;
  while (p->next != NULL)
    {
      t = p;
      p = p->next;
      if (sectionfinded == 0)
	{
	  if (strcmp (p->string, sectionstr) == 0)
	    {
	      sectionfinded = 1;
	      node =
		(struct stringlist *) malloc (sizeof (struct stringlist));
	      bzero (node->string, MAXSTRLEN);
	      sprintf (node->string, "%s=%s", indent, value);
	      node->next = p->next;
	      p->next = node;
	      p = p->next;
	    }
	}
      else
	{
	  if (p->string[0] == '[')
	    {
	      goto end;
	    }
	  else
	    {
	      if (strncmp (p->string, indent, strlen (indent)) == 0)
		{
		  if (p->string[strlen (indent)] == '=')
		    {
		      node = p;
		      t->next = p->next;
		      free (node);
		      goto end;
		    }
		}
	    }
	}
    }
end:
  fp = fopen (inifilename, "wt");
  p = head;
  while (p->next != NULL)
    {
      p = p->next;
      fputs (p->string, fp);
      fputs ("\r\n", fp);
    }
  if (sectionfinded == 0)
    {
      fputs (sectionstr, fp);
      fputs ("\r\n", fp);
      bzero (tempstr, MAXSTRLEN);
      sprintf (tempstr, "%s=%s", indent, value);
      fputs (tempstr, fp);
      fputs ("\r\n", fp);
    }
  fclose (fp);
  while (head->next != NULL)
    {
      t = head->next;
      head->next = t->next;
      free (t);
    }
  free (head);
  return 0;
}

int
IniReadInteger (const char *section, const char *indent, int defaultresult,
		const char *inifilename)
{
  int i, len;
  string str = IniReadString (section, indent, "NULL", inifilename);
  if (str.compare("NULL") == 0)
    {
      return defaultresult;
    }
  len = str.length();
  if (len == 0)
{
	return defaultresult;
}
  for (i = 0; i < len; i++)
    {
      if ((str.at(i) < '0') || (str.at(i) > '9'))
	{
	  return defaultresult;
	}
    }
  return atoi(str.c_str());
}

int
IniWriteInteger (const char *section, const char *indent, int value,
		 char *inifilename)
{
  char str[20];
  memset (str, 0, 20);
  sprintf (str, "%d", value);
  IniWriteString (section, indent, str, inifilename);
  return 0;
}
