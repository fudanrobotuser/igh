/**
 * 基于 ethercat igh example dc_user 例子改写的 伺服控制器
 * 功能为:
 *  1 igh 主站配置
 *  2 伺服状态机切换
 *  3 伺服位置写入
 *  4 伺服状态反馈
 * @date 20240609
 * @author wei1224hf@gmail.com
 * @version beta.1
 * ****************************/
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "ecat_data_buffer.h"

//共享内存区域,2M大小
#define SHM_KEY 12345
#define SHM_SIZE (1024 * 1024 * 2)

static unsigned int counter = 0;
static unsigned int sync_ref_counter = 0;
static struct timespec apptime;

//同步周期时钟配置,8毫秒下发周期控制
#define TASK_FREQUENCY 125 /* Hz */
#define CLOCK_TO_USE CLOCK_REALTIME
#define TIMEOUT_CLEAR_ERROR (1 * TASK_FREQUENCY) /* clearing error timeout */
//时间控制相关函数
#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / TASK_FREQUENCY)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
                       (B).tv_nsec - (A).tv_nsec)
#define TIMESPEC2NS(T) ((uint64_t)(T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
const struct timespec cycletime = {0, PERIOD_NS};

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

int ii = 0;

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;

//季华伺服驱动器配置
#define JiHuaVidPid 0x00000130, 0x01300007

// offsets for PDO entries
static unsigned int ctrl_word;
static unsigned int mode;
static unsigned int tar_torq;
static unsigned int max_torq;
static unsigned int tar_pos;

static unsigned int digital_output;
static unsigned int max_speed;
static unsigned int touch_probe_func;
static unsigned int tar_vel;
static unsigned int error_code;
static unsigned int status_word;
static unsigned int mode_display;
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

int enabled[19];

//伺服电机所有属性结构体,用于读取值的位置指针
static struct
{
    unsigned int ctrl_word;
    unsigned int target_position;
    unsigned int target_torque;
    unsigned int target_velocity;
    unsigned int max_torque;
    unsigned int DO;
    unsigned int DI;
    unsigned int act_DI;

    unsigned int offset_velocity;
    unsigned int offset_torque;
    unsigned int offset_position;

    unsigned int error_code;
    unsigned int status_word;
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
} offset[19];

