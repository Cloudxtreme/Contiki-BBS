/**
 * \file
 *         bbs-setboard.c - select Contiki BBS message boards 
 *
 * \author
 *         (c) 2009-2011 by Niels Haedecke <n.haedecke@unitybox.de>
 */


#include "contiki.h"
#include "shell.h"

#include "bbs-setboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern BBS_STATUS_REC bbs_status;

/*PROCESS(bbs_setboard_process, "setboard");*/
/*SHELL_COMMAND(bbs_setboard_command, "setboard", "setboard: set active board", &bbs_setboard_process);*/
PROCESS(bbs_setboard_process, "board");
SHELL_COMMAND(bbs_setboard_command, "board", "board  : select active board", &bbs_setboard_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(bbs_setboard_process, ev, data)
{

  struct shell_input *input;
  char szBuff[BBS_LINE_WIDTH];
  ST_FILE file;
  BBS_BOARD_REC board;

  PROCESS_BEGIN();

  /* read board data */
  strcpy(file.szFileName, BBS_BOARDCFG_FILE);
  file.ucDeviceNo=8;
  ssReadRELFile(&file, &board, sizeof(BBS_BOARD_REC), bbs_status.bbs_board_id);

  sprintf(szBuff, "{%s (%d)} board # (1-%d, 0=quit)? ", board.board_name, board.board_no, board.max_boards);
  /*sprintf(szBuff, "current board: %d, which 1 - %d (0=quit)? ", bbs_status.bbs_board_id, board.max_boards);*/
  shell_prompt(szBuff);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(atoi(input->data1) < 1 || atoi(input->data1) > board.max_boards) {
      shell_prompt("");
      PROCESS_EXIT();
    } else {
      bbs_status.bbs_board_id = atoi(input->data1);
      shell_prompt("");
      PROCESS_EXIT();
    }
   
  } /* end ... while */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
bbs_setboard_init(void)
{
  shell_register_command(&bbs_setboard_command);
}
