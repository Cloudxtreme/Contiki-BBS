/**
 * \file
 *         bbs-setupfuncs.c - setup functions for the Contiki BBS setup program
 * \author
 *         (c) 2009-2015 by Niels Haedecke <n.haedecke@unitybox.de>
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

      printf("Netmask             : ");
      gets(tmp);
      nibbleIP(tmp, mycnf.netmask);

      printf("Gateway IP          : "); 
      gets(tmp);
      nibbleIP(tmp, mycnf.gateway);

      printf("DNS IP              : ");
      gets(tmp);
      nibbleIP(tmp, mycnf.nameserv);

      printf("Mem addr. ($de08)   : ");
      gets(tmp);
      sscanf(tmp, "%x", &mycnf.mem);

      printf("Driver (cs8900a.eth): ");
      gets(tmp);
      strcpy(mycnf.driver, tmp);

      printf("Write to drive # (8): ");
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
  char buff[4];
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

      printf("Enter BBS prompt: ");
      gets(cfg.bbs_prompt);

      if (strlen(cfg.bbs_prompt) < 1) {
         strcpy(cfg.bbs_prompt, "Contiki BBS> ");
      }

      printf("Login timeout (seconds): ");
      gets(buff);
      sscanf(buff, "%d", &cfg.bbs_timeout_login);

      printf("Session timeout (seconds): ");
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

         printf("\nBoard name: ");
         gets(board.board_name);

         printf("Max. msgs. : ");
         gets(buff);
         sscanf(buff, "%d", &board.board_max);

         printf("Access lvl.: ");
         gets(buff);
         sscanf(buff, "%u", &board.access_req);

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

int enterUserData(BBS_USER_REC *rec, unsigned short *user_count) {
   char input, buff[4];
   unsigned short num;

             do {
                printf("\nUser #  : %03d",  *user_count);
                rec->user_no = *user_count;

                printf("\nUsername: ");
                gets(rec->user_name);

                printf("Password: ");
                gets(rec->user_pwd);

                printf("Access lvl.: ");
                gets(buff);
                sscanf(buff, "%u", &num);
                rec->access_req=num;

                printf("\nUser data correct (y/n)? ");
                input=getchar();

          } while (input != 'y');
   return 0;
}

int readUserData(BBS_USER_REC *rec, ST_FILE *file, unsigned short *count) {
   strcpy(file->szFileName, "user.dat");
   ssReadRELFile(file, rec, sizeof(BBS_USER_REC), *count);
   return 0;
}

int readUserIndex(unsigned short *count, ST_FILE *file) {
  strcpy(file->szFileName, "user.idx");
  ssReadRELFile(file, count, sizeof(unsigned short), 1);
  return 0;
}

int writeUserData(BBS_USER_REC *rec, ST_FILE *file, unsigned short *count) {
   strcpy(file->szFileName, "user.dat");
   ssWriteRELFile(file, rec, sizeof(BBS_USER_REC), *count);
   return 0;
}

int writeUserIndex(unsigned short *count, ST_FILE *file) {
  strcpy(file->szFileName, "user.idx");
  ssWriteRELFile(file, count, sizeof(unsigned short), 1);
  return 0;
}

int userSetup(unsigned short drive) {
   unsigned short count=0,user_count=0;
   char input,buff[20];

   BBS_USER_REC mybbsuser;
   ST_FILE file;

   /* enter users */
   printf("\n* BBS user editor\n");

   strcpy(file.szFileName, "user.idx");
   file.ucDeviceNo = drive;

   if(siFileExists(&file) == 62 /*|| user_count == 0*/) {
     user_count = 1;
     memset(&mybbsuser, 0x20, sizeof(BBS_USER_REC));

     printf("\n* Initializing user databases...\n");
     strcpy(file.szFileName, "user.dat");
     ssInitRELFile(&file, &mybbsuser, sizeof(BBS_USER_REC), BBS_MAX_USERS);
     strcpy(file.szFileName, "user.idx");
     ssInitRELFile(&file, &user_count, sizeof(unsigned short), 1);
     writeUserIndex(&user_count, &file);
   }

   printf("\n(A)dd, (E)dit or (L)list users? ");
   input=getchar();

   switch (input) {

      case 'a':
      case 'A': {
          readUserIndex(&user_count, &file);
          enterUserData(&mybbsuser, &user_count);
          writeUserData(&mybbsuser, &file, &user_count);
          user_count += 1;
          writeUserIndex(&user_count, &file);
      }
      break;

      case 'e':
      case 'E': {
             printf("\n\nEnter user #: ");
             gets(buff);
             sscanf(buff, "%d", &count);
             readUserData(&mybbsuser, &file, &count);
             scrollScreen();
             printf("\nUser #     : %03d", count);
             printf("\nUsername   : %s", mybbsuser.user_name);
             printf("\nPassword   : %s\n", mybbsuser.user_pwd);
             printf("\nAccess lvl.: %03d\n", mybbsuser.access_req);
             enterUserData(&mybbsuser, &count);
             writeUserData(&mybbsuser, &file, &count);
      }
      break;

      case 'l':
      case 'L': {
             count = 1;
             scrollScreen();
             printf("\n\nID# |Acc. | Username");
             printf("\n----+-----+----------------------------");

             readUserIndex(&user_count, &file);

             do {
                readUserData(&mybbsuser, &file, &count);
                printf("\n%03d | %03d | %s",  count, mybbsuser.access_req, mybbsuser.user_name);
                count++;
             } while (count < user_count);

             printf("\n\nPress key"); 
             input=getchar();
      }   
      break;

   }
   return 0;
}
