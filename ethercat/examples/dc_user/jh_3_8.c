#include "ecrt.h"
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <malloc.h>
#include <inttypes.h>

static unsigned int counter = 0;
static unsigned int sync_ref_counter = 0;
static struct timespec apptime;

#define TASK_FREQUENCY  125  /* Hz */
#define CLOCK_TO_USE CLOCK_REALTIME
#define TIMEOUT_CLEAR_ERROR  (1*TASK_FREQUENCY) /* clearing error timeout */

/*Time calculation*/
#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / TASK_FREQUENCY)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC +   \
                       (B).tv_nsec - (A).tv_nsec)
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)


// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};
int pos = 0;
int tqu = 0;
int vel = 0;
int ii = 0;

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;


#define InovanceVidPid  0x00000130, 0x01300007

#define Inovance_IS620N_DC_SYNC_SHIFT 4400000

// offsets for PDO entries
static unsigned int ctrl_word   ;
static unsigned int mode  ;
static unsigned int tar_torq    ;
static unsigned int max_torq    ;
static unsigned int tar_pos    ;

static unsigned int digital_output;
static unsigned int max_speed  ;
static unsigned int touch_probe_func ;
static unsigned int tar_vel ;
static unsigned int error_code  ;
static unsigned int status_word;
static unsigned int mode_display ;
static unsigned int pos_act;
static unsigned int vel_act;
static unsigned int torq_act;
static unsigned int pos_gap_act;
static unsigned int touch_probe_status;
static unsigned int touch_probe_pos;
static unsigned int touch_probe_pos2;
static unsigned int digital_input;

// offsets for PDO entries
static int off_dig_out;
static int off_counter_in;
static int off_counter_out;


static unsigned int blink = 0;

const struct timespec cycletime = {0, PERIOD_NS};

static ec_sdo_request_t *status_request ;
static ec_sdo_request_t *control_word_request ;
static ec_sdo_request_t *position_request ;
static ec_sdo_request_t *position_current_request ;

static struct {
    unsigned int ctrl_word;
    unsigned int target_position;
    unsigned int target_torque;
    unsigned int target_velocity;
    unsigned int max_torque;
    unsigned int DO;
    unsigned int DI;
    unsigned int act_DI;
    
    unsigned int offset_velocity ;
    unsigned int offset_torque ;
    unsigned int offset_position ;

    unsigned int error_code;
    unsigned int status_word ;
    unsigned int act_position;
    unsigned int act_torque;

    unsigned int disp_mode;
    unsigned int actual_ferr;
    unsigned int probe_status;
    unsigned int probe1_pos;
    
    unsigned int probe2_pos;
    unsigned int act_velocity;
    unsigned int mode_Of_Operation;
    unsigned int mode_Of_Operation_dsiplay;
} offset[7];


const static ec_pdo_entry_reg_t domain1_regs[] = {
    //slave 0
    //RxPDO-1602
    { 0,4, InovanceVidPid, 0x6040, 0, &offset[4].ctrl_word },
    //{ 0,0, InovanceVidPid, 0x6060, 0, &offset[4].mode_Of_Operation },
    { 0,4, InovanceVidPid, 0x6071, 0, &offset[4].target_torque },
    { 0,4, InovanceVidPid, 0x607a, 0, &offset[4].target_position },
    { 0,4, InovanceVidPid, 0x60b1, 0, &offset[4].offset_velocity },
    { 0,4, InovanceVidPid, 0x60b2, 0, &offset[4].offset_torque },
    { 0,4, InovanceVidPid, 0x60ff, 0, &offset[4].target_velocity },

    //TxPDO-1a02
    { 0,4, InovanceVidPid, 0x6041, 0, &offset[4].status_word },      
    { 0,4, InovanceVidPid, 0x6064, 0, &offset[4].act_position},
    { 0,4, InovanceVidPid, 0x606c, 0, &offset[4].act_velocity},    
    { 0,4, InovanceVidPid, 0x6077, 0, &offset[4].act_torque },
    { 0,4, InovanceVidPid, 0x6061, 0, &offset[4].mode_Of_Operation_dsiplay },      
      
       

    { 0,5, InovanceVidPid, 0x6040, 0, &offset[5].ctrl_word },
    //{ 0,0, InovanceVidPid, 0x6060, 0, &offset[5].mode_Of_Operation },
    { 0,5, InovanceVidPid, 0x6071, 0, &offset[5].target_torque },
    { 0,5, InovanceVidPid, 0x607a, 0, &offset[5].target_position },
    { 0,5, InovanceVidPid, 0x60b1, 0, &offset[5].offset_velocity },
    { 0,5, InovanceVidPid, 0x60b2, 0, &offset[5].offset_torque },
    { 0,5, InovanceVidPid, 0x60ff, 0, &offset[5].target_velocity },

    //TxPDO-1a02
    { 0,5, InovanceVidPid, 0x6041, 0, &offset[5].status_word },      
    { 0,5, InovanceVidPid, 0x6064, 0, &offset[5].act_position},
    { 0,5, InovanceVidPid, 0x606c, 0, &offset[5].act_velocity},    
    { 0,5, InovanceVidPid, 0x6077, 0, &offset[5].act_torque },
    { 0,5, InovanceVidPid, 0x6061, 0, &offset[5].mode_Of_Operation_dsiplay },   
    
    { 0,6, InovanceVidPid, 0x6040, 0, &offset[6].ctrl_word },
    //{ 0,0, InovanceVidPid, 0x6060, 0, &offset[6].mode_Of_Operation },
    { 0,6, InovanceVidPid, 0x6071, 0, &offset[6].target_torque },
    { 0,6, InovanceVidPid, 0x607a, 0, &offset[6].target_position },
    { 0,6, InovanceVidPid, 0x60b1, 0, &offset[6].offset_velocity },
    { 0,6, InovanceVidPid, 0x60b2, 0, &offset[6].offset_torque },
    { 0,6, InovanceVidPid, 0x60ff, 0, &offset[6].target_velocity },

    //TxPDO-1a02
    { 0,6, InovanceVidPid, 0x6041, 0, &offset[6].status_word },      
    { 0,6, InovanceVidPid, 0x6064, 0, &offset[6].act_position},
    { 0,6, InovanceVidPid, 0x606c, 0, &offset[6].act_velocity},    
    { 0,6, InovanceVidPid, 0x6077, 0, &offset[6].act_torque },
    { 0,6, InovanceVidPid, 0x6061, 0, &offset[6].mode_Of_Operation_dsiplay },  
    {}
};


