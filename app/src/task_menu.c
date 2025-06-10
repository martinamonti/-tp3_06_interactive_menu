/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_XX_IDLE, EV_MEN_ENT_IDLE, false};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;
task_menu_prop_t task_motores_dta[CANT_MOTORES];

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	displayInit( DISPLAY_CONNECTION_GPIO_4BITS );

	char M1[16];
	char M2[16];
    displayCharPositionWrite(0, 0);
	snprintf(M1, sizeof(M1), "Motor1: %d, %ld, %d", (task_motores_dta[0].power),(task_motores_dta[0].speed), (task_motores_dta[0].spin));
	displayCharPositionWrite(0, 0);
	displayStringWrite(M1);
	snprintf(M2, sizeof(M2), "Motor2: %d, %ld, %d", (task_motores_dta[1].power),(task_motores_dta[1].speed), (task_motores_dta[1].spin));
	displayCharPositionWrite(0, 1);
	displayStringWrite(M2);


	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	bool b_time_update_required = false;
	char menu_str[8];

	/* Update Task Menu Counter */
	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_menu_dta = &task_menu_dta;

    	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
		{
			p_task_menu_dta->tick--;
		}
		else
		{
			snprintf(menu_str, sizeof(menu_str), "%lu", (g_task_menu_cnt/1000ul));
			displayCharPositionWrite(10, 1);
			displayStringWrite(menu_str);

			p_task_menu_dta->tick = DEL_MEN_XX_MAX;

			if (true == any_event_task_menu())
			{
				p_task_menu_dta->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}

			switch (p_task_menu_dta->state)
			{
				case ST_MEN_XX_IDLE:

					if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_ACTIVE;
					}

					break;

				case ST_MEN_XX_ACTIVE:

					if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_IDLE == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_IDLE;
					}

					break;
				case ST_MEN_XX_MAIN:
					if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('>Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite(' Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_MOTOR;
					}
				case ST_MEN_XX_MOTOR:
					if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), ">Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_POWER;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event) && (0 == p_task_menu_dta->motor_num))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite(' Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite('>Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_num = 1;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event) && (1 == p_task_menu_dta->motor_num))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('>Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite(' Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_num = 0;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						char M1[16];
						char M2[16];
						snprintf(M1, sizeof(M1), "Motor1: %d, %ld, %d", (task_motores_dta[0].power),(task_motores_dta[0].speed), (task_motores_dta[0].spin));
						displayCharPositionWrite(0, 0);
						displayStringWrite(M1);
						snprintf(M2, sizeof(M2), "Motor2: %d, %ld, %d", (task_motores_dta[1].power),(task_motores_dta[1].speed), (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(M2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_MAIN;
					}
				case ST_MEN_XX_POWER:
					if((true == p_task_menu_dta->flag)&& (EV_MEN_ENT == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('     Power      ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('   False >True  ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_POWER_MENU;
						p_task_menu_dta->motor_borrador.power = true;
					}else if((true == p_task_menu_dta->flag)&& (EV_MEN_NEX == p_task_menu_dta->event))
					{
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d,>Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPEED;
					}else if((true == p_task_menu_dta->flag)&& (EV_MEN_ESC == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('>Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite(' Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_MOTOR;
					}
				case ST_MEN_XX_POWER_MENU:
					if((true == p_task_menu_dta->flag)&& (EV_MEN_NEX == p_task_menu_dta->event) && (true == p_task_menu_dta->motor_borrador.power))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('     Power      ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('  >False  True  ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_borrador.power = false;
					}else if((true == p_task_menu_dta->flag)&& (EV_MEN_NEX == p_task_menu_dta->event) && (false == p_task_menu_dta->motor_borrador.power))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('     Power      ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('   False >True  ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_borrador.power = true;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_POWER;
						task_motores_dta[p_task_menu_dta->motor_num].power = p_task_menu_dta->motor_borrador.power;
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), ">Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_POWER;
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), ">Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
					}
				case ST_MEN_XX_SPEED:
					if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('     Speed     ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('       >0       ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPEED_MENU;
						p_task_menu_dta->motor_borrador.speed = 0;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event))
					{
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "     >Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPIN;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('>Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite(' Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_MOTOR;
					}
				case ST_MEN_XX_SPEED_MENU:
					if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_borrador.speed = (p_task_menu_dta->motor_borrador.speed + 1)%10;
						char V1[16];
						snprintf(V1, sizeof(V1), "       >%d       ", (task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite('     Speed     ');
						displayCharPositionWrite(0, 1);
						displayStringWrite(V1);
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d,>Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPEED;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPEED;
						task_motores_dta[p_task_menu_dta->motor_num].speed = p_task_menu_dta->motor_borrador.speed;
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d,>Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
					}
				case ST_MEN_XX_SPIN:
					if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite(' Spin RIGHT');
						displayCharPositionWrite(0, 1);
						displayStringWrite('>Spin LEFT');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPIN_MENU;
						p_task_menu_dta->motor_borrador.spin = LEFT;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('>Motor 1');
						displayCharPositionWrite(0, 1);
						displayStringWrite(' Motor 2');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_MOTOR;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event))
					{
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), ">Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "     Spin:%d    ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_POWER;
					}
				case ST_MEN_XX_SPIN_MENU:
					if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event) && (LEFT == p_task_menu_dta->motor_borrador.spin))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('      Spin      ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('   >Right Left  ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_borrador.spin = RIGHT;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_NEX == p_task_menu_dta->event) && (RIGHT == p_task_menu_dta->motor_borrador.spin))
					{
						displayCharPositionWrite(0, 0);
						displayStringWrite('      Spin      ');
						displayCharPositionWrite(0, 1);
						displayStringWrite('   Right >Left  ');
						p_task_menu_dta->flag = false;
						p_task_menu_dta->motor_borrador.spin = LEFT;
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ENT == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPIN;
						task_motores_dta[p_task_menu_dta->motor_num].spin =p_task_menu_dta->motor_borrador.spin;
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "      >Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
					}else if((true == p_task_menu_dta->flag) && (EV_MEN_ESC == p_task_menu_dta->event))
					{
						p_task_menu_dta->flag = false;
						p_task_menu_dta->state = ST_MEN_XX_SPIN;
						char F1[16];
						char F2[16];
						snprintf(F1, sizeof(F1), " Power:%d, Speed:%ld", (task_motores_dta[0].power),(task_motores_dta[0].speed));
						displayCharPositionWrite(0, 0);
						displayStringWrite(F1);
						snprintf(F2, sizeof(F2), "     >Spin:%d     ", (task_motores_dta[1].spin));
						displayCharPositionWrite(0, 1);
						displayStringWrite(F2);
					}
				default:

					p_task_menu_dta->tick  = DEL_MEN_XX_MIN;
					p_task_menu_dta->state = ST_MEN_XX_IDLE;
					p_task_menu_dta->event = EV_MEN_ENT_IDLE;
					p_task_menu_dta->flag  = false;

					break;
			}
		}
	}
}


/********************** end of file ******************************************/
