#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "printline.h"

#define DIR_LENGTH	512


void create_html(char *out) {
	char dir[DIR_LENGTH];
	char name[256];
	FILE *fp_temp;
	FILE *fp_html;

	sprintf(name, "%s.html", out);
	fp_temp = fopen(name, "r");
	if (fp_temp) {
	   fprintf(stderr, "HTML file `%s' already exists, exiting...\n", name);
	   exit(-1);
	}

	fp_html = fopen(name, "w");
	if (!fp_html) {
	   fprintf(stderr, "Cannot create HTML file `%s', exiting...\n", name);
	   exit(-1);
	}

	fprintf(fp_html, "<html>\n\n");

	if (!getcwd(dir, DIR_LENGTH)) {
	   fprintf(stderr, "Error while creating HTML file: cannot get current directory pathname, exiting...\n");
	   fprintf(stderr, "If you cannot determine the problem, contact `ericro@ece.ncsu.edu'.\n");
	   exit(-1);
	}

	fprintf(fp_html, "Click <a href=\"%s/%s\">HERE</a> for scope.\n\n",
				dir, out);

	fprintf(fp_html, "</html>\n");
	fclose(fp_html);
}


int main(int argc, char *argv[]) {
	FILE *fp_in;
	FILE *fp_out;

	if (argc != 3) {
	   fprintf(stderr, "Usage: scope <input-file> <output-file>\n");
	   exit(-1);
	}
	else {
	   fp_in = fopen(argv[1], "r");
	   if (!fp_in) {
	      fprintf(stderr, "Cannot open input file `%s', exiting...\n",
			argv[1]);
	      exit(-1);
	   }

	   FILE *fp_temp = fopen(argv[2], "r");
	   if (fp_temp) {
	      fprintf(stderr, "Output file `%s' already exists, exiting...\n",
			argv[2]);
	      exit(-1);
	   }

	   fp_out = fopen(argv[2], "w");
	   if (!fp_out) {
	      fprintf(stderr, "Cannot create output file `%s', exiting...\n",
			argv[2]);
	      exit(-1);
	   }
	}

	printline PL(fp_out);

	char line[512];
	while (fgets(line, 512, fp_in)) {
	   if (line[0] != '#')	// comments are preceded by '#' in first character
	      PL.print(line);
	}

	fclose(fp_in);
	fclose(fp_out);


	// Create an html web page for viewing output file with scroll bars.
	//create_html(argv[2]);
}
