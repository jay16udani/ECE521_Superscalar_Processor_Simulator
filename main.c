#include <stdio.h>
#include <stdlib.h>

//global file pointer for trace file
static FILE* fp = NULL;

//global variable declaration
int cycle_counter=0;
int inst_counter=0;
int rob_size,iq_size,width,cache_size,prefetch;
int i,tmp,k,tmp1,min=0;
int dispatch_counter,iq_counter,read_counter,rob_counter,decode_counter,rename_counter;
unsigned long long pc;

struct pipe
{
    int seq,type,src1,src2,dest,fe,de,rn,rr,di,is,ex,wb,rt_start,rt_end;

};

struct pipe pipeline[15000];
//pipeline registers
struct DE
{
    //unsigned long long pc;
    int type;
    int dest;
    int src1;
    int src2;

    int valid;
    int age;
};

struct RN
{
    //unsigned long long pc;
    int type;
    int dest;
    int src1_tag;
    int src2_tag;
    int src1_value;
  int src2_value;
    int src1_value_rob;
    int src2_value_rob;

    int valid;
    int age;
    int dst_tag;
};

struct RR
{
    //unsigned long long pc;
    int type;
    int dest;
    int src1_tag;
    int src2_tag;
    int src1_value;
    int src2_value;
    int valid;
    int src1_rdy;
    int src2_rdy;
    int age;
    int dst_tag;
    int src1_value_rob;
    int src2_value_rob;

};

struct DI
{
    int type;
    int dest;
    int src1_tag;
    int src2_tag;
    int src1_value;
    int src2_value;
    int valid;
    int src1_rdy;
    int src2_rdy;
    int age;
    int dst_tag;
    int src1_value_rob;
    int src2_value_rob;

};

struct IQ
{
    int valid;
    int dst_tag;
    int src1_rdy;
    int src1_tag;
    int src2_tag;
    int src1_value;
    int src2_value;
    int src2_rdy;
    int age;
    int type;
    int dest;
  int src1_value_rob;
    int src2_value_rob;

};

struct execute_list
{
    int type;
    int dest;
    int src1;
    int src2;
    int valid;
    int ttl;
    int dst_tag;
  int src1_value_rob;
    int src2_value_rob;
    int age;

};

struct WB
{
    int type;
    int dest;
    int src1;
    int src2;
    int valid;
    int dst_tag;
    int src1_value_rob;
    int src2_value_rob;
    int age;

};

struct rename_map_table
{
    int valid;
    int rob_tag;
};

struct reorder_buffer
{
    int number;
    int dst;
    int rdy;
    int exc;
    int mis;
    int valid;
    int age;

};

int head1=0;
int tail1=0;

//function declarations
void start_fetch(FILE *fp,struct DE decode[],int);
void start_decoding(struct DE decode[],struct RN rename[]);
void start_register_renaming(struct reorder_buffer *tail,struct reorder_buffer rob[],struct RN rename[],struct RR register_read[],struct rename_map_table rmt[]);
void start_register_read(struct RR register_read[],struct DI dispatch[],struct rename_map_table[],struct reorder_buffer rob[]);
void start_dispatch(struct DI dispatch[],struct IQ inst_queue[],struct rename_map_table rmt[],struct reorder_buffer rob[]);
void start_issue(struct IQ inst_queue[],struct execute_list execute[],struct rename_map_table rmt[],struct reorder_buffer rob[]);
void start_execution(struct execute_list execute[],struct WB writeback[],struct IQ inst_queue[],struct DI dispatch[],struct RR register_read[]);
void start_writeback(struct WB writeback[],struct reorder_buffer rob[]);
void start_retire(struct reorder_buffer *head,struct rename_map_table rmt[],struct reorder_buffer rob[]);
int advance_cycle(struct DE decode[],struct RN rename[],struct RR register_read[],struct DI dispatch[],struct IQ inst_queue[],struct execute_list execute[],struct WB writeback[],struct reorder_buffer rob[]);
void print_rob(struct reorder_buffer rob[]);

