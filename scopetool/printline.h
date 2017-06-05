#include <stdio.h>
#include <assert.h>

#define PRINT_HEADER	35
#define LEADING_SPACES	"                                    \t"
#define BLANK_CYCLE	"   "
#define NUM_STAGES	9
const char *stage_str[NUM_STAGES] = {"FE ", "DE ", "RN ", "RR ", "DI ", "IS ", "EX ", "WB ", "RT "};

#define ADDMAX	50

typedef struct {
   unsigned int cycle;
   unsigned int dur;
} stamp_t;


class printline {
	private:
		FILE *fp;
		unsigned int lineno;
		unsigned int min_cycle;
		unsigned int max_cycle;
		unsigned int base_cycle;

		void print_header() {
		   base_cycle = min_cycle;

		   fprintf(fp, LEADING_SPACES);
		   for (unsigned int i = min_cycle; i < max_cycle + ADDMAX; i++)
		      fprintf(fp, "%d  ", i/1000);
		   fprintf(fp, "\n");

		   fprintf(fp, LEADING_SPACES);
		   for (unsigned int i = min_cycle; i < max_cycle + ADDMAX; i++)
		      fprintf(fp, "%d  ", i/100 - ((i/1000) * 10));
		   fprintf(fp, "\n");

		   fprintf(fp, LEADING_SPACES);
		   for (unsigned int i = min_cycle; i < max_cycle + ADDMAX; i++)
		      fprintf(fp, "%d  ", i/10 - ((i/100) * 10));
		   fprintf(fp, "\n");

		   fprintf(fp, LEADING_SPACES);
		   for (unsigned int i = min_cycle; i < max_cycle + ADDMAX; i++)
		      fprintf(fp, "%d  ", i - ((i/10) * 10));
		   fprintf(fp, "\n");
		}

	public:
		printline(FILE *fp) {
		   this->fp = fp;
		   this->lineno = 0;
		   this->min_cycle = 0;
		   this->max_cycle = 100;

#if 0
		   if (n_cycles > 10000) {
		      fprintf(stderr, "You are asking to view more than 10000 cycles (%d), this is not supported, exiting...\n", n_cycles);
		      fprintf(stderr, "If you cannot determine the problem, contact `ericro@ece.ncsu.edu'.\n");
		      exit(-1);
		   }
#endif
		}

		~printline() {
		}

		void print(char *line) {
		   unsigned int scan;

		   unsigned int seq_no;
		   unsigned int fu_type;
		   int src1, src2, dst;
		   stamp_t stamps[NUM_STAGES];

		   unsigned int i, j, cycle;

		   // Print header every so often...
		   if ((lineno % PRINT_HEADER) == 0)
		      print_header();

		   scan = sscanf(line, "%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}",
			&seq_no,
			&fu_type,
			&src1, &src2,
			&dst,
			&stamps[0].cycle, &stamps[0].dur,
			&stamps[1].cycle, &stamps[1].dur,
			&stamps[2].cycle, &stamps[2].dur,
			&stamps[3].cycle, &stamps[3].dur,
			&stamps[4].cycle, &stamps[4].dur,
			&stamps[5].cycle, &stamps[5].dur,
			&stamps[6].cycle, &stamps[6].dur,
			&stamps[7].cycle, &stamps[7].dur,
			&stamps[8].cycle, &stamps[8].dur);
		   if (scan != 23) {
		      fprintf(stderr, "Error parsing line %d, exiting...\n",
					lineno);
		      fprintf(stderr, "Here's what I read in: `%s'.\n", line);
		      fprintf(stderr, "If you cannot determine the problem, contact `ericro@ncsu.edu'.\n");
		      exit(-1);
		   }

		   fprintf(fp, "%8d fu{%d} src{%3d,%3d} dst{%3d}\t",
				seq_no, fu_type, src1, src2, dst);

		   //////////////////////////////////////////////////////
		   // Check consistency of cycle/duration information.
		   //////////////////////////////////////////////////////
		   if (stamps[0].cycle < min_cycle) {
		      fprintf(stderr, "Line %d: `FE cycle (%d)' is inconsistent with (i.e., less than) previous fetch cycles, exiting...\n",
			lineno, stamps[0].cycle);
		      fprintf(stderr, "If you cannot determine the problem, contact `ericro@ncsu.edu'.\n");
		      exit(-1);
		   }
		   else {
		      min_cycle = stamps[0].cycle;
		   }

		   for (i = 0; i < (NUM_STAGES - 1); i++) {
		      if ((stamps[i].cycle + stamps[i].dur) != stamps[i+1].cycle) {
		         fprintf(stderr, "Line %d: `%s cycle (%d)' is inconsistent with `%s cycle (%d)' and `%s duration (%d)', exiting...\n",
			   lineno, stage_str[i+1], stamps[i+1].cycle, stage_str[i], stamps[i].cycle, stage_str[i], stamps[i].dur);
		         fprintf(stderr, "If you cannot determine the problem, contact `ericro@ncsu.edu'.\n");
		         exit(-1);
		      }
		   }

		   if (max_cycle < (stamps[NUM_STAGES-1].cycle + stamps[NUM_STAGES-1].dur))
		      max_cycle = (stamps[NUM_STAGES-1].cycle + stamps[NUM_STAGES-1].dur);

		   //////////////////////////
		   // Leading blank cycles.
		   //////////////////////////
		   assert(base_cycle <= stamps[0].cycle);
		   for (i = base_cycle; i < stamps[0].cycle; i++)
		      fprintf(fp, BLANK_CYCLE);

		   //////////////////////////
		   // Print stages
		   //////////////////////////
		   cycle = stamps[0].cycle;	// additional error checking
		   for (i = 0; i < NUM_STAGES; i++) {
		      for (j = 0; j < stamps[i].dur; j++)
		         fprintf(fp, stage_str[i]);

		      cycle += stamps[i].dur;
		      if (i < (NUM_STAGES - 1))
		         assert(cycle == stamps[i+1].cycle);
		   }

		   //////////////////////////
		   // Go to next line.
		   //////////////////////////
		   fprintf(fp, "\n");
		   lineno++;
		}
};
