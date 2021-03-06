#ifndef _SC8800H_REG_TIMER_H_
    #define _SC8800H_REG_TIMER_H_

#ifdef   __cplusplus
    extern   "C" 
    {
#endif

#define TIMER_CTL_BASE          		0x81000000

#define TIMER0_BASE             		(TIMER_CTL_BASE + 0x0000)
#define TM0_LOAD                		(TIMER0_BASE + 0x0000)          //Write to this register will reload the timer with the new value.
#define TM0_VALUE               		(TIMER0_BASE + 0x0004)          //Return the current timer value.
#define TM0_CTL                 		(TIMER0_BASE + 0x0008)
#define TM0_CLR                 		(TIMER0_BASE + 0x000C)          //Write to this register will clear the interrupt generated by this timer.

#define TIMER1_BASE             		(TIMER_CTL_BASE + 0x0020)
#define TM1_LOAD                		(TIMER1_BASE + 0x0000)          //Write to this register will reload the timer with the new value.
#define TM1_VALUE               		(TIMER1_BASE + 0x0004)          //Return the current timer value.
#define TM1_CTL                 		(TIMER1_BASE + 0x0008)
#define TM1_CLR                 		(TIMER1_BASE + 0x000C)          //Write to this register will clear the interrupt generated by this timer.

//The corresponding bit of TIMER0_CTL/TIMER1_CTL register.
#define TMCTL_CLK1              		(0)             //prescale timer clk (apb clk) by 1
#define TMCTL_MODE_B            		(1 << 6)        //
#define TMCTL_EN                		(1 << 7)        //enable bit of this timer.
#define TIMER_MAX_VALUE		0x7FFFFF

//Timer Control.
typedef struct timer_tag
{
    volatile uint32_t load;
    volatile uint32_t value;
    volatile uint32_t ctl;
    volatile uint32_t clr;
}timer_s;

#ifdef   __cplusplus
    }
#endif
#endif
// End