//main function
int main(int argc,char *argv[])
{

 //command line arguments significance
    rob_size=atoi(argv[1]);
    iq_size=atoi(argv[2]);
    width=atoi(argv[3]);
    cache_size=atoi(argv[4]);
    prefetch=atoi(argv[5]);

    //creating pipeline registers of appropriate size
    struct DE decode[width];
    struct RN rename[width];
    struct RR register_read[width];
    struct DI dispatch[width];
    struct IQ inst_queue[iq_size];
    struct execute_list execute[width*5];
    struct WB writeback[width*5];

    //creating a rename map table
    struct rename_map_table rmt[67];
    //creating a ROB
    struct reorder_buffer rob[rob_size];

    for(i=0;i<15000;i++)
    {
        pipeline[i].de=pipeline[i].fe=pipeline[i].rn=pipeline[i].rr=pipeline[i].di=pipeline[i].is=pipeline[i].wb=pipeline[i].ex=pipeline[i].rt_start=pipeline[i].rt_end=pipeline[i].seq=pipeline[i].type=pipeline[i].src1=pipeline[i].src2=0;
    }
    //initializing all the pipeline registers
    for(i=0;i<width;i++)
    {
        //decode[i].pc=0;rename[i].pc=0;register_read[i].pc=0;dispatch[i].pc=0;
        decode[i].dest=rename[i].dest=register_read[i].dest=dispatch[i].dest=0;
        decode[i].src1=rename[i].src1_value=0;register_read[i].src1_value=dispatch[i].src1_value=0;
        decode[i].src2=rename[i].src2_value=0;register_read[i].src2_value=dispatch[i].src2_value=0;
        decode[i].type=rename[i].type=register_read[i].type=dispatch[i].type=0;
        decode[i].valid=rename[i].valid=register_read[i].valid=dispatch[i].valid=0;
        decode[i].age=rename[i].age=register_read[i].age=dispatch[i].age=0;
        register_read[i].src1_rdy=dispatch[i].src1_rdy=0;
        register_read[i].src2_rdy=dispatch[i].src2_rdy=0;
        rename[i].src1_tag=register_read[i].src1_tag=dispatch[i].src1_tag=0;
        rename[i].src2_tag=register_read[i].src2_tag=dispatch[i].src2_tag=0;
        rename[i].dst_tag=register_read[i].dst_tag=dispatch[i].dst_tag=0;
        rename[i].src1_value_rob=register_read[i].src1_value_rob=dispatch[i].src1_value_rob=0;
        rename[i].src2_value_rob=register_read[i].src2_value_rob=dispatch[i].src2_value_rob=0;
    }

    for(i=0;i<iq_size;i++)
    {
        inst_queue[i].valid=0;
        inst_queue[i].dst_tag=0;
        inst_queue[i].src1_rdy=0;
        inst_queue[i].src1_tag=0;
        inst_queue[i].src2_tag=0;
        inst_queue[i].src2_rdy=0;
        inst_queue[i].src2_value=0;
        inst_queue[i].src1_value=0;
        inst_queue[i].age=0;
        inst_queue[i].type=0;
        inst_queue[i].dest=0;
        inst_queue[i].src1_value_rob=0;
        inst_queue[i].src2_value_rob=0;
    }

    for(i=0;i<width*5;i++)
    {
        writeback[i].dest=0;execute[i].dest=0;
        writeback[i].type=0;execute[i].type=0;
        writeback[i].src1=0;execute[i].src1=0;
        writeback[i].src2=0;execute[i].src2=0;
        writeback[i].valid=0;execute[i].valid=0;
        execute[i].ttl=0;
        writeback[i].dst_tag=0;execute[i].dst_tag=0;
        writeback[i].src1_value_rob=0;
        writeback[i].src2_value_rob=0;
        writeback[i].age=execute[i].age=0;
    }

    //initializing rename map table
    for(i=0;i<67;i++)
    {
        rmt[i].rob_tag=0;
        rmt[i].valid=0;
    }

    //initializing reorder buffer
    for(i=0;i<rob_size;i++)
    {
        rob[i].number=i;
        rob[i].dst=0;
        rob[i].exc=0;
        rob[i].mis=0;
        rob[i].rdy=0;
        rob[i].valid=0;
        rob[i].age=0;
    }

    //declaring head and tail pointer to point to ROB and initializing it
    struct reorder_buffer *head=&rob[0];
    struct reorder_buffer *tail=&rob[0];

    //reading the trace file
    fp=fopen(argv[6],"r");
    fseek(fp,0,SEEK_SET);
    if(fp==NULL)
    {
        printf("\nFile does not contain any data: Exiting\n");
        exit(1);
    }
    /*start the superscalar processor*/
    do
    {
      //      printf("At start:\n");print_rob(rob);printf("\n");
      start_retire(head,rmt,rob);
      //printf("After retire:\n");print_rob(rob);printf("\n");
        start_writeback(writeback,rob);
	//printf("After writeback\n");
	// print_rob(rob);printf("\n");
        start_execution(execute,writeback,inst_queue,dispatch,register_read);
	//printf("After exe:\n");print_rob(rob);printf("\n");
        start_issue(inst_queue,execute,rmt,rob);
	//printf("After issue:\n");print_rob(rob);printf("\n");
        start_dispatch(dispatch,inst_queue,rmt,rob);
	//printf("After dispatch:\n");print_rob(rob);printf("\n");
        start_register_read(register_read,dispatch,rmt,rob);
	//printf("After reading:\n");print_rob(rob);printf("\n");
        start_register_renaming(tail,rob,rename,register_read,rmt);
        //printf("After renaming:\n");print_rob(rob);printf("\n");
        start_decoding(decode,rename);
	//printf("After decoding:\n");print_rob(rob);printf("\n");
        start_fetch(fp,decode,cache_size);
	//	printf("After fetch:\n");print_rob(rob);printf("\n");
    }while(advance_cycle(decode,rename,register_read,dispatch,inst_queue,execute,writeback,rob)); //performs several functions

    fclose(fp);
    for(i=1;i<inst_counter+1;i++)
        printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",(pipeline[i].seq)-1,pipeline[i].type,pipeline[i].src1,pipeline[i].src2,pipeline[i].dest,pipeline[i].fe,(pipeline[i].de-pipeline[i].fe),pipeline[i].de,(pipeline[i].rn-pipeline[i].de),pipeline[i].rn,(pipeline[i].rr-pipeline[i].rn),pipeline[i].rr,(pipeline[i].di-pipeline[i].rr),pipeline[i].di,(pipeline[i].is-pipeline[i].di),pipeline[i].is,(pipeline[i].ex-pipeline[i].is),pipeline[i].ex,(pipeline[i].wb-pipeline[i].ex),pipeline[i].wb,(pipeline[i].rt_start-pipeline[i].wb),pipeline[i].rt_start,(pipeline[i].rt_end-pipeline[i].rt_start));
    printf("# === Simulator Command =========\n");
    printf("# ./sim_ds %s %s %s %s %s %s\n",argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE 	= %s\n",argv[1]);
    printf("# IQ_SIZE  	= %s\n",argv[2]);
    printf("# WIDTH    	= %s\n",argv[3]);
    printf("# CACHE_SIZE 	= %s\n",argv[4]);
    printf("# PREFETCHING	= %s\n",argv[5]);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count      = %d\n",inst_counter);
    printf("# Cycles                         = %d\n",cycle_counter);
    printf("# Instructions Per Cycle (IPC)   = %.2f\n",((float)inst_counter)/((float)cycle_counter));


    //print_rob(rob);
    return 0;
}
//(1) Function advances the simulator cycle.
//(2) When it becomes known that the pipeline is empty AND the trace is depleted, the function returns “false” to terminate the loop

int advance_cycle(struct DE decode[],struct RN rename[],struct RR register_read[],struct DI dispatch[],struct IQ inst_queue[],struct execute_list execute[],struct WB writeback[],struct reorder_buffer rob[])
{
    tmp=0;
    cycle_counter+=1;
    //printf("in advance cycle func\n");
    if(feof(fp))    //checks eof
    {
        for(i=0;i<width;i++)
        {
            //checks whether pipeline is empty
            if(decode[i].valid==1 || rename[i].valid==1 || register_read[i].valid==1 || dispatch[i].valid==1)
            {
                tmp=1;
                break;
            }
        }

        //if(tmp==0)
          //  printf("Decode empty\n");
        if(tmp==0)
	  {
            for(i=0;i<iq_size;i++)
            {
                if(inst_queue[i].valid==1)  //checks whether pipeline is empty
                {
                    tmp=1;  //if pipeline is not empty,then come out of the loop
                    break;
                }
            }

            //if(tmp==0)
             //   printf("Iq empty\n");
            if(tmp==0)
            {
                for(i=0;i<width*5;i++)
                {
                    if(execute[i].valid==1)    //checks whether pipeline is empty
                    {
                        tmp=1;
                        break;
                    }
                }
                //if(tmp==0)
                //   printf("EX empty\n");

                for(i=0;i<width;i++)
                {
                    if(writeback[i].valid==1)
                    {
                        tmp=1;
                        break;
                    }
                }

                //if(tmp==0)
                  //  printf("WB empty\n");

                if(tmp==0)
                {
                    for(i=0;i<rob_size;i++)
                    {
                        if(rob[i].rdy==1)  //checks whether pipeline is empty
                        {
                            tmp=1;
                            break;
                        }
                    }
                }
		if(tmp==0)
		  {
		    if(!(head1==tail1 && cycle_counter!=0))
		      {
			tmp=1;

		  }

		  }
	      }
	  }
        if(tmp==0)
            return 0; //since pipeline is empty and eof,therefore return false
        else
        {

	  //    printf("EOF reached Cycle number:%d\n\n\n\n",cycle_counter);
            //print_rob(rob);
            //cycle_counter+=1;
	    //	    if(cycle_counter==30)
	    //  return 0;
	    // else
                return 1;   //continue to run the simulator
        }
    }
    else
    {

      //        printf("Cycle number:%d\n\n\n\n",cycle_counter);
        //print_rob(rob);
        //cycle_counter+=1;
	//if(cycle_counter==30)
	//      return 0;
	// else
            return 1;   //continue to run the simulator

    }
}

//retire stage
// Retire up to WIDTH consecutive “ready” instructions from the head of
// the ROB.
void start_retire(struct reorder_buffer *head,struct rename_map_table rmt[],struct reorder_buffer rob[])
{
  //print_rob(rob);
//for fetching upto width instructions at a time from the header in ROB
    for(i=0;i<width;i++)
    {
        if((rob[head1].rdy)==1 && (rob[head1].dst)!=-1 && (rob[head1].valid)==1)   //if rdy bit is 1 and it contains a destinations register
        {
	  if((rmt[(rob[head1].dst)].rob_tag == (rob[head1].number)) && (rmt[(rob[head1].dst)].valid==1))    //if rmt rob tag matches the rob number, then clear the rmt entry and rob entry
            {
                rmt[(rob[head1].dst)].valid=0;
                rmt[(rob[head1].dst)].rob_tag=0;
            }

                //rob[head1].dst=0;
                rob[head1].exc=0;
                rob[head1].mis=0;
                rob[head1].rdy=0;
                rob[head1].valid=0;

                pipeline[rob[head1].age].rt_end=cycle_counter+1;
                //printf("Destination retired: %d\n",rob[head1].dst);
            if((rob[head1].number)==rob_size-1)   //check if head pointer reaches the end of ROB
                head1=0;                        //if yes, then point to start else point to next
            else
                head1++;
        }

        else if((rob[head1].dst)==-1 && (rob[head1].rdy)==1 && (rob[head1].valid)==1)    //condition if instruction does not have a destination register
        {
            rob[head1].exc=0;
            rob[head1].mis=0;
            rob[head1].rdy=0;
            rob[head1].valid=0;

            pipeline[rob[head1].age].rt_end=cycle_counter+1;
	    //printf("Destination retired: %d\n",rob[head1].dst);

            if(rob[head1].number==rob_size-1)   //check if head pointer reaches the end of ROB
                head1=0;                       //if yes, then point to start else point to next
            else
                head1++;
        }
        //if rdy bit is not equal to 1 then break out of the loop and do not check for any further instructions
        else
            break;
    }
    return;
}
//writeback stage
//Process the writeback bundle in WB: For each instruction in WB, mark
// the instruction as “ready” in its entry in the ROB.
void start_writeback(struct WB writeback[],struct reorder_buffer rob[])
{
    for(i=0;i<width*5;i++)
    {
      // printf("Writeback Bundle: %d\t%d\t%d\t%d\t%d\t%d\n",writeback[i].type,writeback[i].dest,writeback[i].src1,writeback[i].src2,writeback[i].valid,writeback[i].dst_tag);
      if(writeback[i].valid==1)   //if instruction is present in the pipeline
        {
            for(k=0;k<rob_size;k++) //checks for the matching instruction in the ROB by comparing the PC
            {
                if(writeback[i].dst_tag==rob[k].number && rob[k].valid==1 && rob[k].rdy==0)
                {
                  rob[k].rdy=1;        //makes the corresponding ready bit 1
                  //print_rob(rob);
                  writeback[i].valid=0;   //removes the instruction from the pipeline
                  rob[k].age=writeback[i].age;

                    pipeline[rob[k].age].rt_start=cycle_counter+1;

                    break;
                }
            }
        }
    }
    return;
}

// From the execute_list, check for instructions that are finishing
// execution this cycle, and:
// 1) Remove the instruction from the execute_list.
// 2) Add the instruction to WB.
// 3) Wakeup dependent instructions (set their source operand ready
// flags) in the IQ, DI (dispatch bundle), and RR (the register-read
// bundle).
void start_execution(struct execute_list execute[],struct WB writeback[],struct IQ inst_queue[],struct DI dispatch[],struct RR register_read[])
{

    for(i=0;i<width*5;i++)
    {
      //printf("execute bundle : %d %d %d %d timer: %d valid: %d \n",execute[i].type,execute[i].dest,execute[i].src1,execute[i].src2,execute[i].ttl,execute[i].valid);
        if(execute[i].valid==1)
        {
            execute[i].ttl-=1;  //if valid instruction present in execute, decrease the ttl counter

            if(execute[i].ttl==0) //checking if instruction in pipeline has finished executing
            {
                //adding the instruction to WB Bundle to an empty location
                for(k=0;k<width*5;k++)
                {
                    if(writeback[k].valid==0)
                    {
                        execute[i].valid=0;//removing instruction from pipeline

                        writeback[k].valid=1;
                        writeback[k].dest=execute[i].dest;
                        writeback[k].src1=execute[i].src1;
                        writeback[k].src2=execute[i].src2;
                        writeback[k].type=execute[i].type;
                        writeback[k].dst_tag=execute[i].dst_tag;

                        writeback[k].age=execute[i].age;


                        pipeline[writeback[k].age].wb=cycle_counter+1;
                       break;
                    }
                }

                //waking up the dependent instruction in IQ
                for(k=0;k<iq_size;k++)
                {
                    if(inst_queue[k].src1_tag==execute[i].dst_tag && inst_queue[k].src1_rdy==0 && inst_queue[k].valid==1)
                    {
                        inst_queue[k].src1_rdy=1;
                        //inst_queue[k].src1_tag=0;
                        //inst_queue[k].src1_value=execute[i].src1;
                    }

                    if(inst_queue[k].src2_tag==execute[i].dst_tag && inst_queue[k].src2_rdy==0 && inst_queue[k].valid==1)
                    {
                        inst_queue[k].src2_rdy=1;
                        //inst_queue[k].src2_tag=0;
                        //inst_queue[k].src2_value=execute[i].src2;
                    }
                }

                //waking up the dependent instruction in DI
                for(k=0;k<width;k++)
                {
                    if(dispatch[k].src1_tag==execute[i].dst_tag && dispatch[k].src1_rdy==0 && dispatch[k].valid==1)
                        {
                            dispatch[k].src1_rdy=1;
                          //  dispatch[k].src1_tag=0;
                           // dispatch[k].src1_value=execute[i].src1;
                        }

                    if(dispatch[k].src2_tag==execute[i].dst_tag && dispatch[k].src2_rdy==0 && dispatch[k].valid==1)
                        {
                            dispatch[k].src2_rdy=1;
                            //dispatch[k].src2_tag=0;
                            //dispatch[k].src2_value=execute[i].src2;
                        }
                }

                //waking up the dependent instruction in RR
                for(k=0;k<width;k++)
                {
                    if(register_read[k].src1_tag==execute[i].dst_tag && register_read[k].src1_rdy==0 && register_read[k].valid==1)
                    {
                        register_read[k].src1_rdy=1;
			//                        register_read[k].src1_tag=0;
                        //register_read[k].src1_value=execute[i].src1;
                    }

                    if(register_read[k].src2_tag==execute[i].dst_tag && register_read[k].src2_rdy==0 && register_read[k].valid==1)
                    {
                        register_read[k].src2_rdy=1;
                        //register_read[k].src2_tag=0;
                        //register_read[k].src2_value=execute[i].src2;
                    }
                }
            }
        }
    }
    return;
}

// Issue up to WIDTH oldest instructions from the IQ. (One approach to
// implement oldest-first issuing is to make multiple passes through
// the IQ, each time finding the next oldest ready instruction and then
// issuing it. One way to annotate the age of an instruction is to
// assign an incrementing sequence number to each instruction as it is
// fetched from the trace file.) // To issue an instruction:
// 1)  Remove the instruction from the IQ.
// 2)  Add the instruction to the execute_list. Set a timer for
//     the instruction in the execute_list that will allow you to
//     model its execution latency.
void start_issue(struct IQ inst_queue[],struct execute_list execute[],struct rename_map_table rmt[],struct reorder_buffer rob[])
{
    int m;
    iq_counter=0;
    k=0;

    for(i=0;i<iq_size;i++)
    {

      if((rmt[inst_queue[i].src1_value].valid==1) && (inst_queue[i].src1_value_rob) && (inst_queue[i].valid==1))
        {
            if(rob[inst_queue[i].src1_tag].rdy==1)
            {
	      //printf("rob tag lookup in rob table 1 %d",rmt[inst_queue[i].src1_value].rob_tag);
                inst_queue[i].src1_rdy=1;
            }
            /*else
                inst_queue[i].src1_rdy=0;*/
        }

    else if (inst_queue[i].valid==1)
      {
	//printf("rob tag lookup in rob table 2 %d",rmt[inst_queue[i].src1_value].rob_tag);
      inst_queue[i].src1_rdy=1;
      }
    }


    for(i=0;i<iq_size;i++)
    {
        if((rmt[inst_queue[i].src2_value].valid==1) && ((inst_queue[i].src2_value_rob)) &&(inst_queue[i].valid==1))
        {
            if(rob[inst_queue[i].src2_tag].rdy==1)
            {
                inst_queue[i].src2_rdy=1;
            }
            /*else
                inst_queue[i].src2_rdy=0;*/
        }
        else if (inst_queue[i].valid==1)
            inst_queue[i].src2_rdy=1;
    }

    for(i=0;i<iq_size;i++)
      {
        //printf("Issue bundle : type %d dest %d src1 %d src2 %d src1_rdy %d src2_rdy %d valid %d Age %d \n",inst_queue[i].type,inst_queue[i].dest,inst_queue[i].src1_value,inst_queue[i].src2_value,inst_queue[i].src1_rdy,inst_queue[i].src2_rdy,inst_queue[i].valid,inst_queue[i].age);
      }
    //count the number of instructions that are valid and are ready for execution in IQ bundle
    for(i=0;i<iq_size;i++)
    {
      //        printf("%d\n",iq_counter);
        if(inst_queue[i].valid==1 && inst_queue[i].src1_rdy==1 && inst_queue[i].src2_rdy==1)
        {
            iq_counter+=1;
        }
    }

    int iq_array[iq_counter],temp;  //create an array of that size(number of instructions that are valid and are ready for execution in IQ bundle)
    for(i=0;i<iq_size;i++)          //store all the valid ready elements(age parameter) in the array
    {
        if(inst_queue[i].valid==1 && inst_queue[i].src1_rdy==1 && inst_queue[i].src2_rdy==1)
        {
            iq_array[k]=inst_queue[i].age;
            k++;
        }
    }

    //arrange the ages in ascending order
    for(i=0;i<iq_counter;i++)
    {
        for(k=0;k<iq_counter-1;k++)
        {
            if(iq_array[k]>iq_array[k+1])
            {
                temp=iq_array[k];
                iq_array[k]=iq_array[k+1];
                iq_array[k+1]=temp;
            }
        }
    }
    //for(i=0;i<iq_counter;i++)
    // {
    //	printf("Array Contents after sorting: %d\t",iq_array[i]);
    // }
    //printf("\n");
    //if array size is greater than or equal to width,then issue width number of corresponding instruction from IQ
    if(iq_counter>=width)
    {
        for(i=0;i<width;i++)
        {
            for(k=0;k<iq_size;k++)  //finding the corresponding element in IQ
            {
                if(iq_array[i]==inst_queue[k].age && inst_queue[k].valid==1)
                {
                    break;
                }
            }

            inst_queue[k].valid=0;  //removing instruction from IQ

            //adding the instruction to EXE Bundle wherever space is present
            for(m=0;m<width*5;m++)
            {
                if(execute[m].valid==0)
                {
                    execute[m].dst_tag=inst_queue[k].dst_tag;
                    execute[m].dest=inst_queue[k].dest;
                    execute[m].src1=inst_queue[k].src1_value;
                    execute[m].src2=inst_queue[k].src2_value;
                    execute[m].type=inst_queue[k].type;
                    execute[m].valid=1;
					execute[m].src1_value_rob=inst_queue[k].src1_value_rob;
					execute[m].src2_value_rob=inst_queue[k].src2_value_rob;

					execute[m].age=inst_queue[k].age;

                    //setting ttl according to the latency of operation
                    if(inst_queue[k].type==0)
                    execute[m].ttl=1;
                    else if(inst_queue[k].type==1)
                    execute[m].ttl=2;
                    else if(inst_queue[k].type==2)
                    execute[m].ttl=5;

                    pipeline[execute[m].age].ex=cycle_counter+1;

                    break;
                }
            }
        }
    }
    //if array size is less than 'width', then issue all corresponding instructions from the IQ bundle
    else
    {
        for(i=0;i<iq_counter;i++)
        {
            for(k=0;k<iq_size;k++)  //finding the corresponding instruction
            {
                if(iq_array[i]==inst_queue[k].age && inst_queue[k].valid==1)
                {
                    break;
                }
            }

            inst_queue[k].valid=0;  //removing instruction from IQ

            //adding the instruction to EXE Bundle wherever space is present
            for(m=0;m<width*5;m++)
            {
                if(execute[m].valid==0)
                {
                    execute[m].dst_tag=inst_queue[k].dst_tag;
                    execute[m].dest=inst_queue[k].dest;
                    execute[m].src1=inst_queue[k].src1_value;
                    execute[m].src2=inst_queue[k].src2_value;
                    execute[m].type=inst_queue[k].type;
                    execute[m].src1_value_rob=inst_queue[k].src1_value_rob;
                    execute[m].src2_value_rob=inst_queue[k].src2_value_rob;
                    execute[m].valid=1;
                    execute[m].age=inst_queue[k].age;

                    //setting ttl according to the latency of operation
                    if(inst_queue[k].type==0)
                        execute[m].ttl=1;
                    else if(inst_queue[k].type==1)
                        execute[m].ttl=2;
                    else if(inst_queue[k].type==2)
                        execute[m].ttl=5;


                        pipeline[execute[m].age].ex=cycle_counter+1;

                    break;
                }
            }
        }
    }

    return;
}

// If DI contains a dispatch bundle:
// If the number of free IQ entries is less than the size of the
// dispatch bundle in DI, then do nothing. If the number of free IQ
// entries is greater than or equal to the size of the dispatch bundle
// in DI, then dispatch all instructions from DI to the IQ
void start_dispatch(struct DI dispatch[],struct IQ inst_queue[],struct rename_map_table rmt[],struct reorder_buffer rob[])
{
    dispatch_counter=0;
    iq_counter=0;

    for(i=0;i<width;i++)    //counts the number of instructions in dispatch bundle
    {
      //printf("Dispatch Bundle: Type:%d Dest:%d S1_tag:%d S2_tag:%d S1val:%d s2val:%d valid: %d s1rdy: %d s2rdy: %d Age: %d Dst_tag: %d\n",dispatch[i].type,dispatch[i].dest,dispatch[i].src1_tag,dispatch[i].src2_tag,dispatch[i].src1_value,dispatch[i].src2_value,dispatch[i].valid,dispatch[i].src1_rdy,dispatch[i].src2_rdy,dispatch[i].age,dispatch[i].dst_tag);
      if(dispatch[i].valid==1)
	dispatch_counter+=1;
    }

    for(i=0;i<iq_size;i++)  //counts the number of instructions in IQ
    {
        if(inst_queue[i].valid==0)
            iq_counter+=1;
    }
    ////check in rob for readiness for source 1
    for(i=0;i<width;i++)
    {
        if((rmt[dispatch[i].src1_value].valid==1) && ((dispatch[i].src1_value_rob)) && (dispatch[i].valid==1))
        {
            if(rob[dispatch[i].src1_tag].rdy==1)
            {
                dispatch[i].src1_rdy=1;
            }
            /*else
                inst_queue[i].src2_rdy=0;*/
        }
        else if ((dispatch[i].valid==1))
            dispatch[i].src1_rdy=1;
    }

    ////check in rob for readiness for source 2
    for(i=0;i<width;i++)
    {
        if((rmt[dispatch[i].src2_value].valid==1) && ((dispatch[i].src2_value_rob)) && (dispatch[i].valid==1))
        {
            if(rob[dispatch[i].src2_tag].rdy==1)
            {
                dispatch[i].src2_rdy=1;
            }
            /*else
                inst_queue[i].src2_rdy=0;*/
        }
        else if ((dispatch[i].valid==1))
            dispatch[i].src2_rdy=1;
    }


////check dispatch is not empty then proceed
    if(dispatch_counter>0)
    {
        if(iq_counter>=dispatch_counter)//move the entries only if vacancy in IQ is greater than or equal to number of instructions in DI bundle
        {
            for(i=0;i<width;i++)
            {
                if(dispatch[i].valid==1)
                {
                    for(k=0;k<iq_size;k++)  //finding a vacant place in IQ
                    {
		      // printf("Issue bundle seen by Dispatch : type %d dest %d src1 %d src2 %d src1_rdy %d src2_rdy %d valid %d Age %d \n",inst_queue[k].type,inst_queue[k].dest,inst_queue[k].src1_value,inst_queue[k].src2_value,inst_queue[k].src1_rdy,inst_queue[k].src2_rdy,inst_queue[k].valid,inst_queue[k].age);
                        if(inst_queue[k].valid==0)   //move only at a vacant place
                        {
                            dispatch[i].valid=0;    //remove from DI bundle

                            //move from DI to IQ
                            inst_queue[k].age=dispatch[i].age;
                            inst_queue[k].dest=dispatch[i].dest;
                            inst_queue[k].dst_tag=dispatch[i].dst_tag;
                            inst_queue[k].src1_rdy=dispatch[i].src1_rdy;
                            inst_queue[k].src1_tag=dispatch[i].src1_tag;
                            inst_queue[k].src1_value=dispatch[i].src1_value;
                            inst_queue[k].src2_rdy=dispatch[i].src2_rdy;
                            inst_queue[k].src2_tag=dispatch[i].src2_tag;
                            inst_queue[k].src2_value=dispatch[i].src2_value;
                            inst_queue[k].type=dispatch[i].type;
                            inst_queue[k].src1_value_rob=dispatch[i].src1_value_rob;
                            inst_queue[k].src2_value_rob=dispatch[i].src2_value_rob;
                            inst_queue[k].valid=1;

                            pipeline[inst_queue[k].age].is=cycle_counter+1;

                            break;
                        }
                    }
                }
            }
        }

    }

    return;
}

// If RR contains a register-read bundle:
// If DI is not empty (cannot accept a new dispatch bundle), then do
// nothing. If DI is empty (can accept a new dispatch bundle), then
// process (see below) the register-read bundle and advance it from RR
// to DI.
// How to process the register-read bundle:
// Since values are not explicitly modeled, the sole purpose of the
// Register Read stage is to ascertain the readiness of the renamed
// source operands. Apply your learning from the class lectures/notes
// on this topic.
// Also take care that producers in their last cycle of execution
// wakeup dependent operands not just in the IQ, but also in two other
// stages including RegRead()(this is required to avoid deadlock). See
// Execute() description above.
void start_register_read(struct RR register_read[],struct DI dispatch[],struct rename_map_table rmt[],struct reorder_buffer rob[])
{
    int counter=0,counter1=0;
    dispatch_counter=0;
    read_counter=0;

    for(i=0;i<width;i++)    //counts number of instructions in RR bundle
    {
        if(register_read[i].valid==1)
        read_counter+=1;
    }

    if(read_counter>0) //proceed only if the RR bundle is full with instructions
    {
        for(i=0;i<width;i++)    //counts the number of instructions in DI bundle
        {
            if(dispatch[i].valid==0)
                dispatch_counter+=1;
        }
        if(dispatch_counter==width)  //proceed only if DI bundle is empty
        {
            for(i=0;i<width;i++)
            {
                if(register_read[i].valid==1)
                {
                    if(register_read[i].src1_value!=-1) //checks the validity of first source register
                    {
                        if(rmt[register_read[i].src1_value].valid==1)   //if true then dependent instruction;make rdy bit 0
                        {
                            for(k=0;k<rob_size;k++)
                            {
                                if(rob[k].dst==register_read[i].src1_value && rob[k].rdy==1 && (rob[register_read[i].src1_tag].rdy==1))
                                {
                                    counter=1;
                                    break;
                                }
                            if(rob[register_read[i].src1_tag].valid==0)
                              {
                                counter=1;
                                break;
                              }
                            }
                            if(counter==1)
                                register_read[i].src1_rdy=1;
                            //else
			    //  register_read[i].src1_rdy=0;
                        }
                        else
                            register_read[i].src1_rdy=1;    //else independent instruction,hence instruction is ready
                    }

                    if(register_read[i].src2_value!=-1) //checks validity of second source register
                    {
                        if(rmt[register_read[i].src2_value].valid==1)   //if true then dependent instruction
                        {

                            for(k=0;k<rob_size;k++)
                            {
                                if(rob[k].dst==register_read[i].src2_value && rob[k].rdy==1 && (rob[register_read[i].src2_tag].rdy==1))
                                {
                                    counter1=1;
                                    break;
                                }
                              if(rob[register_read[i].src2_tag].valid==0)
                              {
                                counter1=1;
                                break;
                              }

                            }
                            if(counter1==1)
                                register_read[i].src2_rdy=1;
                            //else
			    //  register_read[i].src2_rdy=0;
                        }

                        else
                            register_read[i].src2_rdy=1;    //else independent instruction,hence instruction is ready
                    }

                if(register_read[i].src1_value==-1)     //if no source register,then that instruction is ready partly
                    register_read[i].src1_rdy=1;
                if(register_read[i].src2_value==-1)     //if no source register,then that instruction is ready partly
                    register_read[i].src2_rdy=1;

		}

        //move the RR bundle to DI bundle at a vacant place
                //for(k=0;k<width;k++)
                //{
                    if(dispatch[i].valid==0 && register_read[i].valid==1)
                    {
                        register_read[i].valid=0;       //remove entry from pipeline
                        dispatch[i].age=register_read[i].age;
                        dispatch[i].dest=register_read[i].dest;
                        dispatch[i].dst_tag=register_read[i].dst_tag;
                        dispatch[i].src1_rdy=register_read[i].src1_rdy;
                        dispatch[i].src1_tag=register_read[i].src1_tag;
                        dispatch[i].src1_value=register_read[i].src1_value;
                        dispatch[i].src2_rdy=register_read[i].src2_rdy;
                        dispatch[i].src2_tag=register_read[i].src2_tag;
                        dispatch[i].src2_value=register_read[i].src2_value;
                        dispatch[i].type=register_read[i].type;
                        dispatch[i].src1_value_rob=register_read[i].src1_value_rob;
                        dispatch[i].src2_value_rob=register_read[i].src2_value_rob;
                        dispatch[i].valid=1;

                        pipeline[dispatch[i].age].di=cycle_counter+1;

                        //break;
                        }
                  //  }
                }
            }
        }

    return;
}

 // If RN contains a rename bundle:
 // If either RR is not empty (cannot accept a new register-read bundle)
 // or the ROB does not have enough free entries to accept the entire
 // rename bundle, then do nothing.
 // If RR is empty (can accept a new register-read bundle) and the ROB
 // has enough free entries to accept the entire rename bundle, then
 // process (see below) the rename bundle and advance it from RN to RR. //
 // How to process the rename bundle:
 // Apply your learning from the class lectures/notes on the steps for // renaming:
 // (1) Allocate an entry in the ROB for the instruction,
 // (2) Rename its source registers, and
 // (3) Rename its destination register (if it has one).
 // Note that the rename bundle must be renamed in program order.
 // Fortunately, the instructions in the rename bundle are in program order).
void start_register_renaming(struct reorder_buffer *tail,struct reorder_buffer rob[],struct RN rename[],struct RR register_read[],struct rename_map_table rmt[])
{
    rename_counter=0;
    read_counter=0;
    rob_counter=0;

    for(i=0;i<width;i++)    //checking the number of full stages in RN bundle
    {
        if(rename[i].valid==1)
            rename_counter+=1;
    }

    if(rename_counter>0)   //proceed only if RN bundle is full
    {
        for(i=0;i<width;i++)    //count the number of instructions present in RR
        {
            if(register_read[i].valid==0)
                read_counter+=1;
        }

        for(i=0;i<rob_size;i++) //count the number of instructions present in ROB
        {
            if(rob[i].valid==0)
                rob_counter+=1;
        }

        if(read_counter==width && rob_counter>=width)   //proceed only if RR is empty and ROB has sufficient space to accommodate new instructions
        {
            for(i=0;i<width;i++)
            {
                if(rename[i].valid==1)
                {
                    //renaming source register 1
                    if(rename[i].src1_value!=-1)
                    {
                        if(rmt[(rename[i].src1_value)].valid==1)
                        {
                            rename[i].src1_value_rob=1;
                            rename[i].src1_tag=rmt[(rename[i].src1_value)].rob_tag;
                        }
                        else
                      {
                        rename[i].src1_value_rob=0;
                      }
                    }

                    //renaming source register 2
                    if(rename[i].src2_value!=-1)
                    {
                        if(rmt[(rename[i].src2_value)].valid==1)
                        {
                            rename[i].src2_tag=rmt[(rename[i].src2_value)].rob_tag;
                            rename[i].src2_value_rob=1;
                        }
                        else
                          {
                            rename[i].src2_value_rob=0;
                          }
                    }

                    /////rob entry
                    rob[tail1].valid=1;
                    rob[tail1].rdy=0;
                    rob[tail1].dst=rename[i].dest;
                    rob[tail1].exc=0;
                    rob[tail1].mis=0;


                    //entry in RMT
                    if(rename[i].dest!=-1)
                    {
                        rmt[rob[tail1].dst].rob_tag=rob[tail1].number;
                        rmt[rob[tail1].dst].valid=1;
                    }


                    //renaming destination in RR so that it points to ROB
                    rename[i].dst_tag=rob[tail1].number;

                    if(tail1==rob_size-1)    //if tail points to last entry in ROB then point to start
                        tail1=0;
                    else
                        tail1++;                     //point to next structure


                    //copy from RN to RR to ampty location
                    //for(k=0;k<width;k++)
                    //{
                        if(register_read[i].valid==0 && rename[i].valid==1)
                        {
                            rename[i].valid=0;  //removing instruction from RR bundle
                            register_read[i].age=rename[i].age;
                            register_read[i].dest=rename[i].dest;
                            register_read[i].dst_tag=rename[i].dst_tag;
                            register_read[i].src1_value=rename[i].src1_value;
                            register_read[i].src1_tag=rename[i].src1_tag;
                            register_read[i].src2_tag=rename[i].src2_tag;
                            register_read[i].src2_value=rename[i].src2_value;
                            register_read[i].type=rename[i].type;
                            register_read[i].valid=1;
                            register_read[i].src1_value_rob=rename[i].src1_value_rob;
                            register_read[i].src2_value_rob=rename[i].src2_value_rob;
							register_read[i].src1_rdy=0;
							register_read[i].src2_rdy=0;
                            pipeline[register_read[i].age].rr=cycle_counter+1;
                          //break;
                       }
                    //}
                }
            }
        }
    }
    return;
}

// If DE contains a decode bundle:
// If RN is not empty (cannot accept a new rename bundle), then do
// nothing. If RN is empty (can accept a new rename bundle), then
// advance the decode bundle from DE to RN.
void start_decoding(struct DE decode[],struct RN rename[])
{
    decode_counter=0;
    rename_counter=0;
    for(i=0;i<width;i++)    //count the number of instructions in decode counter
    {
        if(decode[i].valid==1)
            decode_counter+=1;
    }

    if(decode_counter>0)   //proceed only if DE is filled with instructions
    {
        for(i=0;i<width;i++)    //count the number of vacant slots in RN bundle
        {
            if(rename[i].valid==0)
                rename_counter+=1;
        }

        if(rename_counter==width)   //proceed only if RN bundle is fully empty
        {
	  // printf("jgjhfhj\n");
            for(i=0;i<width;i++)    //transfer all instructions from DE to RN at an empty location
            {
                if(decode[i].valid==1)
                {
                    //for(k=0;k<width;k++)
                    //{
                        if(rename[i].valid==0)
                        {
                            decode[i].valid=0;      //removing from pipeline
                            //printf("rename bundle : %d %d %d %d\n",rename[i].type,rename[i].dest,rename[i].src1_value,rename[i].src2_value);
                            rename[i].age=decode[i].age;
                            rename[i].dest=decode[i].dest;
                            rename[i].src1_value=decode[i].src1;
                            rename[i].src2_value=decode[i].src2;
                            rename[i].type=decode[i].type;
                            rename[i].valid=1;


                            pipeline[rename[i].age].rn=cycle_counter+1;



                            //printf("rename bundle : %d %d %d %d\n",rename[i].type,rename[i].dest,rename[i].src1_value,rename[i].src2_value);
                          //break;
                        }
                    //}
                }
            }
        }
    }

    return;
}

// Do nothing if instruction cache is perfect (CACHE_SIZE=0) and either
// (1) there are no more instructions in the trace file or
// (2) DE is not empty (cannot accept a new decode bundle).
//
// If there are more instructions in the trace file and if DE is empty
// (can accept a new decode bundle), then fetch up to WIDTH
// instructions from the trace file into DE. Fewer than WIDTH
// instructions will be fetched only if the trace file has fewer than
// WIDTH instructions left.
//
// If instruction cache is imperfect or next-line prefetcher is
// enabled, in addition to the above operations, adjust the timer to
// model fetch latency when necessary.
void start_fetch(FILE *fp,struct DE decode[],int cache_size)
{
    decode_counter=0;
    for(i=0;i<width;i++)    //count the number of instructions in DE
    {
        if(decode[i].valid==0)
            decode_counter+=1;
    }

    if(cache_size==0 && (feof(fp) || decode_counter!=width))
    {
        //if(decode_counter!=width)
          //  printf("Do nothing\n");
        //if(feof(fp))
          //  printf("End of file hahaha\n");
    }

    else if(cache_size==0 && (!feof(fp)) && decode_counter==width)  //proceed only if end of file has not been reached and DE is empty
    {
        for(i=0;i<width;i++)    //fetch upto width instructions
        {
            fscanf(fp,"%llx %d %d %d %d\n",&pc,&decode[i].type,&decode[i].dest,&decode[i].src1,&decode[i].src2);
            decode[i].valid=1;
            inst_counter+=1;
            decode[i].age=inst_counter;

            pipeline[inst_counter].type=decode[i].type;
            pipeline[inst_counter].seq=decode[i].age;
            pipeline[inst_counter].src1=decode[i].src1;
            pipeline[inst_counter].src2=decode[i].src2;
            pipeline[inst_counter].dest=decode[i].dest;
            pipeline[inst_counter].fe=cycle_counter;
            pipeline[inst_counter].de=cycle_counter+1;


	    //    printf("%llx %d %d %d %d\n",pc,decode[i].type,decode[i].dest,decode[i].src1,decode[i].src2);
	    // printf("%d\n",inst_counter);
            if(feof(fp))    //if no more instructions are present in trace file,then get out the loop
                break;
        }
    }

    //printf("Exiting fetch\n");
    return;
}


void print_rob(struct reorder_buffer rob[])
{
  //    printf("Number\t\tDest\t\tReady\t\tValid\n");
    for(i=0;i<rob_size;i++)
    {
      //    printf("%d\t\t%d\t\t%d\t\t%d\n",rob[i].number,rob[i].dst,rob[i].rdy,rob[i].valid);
    }
    printf("Head:%d\n",head1);
    printf("Tail:%d\n",tail1);
    return;
}
/*

void print_decode(struct DE decode[])
{
    printf("Type\tDest\tSr1\tSr2\tValid\tAge");
    for(i=0;i<width;i++)
        printf("%d\t%d\t%d\t%d\t%d\t%d\n",decode[i].type,decode[i].dest,decode[i].src1,decode[i].src2,decode[i].valid,decode[i].age);
    return;
}

void print_rename(struct RN rename[])
{
    printf("Type\tDest\tSr1 Tag\tSr2 Tag\tSr1_Value\tSr2_Value\tValid\tAge\tDst_Tag");
    for(i=0;i<width;i++)
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",rename[i].type,rename[i].dest,rename[i].src1_tag,rename[i].src2_tag,rename[i].src1_value,rename[i].src2_value,rename[i].valid,rename[i].age,rename[i].dst_tag);
    return;
}

void print_rr(struct RR read[])
{
    printf("Type\tDest\tSr1 Tag\tSr2 Tag\tSr1_Value\tSr2_Value\tValid\tAge\tDst_Tag\tSr1_rdy\tSr2_rdy");
    for(i=0;i<width;i++)
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",read[i].type,read[i].dest,read[i].src1_tag,read[i].src2_tag,read[i].src1_value,read[i].src2_value,read[i].valid,read[i].age,read[i].dst_tag,read[i].src1_rdy,read[i].src2_rdy);
    return;
}

void print_iq*/