//IGH主栈的主映射表,总计15个电机. 因为 0-3 口给扩展板用了,实际电机的总线分配序号为 4-18 
const static ec_pdo_entry_reg_t domain1_regs[] = {
    // slave 0
    // RxPDO-1602
    {0, 4, JiHuaVidPid, 0x6040, 0, &offset[4].ctrl_word},
    //{ 0,0, JiHuaVidPid, 0x6060, 0, &offset[4].mode_Of_Operation },
    {0, 4, JiHuaVidPid, 0x6071, 0, &offset[4].target_torque},
    {0, 4, JiHuaVidPid, 0x607a, 0, &offset[4].target_position},
    {0, 4, JiHuaVidPid, 0x60b1, 0, &offset[4].offset_velocity},
    {0, 4, JiHuaVidPid, 0x60b2, 0, &offset[4].offset_torque},
    {0, 4, JiHuaVidPid, 0x60ff, 0, &offset[4].target_velocity},

    // TxPDO-1a02
    {0, 4, JiHuaVidPid, 0x6041, 0, &offset[4].status_word},
    {0, 4, JiHuaVidPid, 0x6064, 0, &offset[4].act_position},
    {0, 4, JiHuaVidPid, 0x606c, 0, &offset[4].act_velocity},
    {0, 4, JiHuaVidPid, 0x6077, 0, &offset[4].act_torque},
    {0, 4, JiHuaVidPid, 0x6061, 0, &offset[4].mode_Of_Operation_dsiplay},

    {0, 5, JiHuaVidPid, 0x6040, 0, &offset[5].ctrl_word},
    {0, 5, JiHuaVidPid, 0x6071, 0, &offset[5].target_torque},
    {0, 5, JiHuaVidPid, 0x607a, 0, &offset[5].target_position},
    {0, 5, JiHuaVidPid, 0x60b1, 0, &offset[5].offset_velocity},
    {0, 5, JiHuaVidPid, 0x60b2, 0, &offset[5].offset_torque},
    {0, 5, JiHuaVidPid, 0x60ff, 0, &offset[5].target_velocity},

    {0, 5, JiHuaVidPid, 0x6041, 0, &offset[5].status_word},
    {0, 5, JiHuaVidPid, 0x6064, 0, &offset[5].act_position},
    {0, 5, JiHuaVidPid, 0x606c, 0, &offset[5].act_velocity},
    {0, 5, JiHuaVidPid, 0x6077, 0, &offset[5].act_torque},
    {0, 5, JiHuaVidPid, 0x6061, 0, &offset[5].mode_Of_Operation_dsiplay},

    {0, 6, JiHuaVidPid, 0x6040, 0, &offset[6].ctrl_word},
    {0, 6, JiHuaVidPid, 0x6071, 0, &offset[6].target_torque},
    {0, 6, JiHuaVidPid, 0x607a, 0, &offset[6].target_position},
    {0, 6, JiHuaVidPid, 0x60b1, 0, &offset[6].offset_velocity},
    {0, 6, JiHuaVidPid, 0x60b2, 0, &offset[6].offset_torque},
    {0, 6, JiHuaVidPid, 0x60ff, 0, &offset[6].target_velocity},

    {0, 6, JiHuaVidPid, 0x6041, 0, &offset[6].status_word},
    {0, 6, JiHuaVidPid, 0x6064, 0, &offset[6].act_position},
    {0, 6, JiHuaVidPid, 0x606c, 0, &offset[6].act_velocity},
    {0, 6, JiHuaVidPid, 0x6077, 0, &offset[6].act_torque},
    {0, 6, JiHuaVidPid, 0x6061, 0, &offset[6].mode_Of_Operation_dsiplay},

    {0, 7, JiHuaVidPid, 0x6040, 0, &offset[7].ctrl_word},
    {0, 7, JiHuaVidPid, 0x6071, 0, &offset[7].target_torque},
    {0, 7, JiHuaVidPid, 0x607a, 0, &offset[7].target_position},
    {0, 7, JiHuaVidPid, 0x60b1, 0, &offset[7].offset_velocity},
    {0, 7, JiHuaVidPid, 0x60b2, 0, &offset[7].offset_torque},
    {0, 7, JiHuaVidPid, 0x60ff, 0, &offset[7].target_velocity},

    {0, 7, JiHuaVidPid, 0x6041, 0, &offset[7].status_word},
    {0, 7, JiHuaVidPid, 0x6064, 0, &offset[7].act_position},
    {0, 7, JiHuaVidPid, 0x606c, 0, &offset[7].act_velocity},
    {0, 7, JiHuaVidPid, 0x6077, 0, &offset[7].act_torque},
    {0, 7, JiHuaVidPid, 0x6061, 0, &offset[7].mode_Of_Operation_dsiplay},

    {0, 8, JiHuaVidPid, 0x6040, 0, &offset[8].ctrl_word},
    {0, 8, JiHuaVidPid, 0x6071, 0, &offset[8].target_torque},
    {0, 8, JiHuaVidPid, 0x607a, 0, &offset[8].target_position},
    {0, 8, JiHuaVidPid, 0x60b1, 0, &offset[8].offset_velocity},
    {0, 8, JiHuaVidPid, 0x60b2, 0, &offset[8].offset_torque},
    {0, 8, JiHuaVidPid, 0x60ff, 0, &offset[8].target_velocity},

    {0, 8, JiHuaVidPid, 0x6041, 0, &offset[8].status_word},
    {0, 8, JiHuaVidPid, 0x6064, 0, &offset[8].act_position},
    {0, 8, JiHuaVidPid, 0x606c, 0, &offset[8].act_velocity},
    {0, 8, JiHuaVidPid, 0x6077, 0, &offset[8].act_torque},
    {0, 8, JiHuaVidPid, 0x6061, 0, &offset[8].mode_Of_Operation_dsiplay},

    {0, 9, JiHuaVidPid, 0x6040, 0, &offset[9].ctrl_word},
    {0, 9, JiHuaVidPid, 0x6071, 0, &offset[9].target_torque},
    {0, 9, JiHuaVidPid, 0x607a, 0, &offset[9].target_position},
    {0, 9, JiHuaVidPid, 0x60b1, 0, &offset[9].offset_velocity},
    {0, 9, JiHuaVidPid, 0x60b2, 0, &offset[9].offset_torque},
    {0, 9, JiHuaVidPid, 0x60ff, 0, &offset[9].target_velocity},

    {0, 9, JiHuaVidPid, 0x6041, 0, &offset[9].status_word},
    {0, 9, JiHuaVidPid, 0x6064, 0, &offset[9].act_position},
    {0, 9, JiHuaVidPid, 0x606c, 0, &offset[9].act_velocity},
    {0, 9, JiHuaVidPid, 0x6077, 0, &offset[9].act_torque},
    {0, 9, JiHuaVidPid, 0x6061, 0, &offset[9].mode_Of_Operation_dsiplay},

    {0, 10, JiHuaVidPid, 0x6040, 0, &offset[10].ctrl_word},
    {0, 10, JiHuaVidPid, 0x6071, 0, &offset[10].target_torque},
    {0, 10, JiHuaVidPid, 0x607a, 0, &offset[10].target_position},
    {0, 10, JiHuaVidPid, 0x60b1, 0, &offset[10].offset_velocity},
    {0, 10, JiHuaVidPid, 0x60b2, 0, &offset[10].offset_torque},
    {0, 10, JiHuaVidPid, 0x60ff, 0, &offset[10].target_velocity},

    {0, 10, JiHuaVidPid, 0x6041, 0, &offset[10].status_word},
    {0, 10, JiHuaVidPid, 0x6064, 0, &offset[10].act_position},
    {0, 10, JiHuaVidPid, 0x606c, 0, &offset[10].act_velocity},
    {0, 10, JiHuaVidPid, 0x6077, 0, &offset[10].act_torque},
    {0, 10, JiHuaVidPid, 0x6061, 0, &offset[10].mode_Of_Operation_dsiplay},

    {0, 11, JiHuaVidPid, 0x6040, 0, &offset[11].ctrl_word},
    {0, 11, JiHuaVidPid, 0x6071, 0, &offset[11].target_torque},
    {0, 11, JiHuaVidPid, 0x607a, 0, &offset[11].target_position},
    {0, 11, JiHuaVidPid, 0x60b1, 0, &offset[11].offset_velocity},
    {0, 11, JiHuaVidPid, 0x60b2, 0, &offset[11].offset_torque},
    {0, 11, JiHuaVidPid, 0x60ff, 0, &offset[11].target_velocity},

    {0, 11, JiHuaVidPid, 0x6041, 0, &offset[11].status_word},
    {0, 11, JiHuaVidPid, 0x6064, 0, &offset[11].act_position},
    {0, 11, JiHuaVidPid, 0x606c, 0, &offset[11].act_velocity},
    {0, 11, JiHuaVidPid, 0x6077, 0, &offset[11].act_torque},
    {0, 11, JiHuaVidPid, 0x6061, 0, &offset[11].mode_Of_Operation_dsiplay},

    {0, 12, JiHuaVidPid, 0x6040, 0, &offset[12].ctrl_word},
    {0, 12, JiHuaVidPid, 0x6071, 0, &offset[12].target_torque},
    {0, 12, JiHuaVidPid, 0x607a, 0, &offset[12].target_position},
    {0, 12, JiHuaVidPid, 0x60b1, 0, &offset[12].offset_velocity},
    {0, 12, JiHuaVidPid, 0x60b2, 0, &offset[12].offset_torque},
    {0, 12, JiHuaVidPid, 0x60ff, 0, &offset[12].target_velocity},

    {0, 12, JiHuaVidPid, 0x6041, 0, &offset[12].status_word},
    {0, 12, JiHuaVidPid, 0x6064, 0, &offset[12].act_position},
    {0, 12, JiHuaVidPid, 0x606c, 0, &offset[12].act_velocity},
    {0, 12, JiHuaVidPid, 0x6077, 0, &offset[12].act_torque},
    {0, 12, JiHuaVidPid, 0x6061, 0, &offset[12].mode_Of_Operation_dsiplay},

    {0, 13, JiHuaVidPid, 0x6040, 0, &offset[13].ctrl_word},
    {0, 13, JiHuaVidPid, 0x6071, 0, &offset[13].target_torque},
    {0, 13, JiHuaVidPid, 0x607a, 0, &offset[13].target_position},
    {0, 13, JiHuaVidPid, 0x60b1, 0, &offset[13].offset_velocity},
    {0, 13, JiHuaVidPid, 0x60b2, 0, &offset[13].offset_torque},
    {0, 13, JiHuaVidPid, 0x60ff, 0, &offset[13].target_velocity},

    {0, 13, JiHuaVidPid, 0x6041, 0, &offset[13].status_word},
    {0, 13, JiHuaVidPid, 0x6064, 0, &offset[13].act_position},
    {0, 13, JiHuaVidPid, 0x606c, 0, &offset[13].act_velocity},
    {0, 13, JiHuaVidPid, 0x6077, 0, &offset[13].act_torque},
    {0, 13, JiHuaVidPid, 0x6061, 0, &offset[13].mode_Of_Operation_dsiplay},

    {0, 14, JiHuaVidPid, 0x6040, 0, &offset[14].ctrl_word},
    {0, 14, JiHuaVidPid, 0x6071, 0, &offset[14].target_torque},
    {0, 14, JiHuaVidPid, 0x607a, 0, &offset[14].target_position},
    {0, 14, JiHuaVidPid, 0x60b1, 0, &offset[14].offset_velocity},
    {0, 14, JiHuaVidPid, 0x60b2, 0, &offset[14].offset_torque},
    {0, 14, JiHuaVidPid, 0x60ff, 0, &offset[14].target_velocity},

    {0, 14, JiHuaVidPid, 0x6041, 0, &offset[14].status_word},
    {0, 14, JiHuaVidPid, 0x6064, 0, &offset[14].act_position},
    {0, 14, JiHuaVidPid, 0x606c, 0, &offset[14].act_velocity},
    {0, 14, JiHuaVidPid, 0x6077, 0, &offset[14].act_torque},
    {0, 14, JiHuaVidPid, 0x6061, 0, &offset[14].mode_Of_Operation_dsiplay},

    {0, 15, JiHuaVidPid, 0x6040, 0, &offset[15].ctrl_word},
    {0, 15, JiHuaVidPid, 0x6071, 0, &offset[15].target_torque},
    {0, 15, JiHuaVidPid, 0x607a, 0, &offset[15].target_position},
    {0, 15, JiHuaVidPid, 0x60b1, 0, &offset[15].offset_velocity},
    {0, 15, JiHuaVidPid, 0x60b2, 0, &offset[15].offset_torque},
    {0, 15, JiHuaVidPid, 0x60ff, 0, &offset[15].target_velocity},

    {0, 15, JiHuaVidPid, 0x6041, 0, &offset[15].status_word},
    {0, 15, JiHuaVidPid, 0x6064, 0, &offset[15].act_position},
    {0, 15, JiHuaVidPid, 0x606c, 0, &offset[15].act_velocity},
    {0, 15, JiHuaVidPid, 0x6077, 0, &offset[15].act_torque},
    {0, 15, JiHuaVidPid, 0x6061, 0, &offset[15].mode_Of_Operation_dsiplay},

    {0, 16, JiHuaVidPid, 0x6040, 0, &offset[16].ctrl_word},
    {0, 16, JiHuaVidPid, 0x6071, 0, &offset[16].target_torque},
    {0, 16, JiHuaVidPid, 0x607a, 0, &offset[16].target_position},
    {0, 16, JiHuaVidPid, 0x60b1, 0, &offset[16].offset_velocity},
    {0, 16, JiHuaVidPid, 0x60b2, 0, &offset[16].offset_torque},
    {0, 16, JiHuaVidPid, 0x60ff, 0, &offset[16].target_velocity},

    {0, 16, JiHuaVidPid, 0x6041, 0, &offset[16].status_word},
    {0, 16, JiHuaVidPid, 0x6064, 0, &offset[16].act_position},
    {0, 16, JiHuaVidPid, 0x606c, 0, &offset[16].act_velocity},
    {0, 16, JiHuaVidPid, 0x6077, 0, &offset[16].act_torque},
    {0, 16, JiHuaVidPid, 0x6061, 0, &offset[16].mode_Of_Operation_dsiplay},

    {0, 17, JiHuaVidPid, 0x6040, 0, &offset[17].ctrl_word},
    {0, 17, JiHuaVidPid, 0x6071, 0, &offset[17].target_torque},
    {0, 17, JiHuaVidPid, 0x607a, 0, &offset[17].target_position},
    {0, 17, JiHuaVidPid, 0x60b1, 0, &offset[17].offset_velocity},
    {0, 17, JiHuaVidPid, 0x60b2, 0, &offset[17].offset_torque},
    {0, 17, JiHuaVidPid, 0x60ff, 0, &offset[17].target_velocity},

    {0, 17, JiHuaVidPid, 0x6041, 0, &offset[17].status_word},
    {0, 17, JiHuaVidPid, 0x6064, 0, &offset[17].act_position},
    {0, 17, JiHuaVidPid, 0x606c, 0, &offset[17].act_velocity},
    {0, 17, JiHuaVidPid, 0x6077, 0, &offset[17].act_torque},
    {0, 17, JiHuaVidPid, 0x6061, 0, &offset[17].mode_Of_Operation_dsiplay},

    {0, 18, JiHuaVidPid, 0x6040, 0, &offset[18].ctrl_word},
    {0, 18, JiHuaVidPid, 0x6071, 0, &offset[18].target_torque},
    {0, 18, JiHuaVidPid, 0x607a, 0, &offset[18].target_position},
    {0, 18, JiHuaVidPid, 0x60b1, 0, &offset[18].offset_velocity},
    {0, 18, JiHuaVidPid, 0x60b2, 0, &offset[18].offset_torque},
    {0, 18, JiHuaVidPid, 0x60ff, 0, &offset[18].target_velocity},

    {0, 18, JiHuaVidPid, 0x6041, 0, &offset[18].status_word},
    {0, 18, JiHuaVidPid, 0x6064, 0, &offset[18].act_position},
    {0, 18, JiHuaVidPid, 0x606c, 0, &offset[18].act_velocity},
    {0, 18, JiHuaVidPid, 0x6077, 0, &offset[18].act_torque},
    {0, 18, JiHuaVidPid, 0x6061, 0, &offset[18].mode_Of_Operation_dsiplay},

    {}};

