#include <config.h>
#include <common.h>
#include <command.h>
#include <linux/types.h>
#include <android_bootimg.h>
#include <linux/keypad.h>
#include <linux/key_code.h>
#include <boot_mode.h>
#include <asm/arch/gpio.h>

#define COMMAND_MAX 128

//#define DEBUG
#ifdef DEBUG
#define DBG(fmt...) printf(fmt)
#else
#define DBG(fmt...) 
#endif

extern int power_button_pressed(void);
extern int charger_connected(void);
extern int alarm_triggered(void);
extern void CHG_TurnOn (void);
extern void CHG_ShutDown (void);
extern void CHG_Init (void);

int boot_pwr_check(void)
{
    static int total_cnt = 0;
    if(!power_button_pressed())
      total_cnt ++;
    return total_cnt;
}
#define mdelay(_ms) udelay(_ms*1000)


int do_cboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    uint32_t key_mode = 0;
    uint32_t key_code = 0;
    volatile int i;

    if(argc > 2)
      goto usage;

#ifdef CONFIG_AUTOBOOT
	normal_mode();
#endif

    boot_pwr_check();
	
#ifdef CONFIG_SC8800G
    CHG_ShutDown();
    if(charger_connected()){
        mdelay(10);
        CHG_TurnOn();
    }else{
        if(is_bat_low()){
            printf("shut down again for low battery\n");
            power_down_devices();
            while(1)
              ;
        }
    }
#else
	CHG_Init();
#ifndef CONFIG_MACH_CORI
	if(is_bat_low()){
				printf("shut down again for low battery\n");
				mdelay(10000);
				power_down_devices();
				while(1)
				  ;
	}
#endif    
#endif	

    boot_pwr_check();
    board_keypad_init();
    boot_pwr_check();

    int recovery_init(void);
    int ret =0;
    ret = recovery_init();
    if(ret == 1){
        DBG("func: %s line: %d\n", __func__, __LINE__);
        recovery_mode_without_update();
    }else if(ret == 2){
	    try_update_modem(); //update img from mmc
	    normal_mode();
    }

    unsigned check_reboot_mode(void);
    unsigned rst_mode= check_reboot_mode();
#ifdef CONFIG_SPRD_SYSDUMP
    write_sysdump_before_boot(rst_mode);
#endif
    if(rst_mode == RECOVERY_MODE){
        DBG("func: %s line: %d\n", __func__, __LINE__);
        recovery_mode();
    }
    else if(rst_mode == FASTBOOT_MODE){
        DBG("func: %s line: %d\n", __func__, __LINE__);
        fastboot_mode();
    }else if(rst_mode == NORMAL_MODE){
        normal_mode();
    }else if(rst_mode == WATCHDOG_REBOOT){
        watchdog_mode();
    }else if(rst_mode == UNKNOW_REBOOT_MODE){
        unknow_reboot_mode();
    }else if(rst_mode == PANIC_REBOOT){
        panic_reboot_mode();
    }else if(rst_mode == ALARM_MODE){
              int flag =alarm_flag_check();
              if(flag == 1)
			alarm_mode();
              else if(flag == 2)
			normal_mode();
    }else if(rst_mode == SLEEP_MODE){
		sleep_mode();
    }else if(rst_mode == SPECIAL_MODE){
	        special_mode();
	}
#ifdef CONFIG_SC8810
//    normal_mode();
#endif
    DBG("func: %s line: %d\n", __func__, __LINE__);

   if(charger_connected()){
        DBG("%s: charger connected\n", __FUNCTION__);
#if defined (CONFIG_SP8810W)
        calibration_detect(1);
#endif
        #if defined(CONFIG_FASTBOOT_KEY)
        key_code = board_key_scan();
        printf("fastboot key(%d)\n",key_code);
        if (key_code == CONFIG_FASTBOOT_KEY)
            fastboot_mode();
        else
        #endif
            charge_mode();
    }
    //find the power up trigger
    else if(boot_pwr_check() >= get_pwr_key_cnt()){
        DBG("%s: power button press\n", __FUNCTION__);
	printf("boot_pwr_check=%d,get_pwr_key_cnt=%d\n",boot_pwr_check(),get_pwr_key_cnt());
        //go on to check other keys
        mdelay(50);
        for(i=0; i<10;i++){
            key_code = board_key_scan();
            if(key_code != KEY_RESERVED)
              break;
        }
        printf("key_code %d\n", key_code);
        key_mode = check_key_boot(key_code);

        printf("key_mode %d\n", key_mode);
        switch(key_mode){
            case BOOT_FASTBOOT:
                fastboot_mode();
                break;
            case BOOT_RECOVERY:
                recovery_mode();
                break;
            case BOOT_CALIBRATE:
                engtest_mode();
                return 0; //back to normal boot
                break;
            case BOOT_DLOADER:
                dloader_mode();
                break;
            default:
                break;
        }
    }
    else if(alarm_triggered() && alarm_flag_check()){
        DBG("%s: alarm triggered\n", __FUNCTION__);
        int flag =alarm_flag_check();

        if(flag == 1){
			alarm_mode();
        }
        else if(flag == 2){
			normal_mode();
        }
		
    }else{
#if BOOT_NATIVE_LINUX_MODEM
        *(volatile u32*)CALIBRATION_FLAG = 0xca;
#endif
        calibration_detect(0);
        //if calibrate success, it will here
        DBG("%s: power done again\n", __FUNCTION__);
        power_down_devices();
        while(1)
          ;
    }

    if(argc == 1){
	DBG("func: %s line: %d\n", __func__, __LINE__);
        normal_mode();
        return 1;
    }

    if(argc == 2){
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"normal") == 0){
            normal_mode();
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"recovery") == 0){
            recovery_mode();
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"fastboot") == 0){
            fastboot_mode();
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"dloader") == 0){
            dloader_mode();
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"charge") == 0){
            charge_mode();
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);

        if(strcmp(argv[1],"caliberation") == 0){
            calibration_detect(1);
            return 1;
        }
        DBG("func: %s line: %d\n", __func__, __LINE__);
    }
    DBG("func: %s line: %d\n", __func__, __LINE__);

usage:
    cmd_usage(cmdtp);
    return 1;
}

U_BOOT_CMD(
            cboot, CONFIG_SYS_MAXARGS, 1, do_cboot,
            "choose boot mode",
            "mode: \nrecovery, fastboot, dloader, charge, normal, vlx, caliberation.\n"
            "cboot could enter a mode specified by the mode descriptor.\n"
            "it also could enter a proper mode automatically depending on "
            "the environment\n"
          );
