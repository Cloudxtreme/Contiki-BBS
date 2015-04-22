/**
 * \file
 *         bbs-setupfuncs.c - setup functions for the Contiki BBS setup program
 * \author
 *         (c) 2009-2011 by Niels Haedecke <n.haedecke@unitybox.de>
 */

#include "bbs-setup.h"

void scrollScreen(void) {
   unsigned short count=0;

   /* a little effect */
   for (count=0; count < 25; count++)
       putchar('\n');
}

void mainMenu(void) {

   printf("\n*** Contiki BBS %s setup\n", BBS_STRING_VERSION);
   printf("\n1...BBS base setup");
   printf("\n2...BBS board setup");
   printf("\n3...BBS TCP/IP setup");
   printf("\n4...BBS user editor");
   printf("\nq...Quit");
   printf("\n\n> ");

}

int nibbleIP(unsigned char *src, unsigned int *addr)
{
   unsigned char *phlp=NULL;
   unsigned int  pass=0;

   phlp = src;

   do 
   {  
     if (*phlp != '.' || '\0') 
     {
        ++phlp;
     }
     else
     {
        *phlp='\0';

        *addr = atoi(src);
        ++phlp;
        ++addr;
        src = phlp;
     }             
   } while ( *phlp != '\0');

   *addr = atoi(src);

   return 0; 
}

int networkSetup(unsigned short drive)
{
   unsigned char  tmp[40];
   unsigned short cnt, ret;

   CTK_CFG_REC mycnf;

   memset(&mycnf, 0, sizeof(CTK_CFG_REC));
   do {
 
/*             1234567890123456789012345678901234567890   */
      printf("\n* BBS network Setup\n");

      printf("\nHost IP             : ");
      gets(tmp);
      nibbleIP(tmp, mycnf.srvip);

      printf("\nNetmask             : ");
      gets(tmp);
      nibbleIP(tmp, mycnf.netmask);

      printf("\nGateway IP          : "); 
      gets(tmp);
      nibbleIP(tmp, mycnf.gateway);

      printf("\nDNS IP              : ");
      gets(tmp);
      nibbleIP(tmp, mycnf.nameserv);

      printf("\nMem addr. ($de08)   : ");
      gets(tmp);
      sscanf(tmp, "%x", &mycnf.mem);

      printf("\nDriver (cs8900a.eth): ");
      gets(tmp);
      strcpy(mycnf.driver, tmp);

      printf("\nWrite to drive # (8): ");
      gets(tmp);
      sscanf(tmp, "%d", &drive);

      printf("\nNetwork data correct (y/n)? ");

   } while (getchar() != 'y');

   strcpy(tmp, "@0:contiki.cfg,u,w");

   ret = cbm_open(2, drive, CBM_WRITE, tmp);

   if(! ret)
   {
        for(cnt = 0; cnt <= 3; cnt++)
           cbm_write(2, &mycnf.srvip[cnt], sizeof(unsigned char));

        for(cnt = 0; cnt <= 3; cnt++)
           cbm_write(2, &mycnf.netmask[cnt], sizeof(unsigned char));

        for(cnt = 0; cnt <= 3; cnt++) 
          cbm_write(2, &mycnf.gateway[cnt], sizeof(unsigned char));

        for(cnt = 0; cnt <= 3; cnt++)
           cbm_write(2, &mycnf.nameserv[cnt], sizeof(unsigned char));

        cbm_write(2, &mycnf.mem, sizeof(&mycnf.mem));

        cbm_write(2, mycnf.driver, sizeof(mycnf.driver));

   } else {

      printf("\n\r**error - can't write file: 'contiki.cfg'");      
   }

   cbm_close(2);
   
   return ret;
}