/* IS620N */
ec_pdo_entry_info_t IS620N_pdo_entries[] = {
    /* RxPdo 0x1602 */
    { 0x6040, 0x00, 16 }, 
    //{ 0x6060, 0x00, 8 },      
    { 0x6071, 0x00, 16 }, 
    { 0x607a, 0x00, 32 },  
    { 0x60b1, 0x00, 32 }, 
    { 0x60b2, 0x00, 16 },  
    { 0x60ff, 0x00, 32 },  

    /* TxPDO 0x1a02 */
    { 0x6041, 0x00, 16 },
    { 0x6064, 0x00, 32 }, 
    { 0x606c, 0x00, 32 },
    { 0x6077, 0x00, 16 },
    { 0x6061, 0x00, 8 },
};

ec_pdo_info_t IS620N_pdos[] = {
    { 0x1600, 6, IS620N_pdo_entries + 0 },
    { 0x1a00, 5, IS620N_pdo_entries + 6 },
   // { 0x1701, 4, IS620N_pdo_entries + 0 },
   // { 0x1B04, 10, IS620N_pdo_entries + 4 },

};

ec_sync_info_t IS620N_syncs[] = {
    { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE },
    { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE },
    { 2, EC_DIR_OUTPUT, 1, IS620N_pdos + 0, EC_WD_DEFAULT },
    { 3, EC_DIR_INPUT, 1, IS620N_pdos + 1, EC_WD_DEFAULT },
    { 0xFF }
};

/*****************************************************************************/

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    } else {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}

/*****************************************************************************/

void check_domain1_state(void)
{
    ec_domain_state_t ds;
    ecrt_domain_state(domain1, &ds);
    if (ds.working_counter != domain1_state.working_counter)
        printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

/*****************************************************************************/

void check_master_state(void)
{
    ec_master_state_t ms;
    ecrt_master_state(master, &ms);
    if (ms.slaves_responding != master_state.slaves_responding)
        printf("%u slave(s).\n", ms.slaves_responding);
    if (ms.al_states != master_state.al_states)
        printf("AL states: 0x%02X.\n", ms.al_states);
    if (ms.link_up != master_state.link_up)
        printf("Link is %s.\n", ms.link_up ? "up" : "down");

    master_state = ms;
}

/****************************************************************************/

void cyclic_task()
{
    struct timespec wakeupTime, time;
    // get current time
    clock_gettime(CLOCK_TO_USE, &wakeupTime);
    pos = 1;
    int dir = 1;
    while(1) {
        wakeupTime = timespec_add(wakeupTime, cycletime);
        clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wakeupTime, NULL);
        is620n_receive();        

        for(ii=4;ii<=6;ii++){
        
          uint16_t ss = EC_READ_U16(domain1_pd + offset[ii].status_word);
          printf("status %d : 0x%04x \n",ii,ss);
          if((ss&0xFF)==0x50){
              EC_WRITE_U16( domain1_pd + offset[ii].ctrl_word, 0x06 );
              printf("0x%04x \n",0x06);
          }
          if((ss&0xFF)==0x31){
              EC_WRITE_U16( domain1_pd + offset[ii].ctrl_word, 0x07 );
          }  
          if((ss&0xFF)==0x33){
              EC_WRITE_U16( domain1_pd + offset[ii].ctrl_word, 0x0F );
              
          }      
          if( (ss&0xFF) ==0x37){  
              pos = EC_READ_S32(domain1_pd + offset[ii].act_position);
  
              printf("position %d : 0x%04x \n",ii,pos);
              EC_WRITE_S32( domain1_pd + offset[ii].target_position, pos+800 );
              
              //vel = EC_READ_S32(domain1_pd + offset[ii].target_velocity);
              //printf("tvelocity %d : 0x%04x \n",ii,vel);
          }
        }
   
        is620n_start();
    }
}