//伺服电机的PDO映射参数
ec_pdo_entry_info_t Igh_pdo_entries[] = {
    /* RxPdo 0x1602 */
    {0x6040, 0x00, 16},
    //{ 0x6060, 0x00, 8 },
    {0x6071, 0x00, 16},
    {0x607a, 0x00, 32},
    {0x60b1, 0x00, 32},
    {0x60b2, 0x00, 16},
    {0x60ff, 0x00, 32},

    /* TxPDO 0x1a02 */
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x606c, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
};

//伺服电机PDO映射参数的组地址
ec_pdo_info_t Igh_pdos[] = {
    {0x1600, 6, Igh_pdo_entries + 0},
    {0x1a00, 5, Igh_pdo_entries + 6},
    // { 0x1701, 4, Igh_pdo_entries + 0 },
    // { 0x1B04, 10, Igh_pdo_entries + 4 },

};

ec_sync_info_t Igh_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, Igh_pdos + 0, EC_WD_DEFAULT},
    {3, EC_DIR_INPUT, 1, Igh_pdos + 1, EC_WD_DEFAULT},
    {0xFF}};

/*****************************************************************************/

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC)
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    }
    else
    {
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


/**
 * 主时钟循环程序,要每8毫秒执行一次,
 * 功能为,切换电机的状态机,读取上层应用下发的位置命令,写入电机的下发位置,
 * 读取电机当前的 位置 速度 扭力 ，反馈给上层应用
 * **/
void cyclic_task()
{
    struct timespec wakeupTime, time;
    // get current time
    clock_gettime(CLOCK_TO_USE, &wakeupTime);
    int act_position = 0;
    int act_torque = 0;
    int act_velocity = 0;
    int target_postion = 0;
    int dir = 1;
    while (1)
    {

        wakeupTime = timespec_add(wakeupTime, cycletime);
        clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wakeupTime, NULL);
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        GROUP_FEEDBACK  feedback;
        GROUP_REFERENCE reference;

        bool data_ok = false;

        for (ii = 4; ii <= 18; ii++)
        {

            uint16_t ss = EC_READ_U16(domain1_pd + offset[ii].status_word);
            enabled[ii] = 0;
            // printf("status %d : 0x%04x \n",ii,ss);
            if ((ss & 0xFF) == 0x50)
            {
                EC_WRITE_U16(domain1_pd + offset[ii].ctrl_word, 0x06);
                printf("0x%04x \n", 0x06);
            }
            else if ((ss & 0xFF) == 0x31)
            {
                EC_WRITE_U16(domain1_pd + offset[ii].ctrl_word, 0x07);
            }
            else if ((ss & 0xFF) == 0x33)
            {
                EC_WRITE_U16(domain1_pd + offset[ii].ctrl_word, 0x0F);
                act_position = EC_READ_S32(domain1_pd + offset[ii].act_position);
                EC_WRITE_S32(domain1_pd + offset[ii].target_position, act_position);
            }
            else if ((ss & 0xFF) == 0x37)
            {
                enabled[ii] = 1;



                // 所有电机都使能的时候,从共享内存读取位置
                if ((enabled[4] == 1) && (enabled[5] == 1) && (enabled[6] == 1) && (enabled[7] == 1) && (enabled[8] == 1) && (enabled[9] == 1) && (enabled[10] == 1) && (enabled[11] == 1) && (enabled[12] == 1) && (enabled[13] == 1) && (enabled[14] == 1) && (enabled[15] == 1) && (enabled[16] == 1) && (enabled[17] == 1) && (enabled[18] == 1))
                {
                    act_position = EC_READ_S32(domain1_pd + offset[ii].act_position);
                    act_velocity = EC_READ_S32(domain1_pd + offset[ii].act_velocity);
                    act_torque = EC_READ_S16(domain1_pd + offset[ii].act_torque);
                    
                    if(ii==4){   
                        memset(&reference, 0, sizeof(reference));
                        memset(&feedback, 0, sizeof(feedback));                  
                        data_ok = edb_pull_ref(&reference);
                    }
                    if(data_ok){
                        target_postion = reference.motor_ref[ii].target_postion;
                        EC_WRITE_S32(domain1_pd + offset[ii].target_position, target_postion);
                        printf("position %d,  %d to %d  \n", ii, act_position, target_postion);                        
                    }
                    else{
                        EC_WRITE_S32(domain1_pd + offset[ii].target_position, act_position);
                    }

                    feedback.motor_fdbk[ii].feedbk_postion = act_position;
                    feedback.motor_fdbk[ii].feedbk_speed = act_velocity;
                    feedback.motor_fdbk[ii].feedbk_torque = act_torque;
                    feedback.motor_fdbk[ii].status_word = ss;
                    edb_push_fdbk(&feedback);

                }
                else
                {
                    EC_WRITE_S32(domain1_pd + offset[ii].target_position, act_position);
                }  
            }
        }

        Igh_rechekTime();
    }
}

