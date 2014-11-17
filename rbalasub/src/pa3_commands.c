#include "../include/pa3_commands.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

uint16_t self_id;
uint16_t self_port;
uint32_t self_ip;

void processCommands(int argc, char **argv);
bool update_cost(uint16_t my_id, uint16_t server_id, char *cost, char **error_string);
void display_rt();
int node_cmp(const void * n1, const void * n2);
void dump_packet();


/**
 * Handle User Commands to application
 * @param command_string raw string supplied to stdin
 */
void handle_commands(char * command_string){

	/******* Handle enter key press  *********/
        command_string[strlen(command_string)-1]='\0'; //remove new line

        /******* Tokenize input *********/
        char *arg;
        int argc=0;
        char **argv = (char **)calloc(4, sizeof (char*));
        arg = strtok(command_string, " ");
        while(arg)
        {
            argv[argc] = strdup(arg);
            argc++;
            if (argc>4)
            {
                fprintf(stderr, "Too many arguments\n");
                free(argv);
                return;
            }
            arg = strtok(NULL," ");
        }

        /******* Send tokenized commands for processing *********/
        if (!(argc==0))
        {
            processCommands(argc, argv);
            /******* Free the commands array *********/
            int ii =0;
            for (ii = 0; ii < argc; ++ii)
            {
                free(argv[ii]);
            }
        }
        free (argv);
}

/**
 * Process tokenized commands
 * @param argc Number of arguments
 * @param argv The argument array
 */
void processCommands(int argc, char **argv){

    bool error_flag=false;
    char *command_string = argv[0];
    char *error_string = (char *)"";

	if (strcasecmp("academic_integrity",argv[0])==0)
	{	

        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);

		printf("\n");
		cse4589_print_and_log((char *)"I have read and understood the course academic integrity policy \
located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
		printf("\n\n");
 

	}else if (strcasecmp("update",argv[0])==0)
    {
        if (argc != 4)
        {
            error_flag = true;
            error_string = (char *)"Invalid number of arguments";
        }else{
            cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
            error_flag = !update_cost(
                (uint16_t)strtoul(argv[1],NULL,0), 
                (uint16_t)strtoul(argv[2],NULL,0), 
                argv[3],
                &error_string);
        }
             
    }else if (strcasecmp("step",argv[0])==0)
    {
        error_flag = true;
        error_string = (char *)"To do";   
    }else if (strcasecmp("packets",argv[0])==0)
    {
        error_flag = true;
        error_string = (char *)"To do"; 
    }else if (strcasecmp("display",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        display_rt(); 
        
    }else if (strcasecmp("disable",argv[0])==0)
    {
        error_flag = true;
        error_string = (char *)"To do"; 
    }else if (strcasecmp("crash",argv[0])==0)
    {
        error_flag = true;
        error_string = (char *)"To do"; 
    }else if (strcasecmp("dump",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        dump_packet(); 
    }else if (strcasecmp("exit",argv[0])==0)
    {
        exit(EXIT_SUCCESS);
    }else{
        error_flag = true;
        error_string = (char *)"Unknown command";

    }

    if (error_flag)
    {
        cse4589_print_and_log((char *)"%s:%s\n",command_string,error_string);        
    }

}

/**
 * Create and dump the packet to binary
 */
void dump_packet(){

    size_t pkt_size = 8 + 12*environment.num_servers;
    char *packet = (char *)calloc(1,8 + 12*environment.num_servers);
    memcpy(packet, &(environment.num_servers), 2);
    memcpy(packet+2, &(self_port), 2);
    memcpy(packet+4, &(self_ip), 4);
    for (int i = 0; i < environment.num_servers; ++i)
    {
        memcpy(packet+8+(i*12), &(environment.nodes[i].ip_addr_bin), 4);
        memcpy(packet+12+(i*12), &(environment.nodes[i].port), 2);
        memcpy(packet+16+(i*12), &(environment.nodes[i].server_id), 2);
        memcpy(packet+18+(i*12), &(environment.nodes[i].cost), 2);
    }

    cse4589_dump_packet(packet, pkt_size);


}



/**
 * Display the routing table
 */
void display_rt(){

    /******* Sort the nodes *********/
    qsort( environment.nodes, environment.num_servers, sizeof(Node), node_cmp);

    for (int i = 0; i < environment.num_servers ; ++i)
    {
        Node node= environment.nodes[i];
        cse4589_print_and_log((char *)"%-15d%-15d%-15d\n", 
            node.server_id,node.next_hop_server_id,node.cost); 
    }

}

/**
 * Comparator function for sorting array
 * @param  n1 node 1
 * @param  n2 node 2
 * @return    result
 */
int node_cmp(const void * n1, const void * n2){
    Node *n1_s = (Node *)n1;
    Node *n2_s = (Node *)n2;
    return n1_s->server_id - n2_s->server_id;
}

/**
 * Update cost between two links (will be executed on both machines)
 * @param  my_id        self server_id
 * @param  server_id    server_id of the node
 * @param  cost         cost to be updated
 * @param  error_string error string out parameter if any
 * @return              update was successful or not
 */
bool update_cost (uint16_t my_id, uint16_t server_id, char *cost, char **error_string) {

    if (my_id != self_id)
    {
      *error_string = (char *)"Invalid server_id 1";
      return false;
    }else{
        for (int i = 0; i < environment.num_servers; ++i)
        {   
            if (server_id == environment.nodes[i].server_id)
            {
                if ( strcasecmp("inf",cost) == 0 )
                {
                    environment.nodes[i].cost = USHRT_MAX;
                    environment.nodes[i].next_hop_server_id = -1;
                }else if(strtol(cost, NULL, 0) > 0 ){
                    uint16_t cost_int = (uint16_t)strtoul(cost, NULL, 0);
                    environment.nodes[i].cost = cost_int;
                }else{
                    *error_string = (char *)"Invalid cost";
                    return false;
                }
                return true;
            }
        }
        *error_string = (char *)"Invalid server_id 2";
        return false;
    }
}