void is620n_start()
{
    clock_gettime(CLOCK_TO_USE, &apptime);

    ecrt_master_application_time(master, TIMESPEC2NS(apptime));
    if (sync_ref_counter) {
        sync_ref_counter--;
    } else {
        sync_ref_counter = 1; // sync every cycle
        ecrt_master_sync_reference_clock(master); //DC reference clock drift compensation
    }
    ecrt_master_sync_slave_clocks(master);  //DC clock drift compensation     
    // queue process data
    ecrt_domain_queue(domain1);            
    // send EtherCAT data
    ecrt_master_send(master);
}

void is620n_receive()
{
    /* receive process data */
    ecrt_master_receive(master);
    ecrt_domain_process(domain1);
}

void is620n_master_activate()
{
    printf("......Activating master......\n");
    if ( ecrt_master_activate( master ) ) {
        exit( EXIT_FAILURE );
    }
    if ( !(domain1_pd = ecrt_domain_data( domain1 )) ) {
        exit( EXIT_FAILURE );
    }

    printf("......Master  Activated.......\n");

}

/****************************************************************************/


void is620n_init()
{
    ec_master_info_t master_info;
    ec_slave_info_t slave_info;
    int ret;
    int slavecnt;
    ec_slave_config_t *sc;
   
 //   uint8_t* data = (uint8_t*)malloc(4*sizeof(uint8_t));
    int32_t sdo_data;
    //act_pos =(int32_t*)malloc(6*sizeof(int32_t));
   
    //uint32_t  abort_code;
   // size_t rb;
    int i = 0;
    master = ecrt_request_master( 0 );
    if ( !master ) {
        exit( EXIT_FAILURE );
    }
   
    domain1 = ecrt_master_create_domain( master );
    if ( !domain1 ) {
        exit( EXIT_FAILURE );
    }
    
    //---------get master / salve info----------------------
    ret = ecrt_master(master,&master_info);
    slavecnt = master_info.slave_count;
    printf("ret = %d, slavecnt = %d, apptime = %"PRIu64"\n",ret, master_info.slave_count,master_info.app_time);
    ret = ecrt_master_get_slave(master,0,&slave_info);
    printf("ret = %d,spos = %d, pcode = %d\n",ret,slave_info.position,slave_info.product_code);
  
    //---------end get master / salve info----------------------
    

        printf("servo %d  begin init! \n",i);
        

    for(ii=4;ii<=6;ii++){

		   ec_slave_config_t *sc;
        if (!(sc = ecrt_master_slave_config( master,0,ii ,InovanceVidPid)) ) {
            fprintf(stderr, "Failed to get slave configuration for IS620N.\n");
            exit( EXIT_FAILURE );
        }

        printf("Configuring PDOs...\n");
        if ( ecrt_slave_config_pdos( sc, EC_END, IS620N_syncs ) ) {
            fprintf( stderr, "Failed to configure IS620N PDOs.\n" );
            exit( EXIT_FAILURE );
        }
        

        // configure SYNC signals for this slave
        ecrt_slave_config_sdo8( sc, 0x6060, 0, 8 );
        //ecrt_slave_config_sdo16( sc, 0x1C32, 1, 2 );
        //ecrt_slave_config_sdo16( sc, 0x1C33, 1, 2 );
        //ecrt_slave_config_sdo8( sc, 0x60C2, 1, 8 );
        //ecrt_slave_config_sdo8( sc, 0x60C2, 2, -3 );     
        
        ecrt_slave_config_dc(sc, 0x0300, PERIOD_NS,4400000, 0, 0);
      }
        
        
        

       

    if ( ecrt_domain_reg_pdo_entry_list( domain1, domain1_regs ) ) {
        fprintf( stderr, "PDO entry registration failed!\n" );
        exit( EXIT_FAILURE );
    }

}

int main(int argc, char **argv)
{

  
    is620n_init();
    is620n_master_activate();
    
    //while(1){    	
        //ctask2();   
        //ecrt_master_receive(master);ecrt_domain_process(domain1);    
        //ecrt_domain_queue(domain1);ecrt_master_send(master);
    
     //}


    
    
    cyclic_task();    
    return 0;
}

/****************************************************************************/