void Igh_rechekTime()
{
    clock_gettime(CLOCK_TO_USE, &apptime);

    ecrt_master_application_time(master, TIMESPEC2NS(apptime));
    if (sync_ref_counter)
    {
        sync_ref_counter--;
    }
    else
    {
        sync_ref_counter = 1;                     // sync every cycle
        ecrt_master_sync_reference_clock(master); // DC reference clock drift compensation
    }
    ecrt_master_sync_slave_clocks(master); // DC clock drift compensation
    // queue process data
    ecrt_domain_queue(domain1);
    // send EtherCAT data
    ecrt_master_send(master);
}

void Igh_master_activate()
{
    printf("......Activating master......\n");
    if (ecrt_master_activate(master))
    {
        exit(EXIT_FAILURE);
    }
    if (!(domain1_pd = ecrt_domain_data(domain1)))
    {
        exit(EXIT_FAILURE);
    }

    printf("......Master  Activated.......\n");
}

/****************************************************************************/

void Igh_init()
{
    ec_master_info_t master_info;
    ec_slave_info_t slave_info;
    int ret;
    int slavecnt;
    ec_slave_config_t *sc;

    //   uint8_t* data = (uint8_t*)malloc(4*sizeof(uint8_t));
    int32_t sdo_data;
    // act_pos =(int32_t*)malloc(6*sizeof(int32_t));

    // uint32_t  abort_code;
    // size_t rb;
    int i = 0;
    master = ecrt_request_master(0);
    if (!master)
    {
        exit(EXIT_FAILURE);
    }

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
    {
        exit(EXIT_FAILURE);
    }

    //---------get master / salve info----------------------
    ret = ecrt_master(master, &master_info);
    slavecnt = master_info.slave_count;
    printf("ret = %d, slavecnt = %d, apptime = %" PRIu64 "\n", ret, master_info.slave_count, master_info.app_time);
    ret = ecrt_master_get_slave(master, 0, &slave_info);
    printf("ret = %d,spos = %d, pcode = %d\n", ret, slave_info.position, slave_info.product_code);

    //---------end get master / salve info----------------------

    printf("servo %d  begin init! \n", i);

    for (ii = 4; ii <= 18; ii++)
    {

        ec_slave_config_t *sc;
        if (!(sc = ecrt_master_slave_config(master, 0, ii, JiHuaVidPid)))
        {
            fprintf(stderr, "Failed to get slave configuration for Igh.\n");
            exit(EXIT_FAILURE);
        }

        printf("Configuring PDOs...\n");
        if (ecrt_slave_config_pdos(sc, EC_END, Igh_syncs))
        {
            fprintf(stderr, "Failed to configure Igh PDOs.\n");
            exit(EXIT_FAILURE);
        }

        // configure SYNC signals for this slave
        ecrt_slave_config_sdo8(sc, 0x6060, 0, 8);
        // ecrt_slave_config_sdo16( sc, 0x1C32, 1, 2 );
        // ecrt_slave_config_sdo16( sc, 0x1C33, 1, 2 );
        // ecrt_slave_config_sdo8( sc, 0x60C2, 1, 8 );
        // ecrt_slave_config_sdo8( sc, 0x60C2, 2, -3 );

        ecrt_slave_config_dc(sc, 0x0300, PERIOD_NS, PERIOD_NS / 4, 0, 0);
    }

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
    {
        fprintf(stderr, "PDO entry registration failed!\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{

    // Get the shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1)
    {
        perror("shmget");
        return EXIT_FAILURE;
    }

    void *appPtr;

    appPtr = shmat(shmid, 0, 0);
    if (appPtr == (void *)-1)
    {
        return -1;
    }

    edb_init(appPtr, SHM_SIZE, false);

    Igh_init();
    Igh_master_activate();

    cyclic_task();
    return 0;
}

/****************************************************************************/
