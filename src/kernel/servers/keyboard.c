/*================================================================================*/
/*                       ICT Perfect 2.00 keyboard driver                         */
/*                                                                        by: ict */
/*================================================================================*/

#include	"constent.h"
#include	"type.h"
#include	"public.h"
#include	"sig.h"
#include	"kproc.h"
#include    "../io/i8042.h"
#include	"../io/BIOScolor.h"

#define SINGLE_KEY_RQ	0
#define GROUP_KEY_RQ	1
#define GRQ_NULL	-1
#define NO_KEY		0x0

KEYBOARDBUFFER ict_kb_buf;	/* system keyboard buffer */

void int_keyboard();
void keyboard_daemon();
extern void keyboard_interrupt();

/******************************************************************/
/* init the keyboard buf, link all node as a linklist             */
/******************************************************************/
void init_kb()	/* init the keyboard buffer: ict_kb_buf */
{
	//ict_printf(COLOR_YELLOW, "Setup keyboard service...       ");
	ict_kb_buf.used_size = 0;	/* no char in this buffer */
	ict_kb_buf.free_node = &(ict_kb_buf.buffer_node[0]);	/* next free node location */
	ict_kb_buf.queue_entry = &(ict_kb_buf.buffer_node[0]);	/* keys queue entry */
	/* init the first node special, let it back point to the last node */
	ict_kb_buf.buffer_node[0].back = &(ict_kb_buf.buffer_node[KEYBOARD_BUFFER_SIZE - 1]);
	ict_kb_buf.buffer_node[0].next = &(ict_kb_buf.buffer_node[1]);
	int i = 0;	/* for next loop */
	for(i = 1; i < KEYBOARD_BUFFER_SIZE - 1; i++)	/* link the 1 to KEYBOARD_BUFFER_SIZE - 2 nodes */
	{
		ict_kb_buf.buffer_node[i].back = &(ict_kb_buf.buffer_node[i - 1]);
		ict_kb_buf.buffer_node[i].next = &(ict_kb_buf.buffer_node[i + 1]);
	}
	/* init the last node special, let it next point to the first node */
	ict_kb_buf.buffer_node[KEYBOARD_BUFFER_SIZE - 1].back = &(ict_kb_buf.buffer_node[KEYBOARD_BUFFER_SIZE - 2]);
	ict_kb_buf.buffer_node[KEYBOARD_BUFFER_SIZE - 1].next = &(ict_kb_buf.buffer_node[0]);
	ict_setupint(0x21, keyboard_interrupt);	/* setup the keyboard interrupt handle */
	//ict_printf(COLOR_YELLOW, "[ OK ]\n");
}

/******************************************************************/
/* get a scan code of a key from keyboard buf                     */
/******************************************************************/
char get_key()	/* get one key scan code from buffer */
{
	if(ict_kb_buf.used_size == 0)	/* if the buffer is empty */
		return NO_KEY;	/* return the 0xff(-1) means no keys */
	char sc = ict_kb_buf.queue_entry->scan_code;	/* get one scan code */
	ict_kb_buf.queue_entry = ict_kb_buf.queue_entry->next;	/* queue entry move to next node */
	ict_kb_buf.used_size--;	/* buffer used size decrease */
	return sc;	/* return the scan code */
}

/******************************************************************/
/* keyboard interrupt handle function                             */
/******************************************************************/
void int_keyboard()	/* keyboard interrupt handle function */
{
	char sc = ict_in(I8042_OUTPUT_BUFFER);	/* get a key from i8042 */
	if(ict_kb_buf.used_size < KEYBOARD_BUFFER_SIZE)	/* if the buffer is full */
	{
		ict_kb_buf.free_node->scan_code = sc;	/* load the sc to node */
		ict_kb_buf.free_node = ict_kb_buf.free_node->next;	/* free node move to next node */
		ict_kb_buf.used_size++;	/* increase the buffer used size */
	}
}

/******************************************************************/
/* keyboard daemon, the only way to get a key is to ask this proc */
/******************************************************************/
void keyboard_daemon()
{
	MSG m;	/* msg space */
	int state = SINGLE_KEY_RQ;	/* the daemon init works as single key mode */
	int grq_id = GRQ_NULL;	/* the group keys required proc */
	DWORD sc;	/* scan code */
	DWORD temp_sc;	/* temp scan code, for some key have more than one byte scan code */
	while(TRUE)	/* the daemon loop */
	{
		while((sc = get_key() & 0xff) == NO_KEY);	/* wait a key */
		if(sc == 0xe0)	/* if the key have more than one byte scan code */
		{
			while((temp_sc = get_key() & 0xff) == NO_KEY);	/* get the next byte */
			sc = (sc * 0x100) & 0xff00 | temp_sc & 0xff;	/* combine the two bytes */
		}
		if(state == GROUP_KEY_RQ)	/* if the state is group require */
			send_msg(grq_id, sc, NULL);	/* send the key scan code directly */
		if(have_msg())	/* if the daemon have msg */
		{
			read_msg(&m);	/* read this msg */
			switch(m.sig)	/* handle this msg */
			{
				case KB_KEY :	/* one proc require a key */
					send_msg(m.sproc_id, sc, NULL, NULL);	/* send this key to him */
					break;
				case KB_KEYS :	/* one proc require a group of keys */
					if(grq_id == GRQ_NULL)	/* if there are no proc do this */
					{
						grq_id = m.sproc_id;	/* set the group require proc id */
						state = GROUP_KEY_RQ;	/* the daemon works mode change */
						send_msg(grq_id, sc, NULL, NULL);	/* send this key to it */
					}
					break;
				case KB_KEYS_END :	/* the group require is end */
					if(state == GROUP_KEY_RQ)	/* if the daemon works mode is GRQ */
					{
						state = SINGLE_KEY_RQ;	/* change the state */
						grq_id = GRQ_NULL;	/* clear the group require proc id */
					}
					break;
			}
		}
	}
}