int baseSetup(void) {
  char input;
  char buff[20];
  ST_FILE file;
  BBS_STATUS_REC cfg;

  memset(&cfg, 0, sizeof(BBS_STATUS_REC));

   /* enter BBS base configuration */
  printf("\n* BBS base setup\n");

   do {
      printf("\nEnter board drive #: ");
      gets(buff);
      sscanf(buff, "%d", &cfg.board_drive);

      /*printf("\nEnter BBS name: ");
      gets(cfg.bbs_name);

      printf("\nEnter sysop name: ");
      gets(cfg.bbs_sysop);*/

      printf("\nEnter BBS prompt: ");
      gets(cfg.bbs_prompt);

      if (strlen(cfg.bbs_prompt) < 1) {
         strcpy(cfg.bbs_prompt, "Contiki BBS> ");
      }

      printf("\nLogin timeout (seconds): ");
      gets(buff);
      sscanf(buff, "%d", &cfg.bbs_timeout_login);

      printf("\nSession timeout (seconds): ");
      gets(buff);
      sscanf(buff, "%d", &cfg.bbs_timeout_session);

      printf("\nBase data correct (y/n)? ");
      input=getchar();
   } while (input != 'y');

   /* write BBS base configuration */
   strcpy(file.szFileName, BBS_CFG_FILE);  
   file.ucDeviceNo = cfg.board_drive;
   ssWriteRELFile(&file, &cfg, sizeof(cfg), 1);

   return cfg.board_drive;
}

int boardSetup(unsigned short drive) {
   unsigned short num,count=0;
   char input;
   char buff[20];
   char *msgbuff;

   ST_FILE file;
   BBS_BOARD_REC board;

   /* enter board base configuration */
   count = 1;
   printf("\n* BBS board setup\n");
   printf("\nHow many boards ? ");
   gets(buff);
   sscanf(buff, "%d", &num);
   strcpy(file.szFileName, "board.cfg");
   memset(&board, 0, sizeof(BBS_BOARD_REC));
   file.ucDeviceNo=drive;
   ssInitRELFile(&file, &board, sizeof(BBS_BOARD_REC), num);

   do {

      do {
         board.max_boards=num;
         board.board_no=count;
         board.access_req=0;
         board.board_ptr=0;

         printf("\n\nBoard #%d", count);

         printf("\nEnter board name: ");
         gets(board.board_name);

         printf("\nMaximum messages for board : ");
         gets(buff);
         sscanf(buff, "%d", &board.board_max);

         printf("\nBoard data correct (y/n)? ");
         input=getchar();
 
      } while (input != 'y');
 
      /* write board configuration */
      strcpy(file.szFileName, "board.cfg");
      file.ucDeviceNo=drive;
      ssWriteRELFile(&file, &board, sizeof(BBS_BOARD_REC), count);

      msgbuff = (char*) malloc((BBS_MAX_MSGLINES*BBS_LINE_WIDTH) * sizeof(char));

      if (msgbuff == NULL) {
         printf("ERR: malloc failed!");
         exit(1);
      } else {
        sprintf(buff,"board%d.msg", count);
        strcpy(file.szFileName, buff);
        ssInitRELFile(&file, msgbuff, (BBS_MAX_MSGLINES*BBS_LINE_WIDTH) * sizeof(char), board.board_max);
        siDriveStatus(&file); 
        free(msgbuff);
        msgbuff=NULL;
      }

      count++;
   } while (count <= board.max_boards);

   return 0;
}

int userSetup(unsigned short drive) {
   int user_count=0;
   char input;

   BBS_USER_REC mybbsuser;
   ST_FILE file;

   /* enter users */
   printf("\n* BBS user editor\n");

   strcpy(file.szFileName, "user.idx");
   file.ucDeviceNo = drive;

   if(siFileExists(&file) == 62 /*|| user_count == 0*/) {
     user_count = 1;
     memset(&mybbsuser, 0x20, sizeof(BBS_USER_REC));
     strcpy(file.szFileName, "user.dat");
     printf("\n* Initializing user database...\n");
     ssInitRELFile(&file, &mybbsuser, sizeof(BBS_USER_REC), BBS_MAX_USERS);
   } else {
     strcpy(file.szFileName, "user.idx");
     ssReadRELFile(&file, &user_count, sizeof(int), 1);
     user_count += 1;
   }

   do {
      printf("\nUser #  : %03d",  user_count);
      mybbsuser.user_no = user_count;
      
      printf("\nUsername: ");
      gets(mybbsuser.user_name);

      printf("\nPassword: ");
      gets(mybbsuser.user_pwd);

      printf("\nUser data correct (y/n)? ");
      input=getchar();

   } while (input != 'y');

   strcpy(file.szFileName, "user.dat");
   ssWriteRELFile(&file, &mybbsuser, sizeof(BBS_USER_REC), user_count);

   strcpy(file.szFileName, "user.idx");
   file.ucDeviceNo = drive;
   ssWriteRELFile(&file, &user_count, sizeof(int), 1);

   return 0;
}
