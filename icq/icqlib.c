/*
$Id$
$Log$
Revision 1.1  1999/09/28 14:31:28  cafuego
*** empty log message ***

Revision 1.16  1998/12/08 16:00:59  denis
Cleaned up a little before releasing

Revision 1.15  1998/11/25 19:18:16  denis
Added close icq_ProxySok in icq_Disconnect

Revision 1.14  1998/11/25 09:48:49  denis
icq_GetProxySok and icq_HandleProxyResponse methods added
Connection terminated support added

Revision 1.13  1998/11/19 12:22:48  denis
SOCKS support cleaned a little
icq_RecvUrl renamed to icq_RecvURL
icq_ProxyAuth added for Username/Password Authentication
URL/Description order inverted
icq_Quit splitted to icq_Logout and icq_Disconnect
icq_ProxyName and icq_ProxyPass range checking added

Revision 1.12  1998/11/18 16:21:29  denis
Fixed SOCKS5 proxy support

 */

#include "icq.h"
#include "icqlib.h"

#include <stdio.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>

/*#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>*/

int icq_Sok;
BOOL icq_Russian = TRUE;
BYTE icq_ServMess[8192]; /* 65536 seqs max, 1 bit per seq -> 65536/8 = 8192 */
WORD icq_SeqNum = 1;
DWORD icq_OurIp = 0x0100007f;
DWORD icq_OurPort;
DWORD icq_Uin;
icq_ContactItem *icq_ContFirst;
DWORD icq_Status = STATUS_OFFLINE;
char *icq_Password = 0L;
BYTE icq_LogLevel = 0;
BYTE icq_UseProxy = 0;
char *icq_ProxyHost = 0L;
WORD icq_ProxyPort = 0;
int icq_ProxyAuth = 0;
char *icq_ProxyName = 0L;
char *icq_ProxyPass = 0L;
int icq_ProxySok;
DWORD icq_ProxyDestHost = 0;
WORD icq_ProxyDestPort = 0;
WORD icq_ProxyOurPort = 0;

void (*icq_Logged)(void);
void (*icq_Disconnected)(void);
void (*icq_RecvMessage)(DWORD uin, BYTE hour, BYTE minute, BYTE day, BYTE month, WORD year, const char *msg);
void (*icq_RecvURL)(DWORD uin, BYTE hour, BYTE minute, BYTE day, BYTE month, WORD year, const char *url, const char *descr);
void (*icq_RecvAdded)(DWORD uin, BYTE hour, BYTE minute, BYTE day, BYTE month, WORD year, const char *nick, const char *first, const char *last, const char *email);
void (*icq_RecvAuthReq)(DWORD uin, BYTE hour, BYTE minute, BYTE day, BYTE month, WORD year, const char *nick, const char *first, const char *last, const char *email, const char *reason);
void (*icq_UserFound)(DWORD uin, const char *nick, const char *first, const char *last, const char *email, char auth);
void (*icq_SearchDone)(void);
void (*icq_UserOnline)(DWORD uin, DWORD status, DWORD ip, DWORD port, DWORD realip);
void (*icq_UserOffline)(DWORD uin);
void (*icq_UserStatusUpdate)(DWORD uin, DWORD status);
void (*icq_InfoReply)(DWORD uin, const char *nick, const char *first, const char *last, const char *email, char auth);
void (*icq_ExtInfoReply)(DWORD uin, const char *city, WORD country_code, char country_stat, const char *state, WORD age, char gender, const char *phone, const char *hp, const char *about);
void (*icq_Log)(time_t time, unsigned char level, const char *str);
void (*icq_SrvAck)(WORD seq);

BYTE kw[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
             144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
             160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
             176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
             254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
             239,255,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
             222,192,193,214,196,197,212,195,213,200,201,202,203,204,205,206,
             207,223,208,209,210,211,198,194,220,219,199,216,221,217,215,218};

BYTE wk[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
             144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
             160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
             176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
             225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
             242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
             193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
             210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209};

static COUNTRY_CODE Country_Codes[] = {
  {"N/A",0 },
  {"USA",1 },
  {"Russia",7 },
  {"Australia",61 },
  {"Denmark",45 },
  {"Sweden",46 },
  {"Norway",47 },
  {"Canada",107 },
  {"Brazil",55 },
  {"UK",0x2c },
  {"Finland", 358 },
  {"Iceland", 354 },
  {"Algeria", 213 },
  {"American Samoa", 684 },
  {"Argentina", 54 },
  {"Aruba", 297 },
  {"Austria", 43 },
  {"Bahrain", 973 },
  {"Bangladesh", 880 },
  {"Belgium", 32 },
  {"Belize", 501 },
  {"Bolivia", 591 },
  {"Cameroon", 237 },
  {"Chile", 56 },
  {"China", 86 },
  {"Columbia", 57 },
  {"Costa Rice", 506 },
  {"Cyprus", 357 },
  {"Czech Republic", 42 },
  {"Ecuador", 593 },
  {"Egypt", 20 },
  {"El Salvador", 503 },
  {"Ethiopia", 251 },
  {"Fiji", 679 },
  {"France", 33 },
  {"French Antilles", 596 },
  {"French Polynesia", 689 },
  {"Gabon", 241 },
  {"German", 49 },
  {"Ghana", 233 },
  {"Greece", 30 },
  {"Guadeloupe", 590 },
  {"Guam", 671 },
  {"Guantanomo Bay", 53 },
  {"Guatemala", 502 },
  {"Guyana", 592 },
  {"Haiti", 509 },
  {"Honduras", 504 },
  {"Hong Kong", 852 },
  {"Hungary", 36 },
  {"India", 91 },
  {"Indonesia", 62 },
  {"Iran", 98 },
  {"Iraq", 964 },
  {"Ireland", 353 },
  {"Israel", 972 },
  {"Italy", 39 },
  {"Ivory Coast", 225 },
  {"Japan", 81 },
  {"Jordan", 962 },
  {"Kenya", 254 },
  {"South Korea", 82 },
  {"Kuwait", 965 },
  {"Liberia", 231 },
  {"Libya", 218 },
  {"Liechtenstein", 41 },
  {"Luxembourg", 352 },
  {"Malawi", 265 },
  {"Malaysia", 60 },
  {"Mali", 223 },
  {"Malta", 356 },
  {"Mexico", 52 },
  {"Monaco", 33 },
  {"Morocco", 212 },
  {"Namibia", 264 },
  {"Nepal", 977 },
  {"Netherlands", 31 },
  {"Netherlands Antilles", 599 },
  {"New Caledonia", 687 },
  {"New Zealand", 64 },
  {"Nicaragua", 505 },
  {"Nigeria", 234 },
  {"Oman", 968 },
  {"Pakistan", 92 },
  {"Panama", 507 },
  {"Papua New Guinea", 675 },
  {"Paraguay", 595 },
  {"Peru", 51 },
  {"Philippines", 63 },
  {"Poland", 48 },
  {"Portugal", 351 },
  {"Qatar", 974 },
  {"Romania", 40 },
  {"Saipan", 670 },
  {"San Marino", 39 },
  {"Saudia Arabia", 966 },
  {"Saipan", 670 },
  {"Senegal", 221},
  {"Singapore", 65 },
  {"Slovakia", 42 },
  {"South Africa", 27 },
  {"Spain", 34 },
  {"Sri Lanka", 94 },
  {"Suriname", 597 },
  {"Switzerland", 41 },
  {"Taiwan", 886 },
  {"Tanzania", 255 },
  {"Thailand", 66 },
  {"Tunisia", 216 },
  {"Turkey", 90 },
  {"Ukraine", 380 },
  {"United Arab Emirates", 971 },
  {"Uruguay", 598 },
  {"Vatican City", 39 },
  {"Venezuela", 58 },
  {"Vietnam", 84 },
  {"Yemen", 967 },
  {"Yugoslavia", 38 },
  {"Zaire", 243 },
  {"Zimbabwe", 263 },
  {"Not entered",0xffff } };

int icq_SockWrite(int sok, const void *buf, size_t count)
{
  char tmpbuf[1024];
  if(!icq_UseProxy)
    return write(sok, buf, count);
  else
  {
    tmpbuf[0] = 0; /* reserved */
    tmpbuf[1] = 0; /* reserved */
    tmpbuf[2] = 0; /* standalone packet */
    tmpbuf[3] = 1; /* address type IP v4 */
    memcpy(&tmpbuf[4], &icq_ProxyDestHost, 4);
    memcpy(&tmpbuf[8], &icq_ProxyDestPort, 2);
    memcpy(&tmpbuf[10], buf, count);
    return write(sok, tmpbuf, count+10)-10;
  }
}

int icq_SockRead(int sok, void *buf, size_t count)
{
  int res;
  char tmpbuf[1024];
  if(!icq_UseProxy)
    return read(sok, buf, count);
  else
  {
    res = read(sok, tmpbuf, count+10);
    memcpy(buf, &tmpbuf[10], res-10);
    return res-10;
  }
}

/****************************************
This must be called every 2 min.
so the server knows we're still alive.
JAVA client sends two different commands
so we do also :)
*****************************************/
void icq_KeepAlive()
{
  net_icq_pak pak;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_KEEP_ALIVE);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  icq_SockWrite(icq_Sok, &pak.head, sizeof(pak.head));

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_KEEP_ALIVE2);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  icq_SockWrite(icq_Sok, &pak.head, sizeof(pak.head));

  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Send Keep Alive packet to the server\n");
}

/**********************************
This must be called to remove
messages from the server
***********************************/
void icq_SendGotMessages()
{
  net_icq_pak pak;
   
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_ACK_MESSAGES);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);

  icq_SockWrite(icq_Sok, &pak.head, sizeof(pak.head));
}

/*************************************
this sends over the contact list
*************************************/
void icq_SendContactList()
{
  net_icq_pak pak;
  char num_used;
  int size;
  char *tmp;
  icq_ContactItem *ptr = icq_ContFirst;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_CONT_LIST);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);

  tmp = pak.data;
  tmp++;
  num_used = 0;
  while(ptr)
  {
    DW_2_Chars(tmp, ptr->uin);
    tmp+=4;
    num_used++;
    ptr = ptr->next;
  }
  pak.data[0] = num_used;
  size = ((int)tmp - (int)pak.data);
  size += sizeof(pak.head);
  icq_SockWrite(icq_Sok, &pak.head, size);
}

void icq_SendNewUser(unsigned long uin)
{
  net_icq_pak pak;
  int size;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_ADD_TO_LIST);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  DW_2_Chars(pak.data, uin);
  size = sizeof(pak.head) + 4;
  icq_SockWrite(icq_Sok, &pak.head, size);
}

/*************************************
this sends over the visible list
that allows certain users to see you
if you're invisible.
*************************************/
void icq_SendVisibleList()
{
  net_icq_pak pak;
  char num_used;
  int size;
  char *tmp;
  icq_ContactItem *ptr = icq_ContFirst;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_INVIS_LIST);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);

  tmp = pak.data;
  tmp++;
  num_used = 0;
  while(ptr)
  {
    if(ptr->vis_list)
    {
      DW_2_Chars(tmp, ptr->uin);
      tmp+=4;
      num_used++;
    }
    ptr = ptr->next;
  }
  if(num_used != 0)
  {
    pak.data[0] = num_used;
    size = ((int)tmp - (int)pak.data);
    size += sizeof(pak.head);
    icq_SockWrite(icq_Sok, &pak.head, size);
  }
}

/**************************************
This sends the second login command
this is necessary to finish logging in.
***************************************/
void icq_SendLogin1()
{
  net_icq_pak pak;
   
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_LOGIN_1);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
   
  icq_SockWrite(icq_Sok, &pak.head, sizeof(pak.head));
}

/************************************************
This is called when a user goes offline
*************************************************/
void icq_HandleUserOffline(srv_net_icq_pak pak)
{
  DWORD remote_uin;
  /* icq_ContactItem *ptr; */
  char buf[256];

  remote_uin = Chars_2_DW(pak.data);
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "User %lu logged off\n", remote_uin);
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_UserOffline)
    (*icq_UserOffline)(remote_uin);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

void icq_HandleUserOnline(srv_net_icq_pak pak)
{
  DWORD remote_uin, new_status, remote_ip, remote_real_ip;
  DWORD remote_port; /* Why Mirabilis used 4 bytes for port? */
  /* icq_ContactItem *ptr; */
  char buf[256];

  remote_uin = Chars_2_DW(pak.data);
  new_status = Chars_2_DW(&pak.data[17]);
  remote_ip = ntohl(Chars_2_DW(&pak.data[4]));
  remote_port = ntohl(Chars_2_DW(&pak.data[8]));
  remote_real_ip = ntohl(Chars_2_DW(&pak.data[12]));
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "User %lu (%s) logged on\n", remote_uin, icq_ConvertStatus2Str(new_status));
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_UserOnline)
    (*icq_UserOnline)(remote_uin, new_status, remote_ip, remote_port, remote_real_ip);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

void icq_StatusUpdate(srv_net_icq_pak pak)
{
  unsigned long remote_uin, new_status;
  /* icq_ContactItem *ptr; */
  char buf[256];

  remote_uin = Chars_2_DW(pak.data);
  new_status = Chars_2_DW(&pak.data[4]);
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "%lu changed status to %s\n", remote_uin, icq_ConvertStatus2Str(new_status));
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_UserStatusUpdate)
    (*icq_UserStatusUpdate)(remote_uin, new_status);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

/************************************
This procedure logins into the server with icq_Uin and pass
on the socket icq_Sok and gives our ip and port.
It does NOT wait for any kind of a response.
*************************************/
void icq_Login(DWORD status)
{
  net_icq_pak pak;
  int size, ret;
  login_1 s1;
  login_2 s2;

  memset(icq_ServMess, FALSE, sizeof(icq_ServMess));
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_LOGIN);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
   
  DW_2_Chars(s1.port, icq_OurPort);
  Word_2_Chars(s1.len, strlen(icq_Password) + 1);

  DW_2_Chars(s2.ip, icq_OurIp);
  DW_2_Chars(s2.status, status);
  Word_2_Chars(s2.seq, icq_SeqNum++);

  DW_2_Chars(s2.X1, LOGIN_X1_DEF);
  s2.X2[0] = LOGIN_X2_DEF;
  DW_2_Chars(s2.X3, LOGIN_X3_DEF);
  DW_2_Chars(s2.X4, LOGIN_X4_DEF);
  DW_2_Chars(s2.X5, LOGIN_X5_DEF);

  memcpy(pak.data, &s1, sizeof(s1));
  size = 6;
  memcpy(&pak.data[size], icq_Password, Chars_2_Word(s1.len));
  size += Chars_2_Word(s1.len);
  memcpy(&pak.data[size], &s2, sizeof(s2));
  size += sizeof(s2);
  ret = icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

/*******************************
This routine sends the aknowlegement cmd to the
server it appears that this must be done after
everything the server sends us
*******************************/
void icq_AckSrv(int seq)
{
  int i;
  net_icq_pak pak;
   
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_ACK);
  Word_2_Chars(pak.head.seq, seq);
  DW_2_Chars(pak.head.uin, icq_Uin);
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Acking\n");
  for(i = 0; i < 6; i++)
    icq_SockWrite(icq_Sok, &pak.head, sizeof(pak.head));
}

void icq_HandleInfoReply(srv_net_icq_pak pak)
{
  char *tmp, *ptr1, *ptr2, *ptr3, *ptr4, buf[256];
  int len;
  DWORD uin;
  WORD seq;
  seq = Chars_2_Word(pak.data);
  uin = Chars_2_DW(&pak.data[2]);
  len = Chars_2_Word(&pak.data[6]);
  ptr1 = &pak.data[8];
  icq_RusConv("wk", ptr1);
  tmp = &pak.data[8 + len];
  len = Chars_2_Word(tmp);
  ptr2 = tmp+2;
  icq_RusConv("wk", ptr2);
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  ptr3 = tmp+2;
  icq_RusConv("wk", ptr3);
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  ptr4 = tmp+2;
  icq_RusConv("wk", ptr4);
  tmp += len + 2;
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "Info reply for %lu\n", uin);
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_InfoReply)
    (*icq_InfoReply)(uin, ptr1, ptr2, ptr3, ptr4, *tmp);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

void icq_HandleExtInfoReply(srv_net_icq_pak pak)
{
  unsigned char *tmp, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
  int len;
  DWORD uin;
  WORD cnt_code, age;
  char cnt_stat, gender, buf[256];
  uin = Chars_2_DW(&pak.data[2]);
  len = Chars_2_Word(&pak.data[6]);
  ptr1 = &pak.data[8];
  icq_RusConv("wk", ptr1);
  cnt_code = Chars_2_Word(&pak.data[8+len]);
  cnt_stat = pak.data[len+10];
  tmp = &pak.data[11 + len];
  len = Chars_2_Word(tmp);
  icq_RusConv("wk", tmp+2);
  ptr2 = tmp+2;
  age = Chars_2_Word(tmp+2+len);
  gender = *(tmp+len+4);
  tmp += len + 5;
  len = Chars_2_Word(tmp);
  icq_RusConv("wk", tmp+2);
  ptr3 = tmp+2;
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  icq_RusConv("wk", tmp+2);
  ptr4 = tmp+2;
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  icq_RusConv("wk", tmp+2);
  ptr5 = tmp+2;
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "Extended info reply for %lu\n", uin);
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_ExtInfoReply)
    (*icq_ExtInfoReply)(uin, ptr1, cnt_code, cnt_stat, ptr2, age, gender, ptr3, ptr4, ptr5);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

void icq_HandleSearchReply(srv_net_icq_pak pak)
{
  char *tmp, *ptr1, *ptr2, *ptr3, *ptr4;
  int len;
  char buf[512];
  DWORD uin;
  uin = Chars_2_DW(&pak.data [2]);
  len = Chars_2_Word(&pak.data[6]);
  ptr1 = &pak.data[8];
  icq_RusConv("wk", ptr1);
  tmp = &pak.data[8 + len];
  len = Chars_2_Word(tmp);
  ptr2 = tmp+2;
  icq_RusConv("wk", ptr2);
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  ptr3 = tmp+2;
  icq_RusConv("wk", ptr3);
  tmp += len + 2;
  len = Chars_2_Word(tmp);
  ptr4 = tmp+2;
  icq_RusConv("wk", ptr4);
  tmp += len + 2;
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
  {
    sprintf(buf, "User found %lu, Nick: %s, First Name: %s, Last Name: %s, EMail: %s, Auth: %s\n", uin, ptr1, ptr2, ptr3, ptr4, *tmp==1?"no":"yes");
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
  }
  if(icq_UserFound)
    (*icq_UserFound)(uin, ptr1, ptr2, ptr3, ptr4, *tmp);
  icq_AckSrv(Chars_2_Word(pak.head.seq));
}

void icq_DoMsg(DWORD type, WORD len, char *data, DWORD uin, BYTE hour, BYTE minute, BYTE day, BYTE month, WORD year)
{
  char *tmp;
  char *ptr1, *ptr2, *ptr3, *ptr4;
  char buf[1024];

  switch(type)
  {
    case USER_ADDED_MESS:
      tmp = strchr(data, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      ptr1 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv ("wk",data);
      ptr2 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv ("wk",data);
      ptr3 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      *tmp = 0;
      icq_RusConv ("wk",data);
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "%lu has added you to their contact list, Nick: %s, "
                "First Name: %s, Last Name: %s, EMail: %s\n", uin, ptr1, ptr2, ptr3, data);
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
      }
      if(icq_RecvAdded)
        (*icq_RecvAdded)(uin, hour, minute, day, month, year, ptr1, ptr2, ptr3, data);
      break;
    case AUTH_REQ_MESS:
      tmp = strchr(data, '\xFE');
      *tmp = 0;
      ptr1 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv ("wk",data);
      ptr2 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv ("wk",data);
      ptr3 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv ("wk",data);
      ptr4 = data;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      tmp++;
      data = tmp;
      tmp = strchr(tmp, '\x00');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv("wk",data);
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "%lu has requested your authorization to be added to "
                "their contact list, Nick: %s, First Name: %s, Last Name: %s, "
                "EMail: %s, Reason: %s\n", uin, ptr1, ptr2, ptr3, ptr4, data);
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
      }
      if(icq_RecvAuthReq)
        (*icq_RecvAuthReq)(uin, hour, minute, day, month, year, ptr1, ptr2, ptr3, ptr4, data);
      break;
    case URL_MESS:
      tmp = strchr(data, '\xFE');
      if(tmp == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
          (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Bad packet!\n");
        return;
      }
      *tmp = 0;
      icq_RusConv("wk",data);
      ptr1 = data;
      tmp++;
      data = tmp;
      icq_RusConv("wk",data);
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "URL received from %lu, URL: %s, Description: %s", uin, ptr1, data);
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
      }
      if(icq_RecvURL)
        (*icq_RecvURL)(uin, hour, minute, day, month, year, data, ptr1);
      break;
    default:
      icq_RusConv ("wk",data);
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "Instant message from %lu:\n%s\n", uin, data);
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
      }
      if(icq_RecvMessage)
        (*icq_RecvMessage)(uin, hour, minute, day, month, year, data);
  }
}

/**********************************
Connects to hostname on port port
hostname can be DNS or nnn.nnn.nnn.nnn
write out messages to the FD aux
***********************************/
int icq_Connect(const char *hostname, int port)
{
  char buf[1024];
  char tmpbuf[256];
  int conct, length, res;
  struct sockaddr_in sin, prsin;  /* used to store inet addr stuff */
  struct hostent *host_struct; /* used in DNS llokup */

  icq_Sok = socket(AF_INET, SOCK_DGRAM, 0);/* create the unconnected socket*/
  if(icq_Sok == -1)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      (*icq_Log)(time(0L), ICQ_LOG_FATAL, "Socket creation failed\n");
    return -1;
  }
  if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
    (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Socket created attempting to connect\n");
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_family = AF_INET; /* we're using the inet not appletalk*/
  sin.sin_port = 0;
  if(bind(icq_Sok, (struct sockaddr*)&sin, sizeof(struct sockaddr))<0)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      (*icq_Log)(time(0L), ICQ_LOG_FATAL, "Can't bind socket to free port\n");
    return -1;
  }
  length = sizeof(sin);
  getsockname(icq_Sok, (struct sockaddr*)&sin, &length);
  icq_ProxyOurPort = sin.sin_port;
  if(icq_UseProxy)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "[SOCKS] Trying to use SOCKS5 proxy\n");
    prsin.sin_addr.s_addr = inet_addr(icq_ProxyHost);
    if(prsin.sin_addr.s_addr  == -1) /* name isn't n.n.n.n so must be DNS */
    {
      host_struct = gethostbyname(icq_ProxyHost);
      if(host_struct == 0L)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
        {
          sprintf(tmpbuf, "[SOCKS] Can't find hostname: %s\n", icq_ProxyHost);
          (*icq_Log)(time(0L), ICQ_LOG_FATAL, tmpbuf);
        }
        return -1;
      }
      prsin.sin_addr = *((struct in_addr *)host_struct->h_addr);
    }
    prsin.sin_family = AF_INET; /* we're using the inet not appletalk*/
    prsin.sin_port = htons(icq_ProxyPort); /* port */
    icq_ProxySok = socket(AF_INET, SOCK_STREAM, 0);/* create the unconnected socket*/
    if(icq_ProxySok == -1)
    {
      if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
        (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Socket creation failed\n");
      return -1;
    }
    if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "[SOCKS] Socket created attempting to connect\n");
    conct = connect(icq_ProxySok, (struct sockaddr *) &prsin, sizeof(prsin));
    if(conct == -1) /* did we connect ?*/
    {
      if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
        (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Connection refused\n");
      return -1;
    }
    buf[0] = 5; /* protocol version */
    buf[1] = 1; /* number of methods */
    if(!strlen(icq_ProxyName) || !strlen(icq_ProxyPass) || !icq_ProxyAuth)
      buf[2] = 0; /* no authorization required */
    else
      buf[2] = 2; /* method username/password */
    write(icq_ProxySok, buf, 3);
    res = read(icq_ProxySok, buf, 2);
    if(res != 2 || buf[0] != 5 || buf[1] != 2)
    {
      if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
        (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Authentication method incorrect\n");
      close(icq_ProxySok);
      return -1;
    }
    if(strlen(icq_ProxyName) && strlen(icq_ProxyPass) && icq_ProxyAuth)
    {
      buf[0] = 1; /* version of subnegotiation */
      buf[1] = strlen(icq_ProxyName);
      memcpy(&buf[2], icq_ProxyName, buf[1]);
      buf[2+buf[1]] = strlen(icq_ProxyPass);
      memcpy(&buf[3+buf[1]], icq_ProxyPass, buf[2+buf[1]]);
      write(icq_ProxySok, buf, buf[1]+buf[2+buf[1]]+3);
      res = read(icq_ProxySok, buf, 2);
      if(res != 2 || buf[0] != 1 || buf[1] != 0)
      {
        if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
          (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Authorization failure\n");
        close(icq_ProxySok);
        return -1;
      }
    }
    buf[0] = 5; /* protocol version */
    buf[1] = 3; /* command UDP associate */
    buf[2] = 0; /* reserved */
    buf[3] = 1; /* address type IP v4 */
    buf[4] = (char)0;
    buf[5] = (char)0;
    buf[6] = (char)0;
    buf[7] = (char)0;
    memcpy(&buf[8], &icq_ProxyOurPort, 2);
    write(icq_ProxySok, buf, 10);
    res = read(icq_ProxySok, buf, 10);
    if(res != 10 || buf[0] != 5 || buf[1] != 0)
    {
      switch(buf[1])
      {
        case 1:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] General SOCKS server failure\n");
          break;
        case 2:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Connection not allowed by ruleset\n");
          break;
        case 3:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Network unreachable\n");
          break;
        case 4:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Host unreachable\n");
          break;
        case 5:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Connection refused\n");
          break;
        case 6:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] TTL expired\n");
          break;
        case 7:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Command not supported\n");
          break;
        case 8:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Address type not supported\n");
          break;
        default:
          if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
            (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Unknown SOCKS server failure\n");
          break;
      }
      close(icq_ProxySok);
      return -1;
    }
  }
  sin.sin_addr.s_addr = inet_addr(hostname); /* checks for n.n.n.n notation */
  if(sin.sin_addr.s_addr == -1) /* name isn't n.n.n.n so must be DNS */
  {
    host_struct = gethostbyname(hostname);
    if(host_struct == 0L)
    {
      if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      {
        sprintf(tmpbuf, "Can't find hostname: %s\n", hostname);
        (*icq_Log)(time(0L), ICQ_LOG_FATAL, tmpbuf);
      }
      if(icq_UseProxy)
        close(icq_ProxySok);
      return -1;
    }
    sin.sin_addr = *((struct in_addr *)host_struct->h_addr);
  }
  if(icq_UseProxy)
  {
    icq_ProxyDestHost = sin.sin_addr.s_addr;
    memcpy(&sin.sin_addr.s_addr, &buf[4], 4);
  }
  sin.sin_family = AF_INET; /* we're using the inet not appletalk*/
  sin.sin_port = htons(port); /* port */
  if(icq_UseProxy)
  {
    icq_ProxyDestPort = htons(port);
    memcpy(&sin.sin_port, &buf[8], 2);
  }
  conct = connect(icq_Sok, (struct sockaddr*)&sin, sizeof(sin));
  if(conct == -1) /* did we connect ?*/
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      (*icq_Log)(time(0L), ICQ_LOG_FATAL, "Connection refused\n");
    if(icq_UseProxy)
      close(icq_ProxySok);
    return -1;
  }
  length = sizeof(sin) ;
  getsockname(icq_Sok, (struct sockaddr *) &sin, &length);
  icq_OurIp = sin.sin_addr.s_addr;
  icq_OurPort = sin.sin_port;
  return icq_Sok;
}

void icq_HandleProxyResponse()
{
  int s;
  char buf[256];
  s = read(icq_ProxySok, &buf, sizeof(buf));
  if(s<=0)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      (*icq_Log)(time(0L), ICQ_LOG_FATAL, "[SOCKS] Connection terminated\n");
    if(icq_Disconnected)
      (*icq_Disconnected)();
    SOCKCLOSE(icq_Sok);
    SOCKCLOSE(icq_ProxySok);
  }
}

/******************************************
Handles packets that the server sends to us.
*******************************************/
void icq_HandleServerResponse()
{
  srv_net_icq_pak pak;
  SIMPLE_MESSAGE *s_mesg;
  RECV_MESSAGE *r_mesg;
  time_t cur_time;
  struct tm *tm_str;
  int s,len;
  char buf[1024];
  /* cback acback; */

  s = icq_SockRead(icq_Sok, &pak.head, sizeof(pak));
  if(s<=0)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_FATAL)
      (*icq_Log)(time(0L), ICQ_LOG_FATAL, "Connection terminated\n");
    if(icq_Disconnected)
      (*icq_Disconnected)();
    SOCKCLOSE(icq_Sok);
  }
  if((icq_GetServMess(Chars_2_Word(pak.head.seq))) && (Chars_2_Word(pak.head.cmd) != SRV_NEW_UIN) && (Chars_2_Word(pak.head.cmd) != SRV_GO_AWAY))
  {
    if(Chars_2_Word(pak.head.cmd) != SRV_ACK) /* ACKs don't matter */
    {
      if(icq_Log && icq_LogLevel>=ICQ_LOG_WARNING)
      {
        sprintf(buf, "Ignored a message cmd %04x, seq %04x\n", Chars_2_Word(pak.head.cmd), Chars_2_Word(pak.head.seq));
        (*icq_Log)(time(0L), ICQ_LOG_WARNING, buf);
      }
      icq_AckSrv(Chars_2_Word(pak.head.seq)); /* LAGGGGG!! */
      return;
    }
  }
  if(Chars_2_Word(pak.head.cmd) != SRV_ACK)
    icq_SetServMess(Chars_2_Word(pak.head.seq));
  switch(Chars_2_Word(pak.head.cmd))
  {
    case SRV_ACK:
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "The server acknowledged the command\n");
      if(icq_SrvAck)
        (*icq_SrvAck)(Chars_2_Word(pak.head.seq));
      break;
    case SRV_NEW_UIN:
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "The new uin is %lu\n", Chars_2_DW(&pak.data[2]));
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
        printf("%s\n", buf);
      }
      break;
    case SRV_LOGIN_REPLY:
      icq_OurIp = Chars_2_DW(&pak.data[4]);
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
      {
        sprintf(buf, "Login successful, UIN: %lu, IP: %u.%u.%u.%u\n", Chars_2_DW(pak.data), pak.data[4], pak.data[5], pak.data[6], pak.data[7]);
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, buf);
        printf("%s\n", buf);
      }
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      icq_SendLogin1();
      icq_SendContactList();
      icq_SendVisibleList();
      if(icq_Logged)
        (*icq_Logged)();
      break;
    case SRV_RECV_MESSAGE:
      r_mesg = (RECV_MESSAGE*)pak.data;
      icq_DoMsg(Chars_2_Word(r_mesg->type), Chars_2_Word(r_mesg->len), (char*)(r_mesg->len + 2), Chars_2_DW(r_mesg->uin), r_mesg->hour, r_mesg->minute, r_mesg->day, r_mesg->month, Chars_2_Word(r_mesg->year));
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      break;
    case SRV_X1: /* unknown message  sent after login*/
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Acknowleged SRV_X1 (Begin messages)\n");
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      break;
    case SRV_X2: /* unknown message  sent after login*/
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Acknowleged SRV_X2 (Done old messages)\n");
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      icq_SendGotMessages();
      break;
    case SRV_INFO_REPLY:
      icq_HandleInfoReply(pak);
      break;
    case SRV_EXT_INFO_REPLY:
      icq_HandleExtInfoReply(pak);
      break;
    case SRV_USER_ONLINE:
      icq_HandleUserOnline(pak);
      break;
    case SRV_USER_OFFLINE:
      icq_HandleUserOffline(pak);
      break;
    case SRV_TRY_AGAIN:
      if(icq_Log && icq_LogLevel>=ICQ_LOG_WARNING)
        (*icq_Log)(time(0L), ICQ_LOG_WARNING, "Server is busy, please try again\n");
      icq_Login(icq_Status);
      break;
    case SRV_STATUS_UPDATE:
      icq_StatusUpdate(pak);
      break;
    case SRV_GO_AWAY:
      if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
        (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Server has forced us to disconnect\n");
      if(icq_Disconnected)
        (*icq_Disconnected)();
      break;
    case SRV_END_OF_SEARCH:
      if(icq_Log && icq_LogLevel>=ICQ_LOG_MESSAGE)
        (*icq_Log)(time(0L), ICQ_LOG_MESSAGE, "Search done\n");
      if(icq_SearchDone)
        (*icq_SearchDone)();
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      break;
    case SRV_USER_FOUND:
      icq_HandleSearchReply(pak);
      break;
    case SRV_SYS_DELIVERED_MESS:
      s_mesg = (SIMPLE_MESSAGE*)pak.data;
      cur_time = time(0L);
      tm_str = localtime(&cur_time);
      icq_DoMsg(Chars_2_Word(s_mesg->type), Chars_2_Word(s_mesg->len), (char*)(s_mesg->len + 2), Chars_2_DW(s_mesg->uin), tm_str->tm_hour, tm_str->tm_min, tm_str->tm_mday, tm_str->tm_mon+1, tm_str->tm_year+1900);
      icq_AckSrv(Chars_2_Word(pak.head.seq));
      break;
    default: /* commands we dont handle yet */
      len = s - (sizeof(pak.head));
      if(icq_Log && icq_LogLevel>=ICQ_LOG_WARNING)
      {
        sprintf(buf, "Unhandled message %04x, Version: %x, Sequence: %04x, Size: %d\n",
                Chars_2_Word(pak.head.cmd), Chars_2_Word(pak.head.ver), Chars_2_Word(pak.head.seq), len);
        (*icq_Log)(time(0L), ICQ_LOG_WARNING, buf);
        printf("%s\n", buf);
      }
      icq_AckSrv(Chars_2_Word(pak.head.seq)); /* fake like we know what we're doing*/
      break;
  }
}

void icq_Init(DWORD uin, const char *password)
{
  memset(icq_ServMess, FALSE, sizeof(icq_ServMess));
  icq_Uin = uin;
  if(icq_Password)
    free(icq_Password);
  icq_Password = strdup(password);
}

void icq_Done(void)
{
  if(icq_Password)
    free(icq_Password);
}

/******************************
Main function connects gets icq_Uin
and icq_Password and logins in and sits
in a loop waiting for server responses.
*******************************/
void icq_Main()
{
  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(icq_Sok, &readfds);
  select(icq_Sok+1, &readfds, 0L, 0L, &tv);
  if(FD_ISSET(icq_Sok, &readfds))
    icq_HandleServerResponse();
}

/********************************************************
Russian language ICQ fix.
Usual Windows ICQ users do use Windows 1251 encoding but
unix users do use koi8 encoding, so we need to convert it.
This function will convert string from windows 1251 to koi8
or from koi8 to windows 1251.
Andrew Frolov dron@ilm.net
*********************************************************/
void icq_RusConv(const char to[4], char *t_in)
{
  BYTE *table;
  int i;

/* 6-17-1998 by Linux_Dude
 * Moved initialization of table out front of 'if' block to prevent compiler
 * warning. Improved error message, and now return without performing string
 * conversion to prevent addressing memory out of range (table pointer would
 * previously have remained uninitialized (= bad)).
 */

  table = wk;
  if(strcmp(to, "kw") == 0)
    table = kw;
  else if(strcmp(to, "wk") != 0)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
      (*icq_Log)(time(0L), ICQ_LOG_ERROR, "Unknown option in call to Russian Convert\n");
    return;
  }
      
/* End Linux_Dude's changes ;) */

  if(icq_Russian)
  {
    for(i=0;t_in[i]!=0;i++)
    {
      t_in[i] &= 0377;
      if(t_in[i] & 0200)
        t_in[i] = table[t_in[i] & 0177];
    }
  }
}

/**************************************************
Sends a message thru the server to icq_Uin.  Text is the
message to send.
***************************************************/
WORD icq_SendMessage(DWORD uin, const char *text)
{
  SIMPLE_MESSAGE msg;
  net_icq_pak pak;
  int size, len, i;
  char buf[512]; /* message may be only 450 bytes long */

  strncpy(buf, text, 512);
  icq_RusConv("kw",buf);
  len = strlen(buf);
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SENDM);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  DW_2_Chars(msg.uin, uin);
  DW_2_Chars(msg.type, 0x0001);   /* A type 1 msg*/
  Word_2_Chars(msg.len, len + 1);   /* length + the NULL */
  memcpy(&pak.data, &msg, sizeof(msg));
  memcpy(&pak.data[8], buf, len + 1);
  size = sizeof(msg) + len + 1;
  for(i=0; i<6; i++)
    icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
  return icq_SeqNum-1;
}

WORD icq_SendURL(DWORD uin, const char *url, const char *descr)
{
  SIMPLE_MESSAGE msg;
  net_icq_pak pak;
  int size, len1, len2;
  char buf1[512], buf2[512];

  strncpy(buf1, descr, 512);
  strncpy(buf2, url, 512);
/* Do we need to convert URL? */
  icq_RusConv("kw",buf2);
  len1 = strlen(buf1);
  len2 = strlen(buf2);
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SENDM);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  DW_2_Chars(msg.uin, uin);
  DW_2_Chars(msg.type, 0x0004);   /* A type 4 msg*/
  Word_2_Chars(msg.len, len1+len2+2);   /* length + the NULL + 0xFE delimiter */
  memcpy(&pak.data, &msg, sizeof(msg));
  memcpy(&pak.data[8], buf1, len1);
  pak.data[8+len1] = 0xFE;
  memcpy(&pak.data[8+len1+1], buf2, len2+1);
  size = sizeof(msg)+len1+len2+2;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
  return icq_SeqNum-1;
}

/**************************************************
Sends a authorization to the server so the Mirabilis
client can add the user.
***************************************************/
void icq_SendAuthMsg(DWORD uin)
{
  SIMPLE_MESSAGE msg;
  net_icq_pak pak;
  int size; 

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SENDM);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  DW_2_Chars(msg.uin, uin);
  DW_2_Chars(msg.type, AUTH_MESSAGE);   /* A type authorization msg*/
  Word_2_Chars(msg.len, 2);   
  memcpy(&pak.data, &msg, sizeof(msg));
  pak.data[sizeof(msg)] = 0x03;
  pak.data[sizeof(msg) + 1] = 0x00;
  size = sizeof(msg) + 2;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

/**************************************************
Changes the users status on the server
***************************************************/
void icq_ChangeStatus(DWORD status)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_STATUS_CHANGE);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  DW_2_Chars(pak.data, status);
  icq_Status = status;
  size = 4;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

/**********************
Logs off ICQ
***********************/
void icq_Logout()
{
  net_icq_pak pak;
  int size, len;
   
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SEND_TEXT_CODE);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  len = strlen("B_USER_DISCONNECTED") + 1;
  *(short*)pak.data = len;
  size = len + 4;
  memcpy(&pak.data[2], "B_USER_DISCONNECTED", len);
  pak.data[2 + len] = 05;
  pak.data[3 + len] = 00;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

void icq_Disconnect()
{
  SOCKCLOSE(icq_Sok);
  SOCKCLOSE(icq_Sok);
  if(icq_UseProxy)
    SOCKCLOSE(icq_ProxySok);
}

/********************************************************
Sends a request to the server for info on a specific user
*********************************************************/
WORD icq_SendInfoReq(DWORD uin)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_INFO_REQ);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  Word_2_Chars(pak.data, icq_SeqNum++);
  DW_2_Chars(&pak.data[2], uin);
  size = 6;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
  return icq_SeqNum-1;
}

/********************************************************
Sends a request to the server for info on a specific user
*********************************************************/
WORD icq_SendExtInfoReq(DWORD uin)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_EXT_INFO_REQ);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  Word_2_Chars(pak.data, icq_SeqNum++);
  DW_2_Chars(&pak.data[2], uin);
  size = 6;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
  return icq_SeqNum-1;
}

/**************************************************************
Initializes a server search for the information specified
***************************************************************/
void icq_SendSearchReq(const char *email, const char *nick, const char* first, const char* last)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SEARCH_USER);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  Word_2_Chars(pak.data, icq_SeqNum++);
  size = 2;
  Word_2_Chars(&pak.data[size], strlen(nick) + 1);
  size += 2;
  strcpy(pak.data + size, nick);
  size += strlen(nick) + 1;
  Word_2_Chars(&pak.data[size], strlen(first) + 1);
  size += 2;
  strcpy(pak.data + size, first);
  size += strlen(first) + 1;
  Word_2_Chars(&pak.data[size], strlen(last) + 1);
  size += 2;
  strcpy(pak.data + size, last);
  size += strlen(last) +1 ;
  Word_2_Chars(&pak.data[size], strlen(email) + 1);
  size += 2;
  strcpy(pak.data + size , email);
  size += strlen(email) + 1;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

/**************************************************************
Initializes a server search for the information specified
***************************************************************/
void icq_SendSearchUINReq(DWORD uin)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_SEARCH_UIN);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  Word_2_Chars(pak.data, icq_SeqNum++);
  size = 2;
  DW_2_Chars(&pak.data[size], uin);
  size += 4;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

/**************************************************
Registers a new uin in the ICQ network
***************************************************/
void icq_RegNewUser(const char *pass)
{
  srv_net_icq_pak pak;
  BYTE len_buf[2];
  int size, len; 

  len = strlen(pass);
  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_REG_NEW_USER);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  Word_2_Chars(len_buf, len);
  memcpy(&pak.data, "\x02\x00", 2);
  memcpy(&pak.data[2], &len_buf, 2);
  memcpy(&pak.data[4], pass, len + 1);
  DW_2_Chars(&pak.data[4+len], 0x0072);
  DW_2_Chars(&pak.data[8+len], 0x0000);
  size = len + 12;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

void icq_UpdateUserInfo(USER_INFO *user)
{
  net_icq_pak pak;
  int size ;

  Word_2_Chars(pak.head.ver, ICQ_VER);
  Word_2_Chars(pak.head.cmd, CMD_UPDATE_INFO);
  Word_2_Chars(pak.head.seq, icq_SeqNum++);
  DW_2_Chars(pak.head.uin, icq_Uin);
  Word_2_Chars(pak.data, icq_SeqNum++);
  size = 2;
  Word_2_Chars(&pak.data[size], strlen(user->nick) + 1);
  size += 2;
  strcpy(pak.data + size , user->nick);
  size += strlen(user->nick) + 1;
  Word_2_Chars(&pak.data[size], strlen(user->first) + 1);
  size += 2;
  strcpy(pak.data + size , user->first);
  size += strlen(user->first) + 1;
  Word_2_Chars(&pak.data[size], strlen(user->last) + 1);
  size += 2;
  strcpy(pak.data + size , user->last);
  size += strlen(user->last) +1 ;
  Word_2_Chars(&pak.data[size], strlen(user->email) + 1);
  size += 2;
  strcpy(pak.data + size , user->email);
  size += strlen(user->email) + 1;
  pak.data[size] = user->auth;
  size++;
  icq_SockWrite(icq_Sok, &pak.head, size + sizeof(pak.head));
}

const char *icq_GetCountryName(int code)
{
  int i;
   
  for(i = 0; Country_Codes[i].code != 0xffff; i++)
  {
    if(Country_Codes[i].code == code)
    {
      return Country_Codes[i].name;
    }
  }
  if(Country_Codes[i].code == code)
  {
    return Country_Codes[i].name;
  }
  return "N/A";
}

/********************************************
returns a string describing the status or
a "Error" if no such string exists
*********************************************/
const char *icq_ConvertStatus2Str(int status)
{
  if(STATUS_OFFLINE == status) /* this because -1 & 0x01FF is not -1 */
  {
    return "Offline";
  }

/* To handle icq99a statuses 0xFFFF changed to 0x01FF */
  switch(status & 0x01FF)
  {
    case STATUS_ONLINE:
      return "Online";
      break;
    case STATUS_DND:
      return "Do not disturb";
      break;
    case STATUS_AWAY:
      return "Away";
      break;
    case STATUS_OCCUPIED:
      return "Occupied";
      break;
    case STATUS_NA:
      return "Not available";
      break;
    case STATUS_INVISIBLE:
      return "Invisible";
      break;
    case STATUS_INVISIBLE_2:
      return "Invisible mode 2";
      break;
    case STATUS_FREE_CHAT:
      return "Free for chat";
      break;
    default:
      return "Error";
      break;
  }
}

void icq_InitNewUser(const char *hostname, DWORD port)
{
  srv_net_icq_pak pak;
  int s;
  struct timeval tv;
  fd_set readfds;

  icq_Connect(hostname, port);
  if((icq_Sok == -1) || (icq_Sok == 0))
  {
    printf("Couldn't establish connection\n");
    exit(1);
  }
  icq_RegNewUser(icq_Password);
  for(;;)
  {
    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    FD_ZERO(&readfds);
    FD_SET(icq_Sok, &readfds);

    /* don't care about writefds and exceptfds: */
    select(icq_Sok+1, &readfds, 0L, 0L, &tv);

    if(FD_ISSET(icq_Sok, &readfds))
    {
      s = icq_SockRead(icq_Sok, &pak.head, sizeof(pak));
      if(Chars_2_Word(pak.head.cmd) == SRV_NEW_UIN)
      {
        icq_Uin = Chars_2_DW(&pak.data[2]);
        return;
      }
    }
  }
}

/********************************************
Converts an intel endian character sequence to
a DWORD
*********************************************/
DWORD Chars_2_DW(unsigned char *buf)
{
  DWORD i;
   
  i= buf[3];
  i <<= 8;
  i+= buf[2];
  i <<= 8;
  i+= buf[1];
  i <<= 8;
  i+= buf[0];
   
  return i;
}

/********************************************
Converts an intel endian character sequence to
a WORD
*********************************************/
WORD Chars_2_Word(unsigned char *buf)
{
  WORD i;
   
  i= buf[1];
  i <<= 8;
  i += buf[0];
   
  return i;
}

/********************************************
Converts a DWORD to
an intel endian character sequence 
*********************************************/
void DW_2_Chars(unsigned char *buf, DWORD num)
{
  buf[3] = (unsigned char) ((num)>>24)& 0x000000FF;
  buf[2] = (unsigned char) ((num)>>16)& 0x000000FF;
  buf[1] = (unsigned char) ((num)>>8)& 0x000000FF;
  buf[0] = (unsigned char) (num) & 0x000000FF;
}

/********************************************
Converts a WORD to
an intel endian character sequence 
*********************************************/
void Word_2_Chars(unsigned char *buf, WORD num)
{
  buf[1] = (unsigned char) (((unsigned)num)>>8) & 0x00FF;
  buf[0] = (unsigned char) ((unsigned)num) & 0x00FF;
}

/***************************
ContactList functions
***************************/
void icq_ContAddUser(DWORD cuin)
{
  icq_ContactItem *p = malloc(sizeof(icq_ContactItem));
  icq_ContactItem *ptr = icq_ContFirst;
  p->uin = cuin;
  p->next = 0L;
  p->vis_list = FALSE;
  if(ptr)
  {
    while(ptr->next)
      ptr = ptr->next;
    ptr->next = p;
  }
  else
    icq_ContFirst = p;
}

void icq_ContDelUser(DWORD cuin)
{
  icq_ContactItem *ptr = icq_ContFirst;
  if(!ptr)
    return;
  if(ptr->uin == cuin)
  {
    icq_ContFirst = ptr->next;
    free(ptr);
    ptr = icq_ContFirst;
  }
  while(ptr->next)
  {
    if(ptr->next->uin == cuin)
    {
      ptr->next = ptr->next->next;
      free(ptr->next);
    }
    ptr = ptr->next;
  }
}

void icq_ContClear()
{
  icq_ContactItem *tmp, *ptr = icq_ContFirst;
  while(ptr)
  {
    tmp = ptr->next;
    free(ptr);
    ptr = tmp;
    icq_ContFirst = ptr;
  }
}

icq_ContactItem *icq_ContFindByUin(DWORD cuin)
{
  icq_ContactItem *ptr = icq_ContFirst;
  if(!ptr)
    return 0L;
  while(ptr)
  {
    if(ptr->uin == cuin)
      return ptr;
    ptr = ptr->next;
  }
  return 0L;
}

icq_ContactItem *icq_ContGetFirst()
{
  return icq_ContFirst;
}

void icq_ContSetVis(DWORD cuin)
{
  icq_ContactItem *ptr = icq_ContFirst;
  if(!ptr)
    return;
  while(ptr)
  {
    if(ptr->uin == cuin)
      ptr->vis_list = TRUE;
    ptr = ptr->next;
  }
}

/************************
icq_ServMess functions
*************************/
BOOL icq_GetServMess(WORD num)
{
  return ((icq_ServMess[num/8] & (1 << (num%8))) >> (num%8));
}

void icq_SetServMess(WORD num)
{
  icq_ServMess[num/8] |= (1 << (num%8));
}

int icq_GetSok()
{
  return icq_Sok;
}

int icq_GetProxySok()
{
  return icq_ProxySok;
}

/*******************
SOCKS5 Proxy support
********************/
void icq_SetProxy(const char *phost, unsigned short pport, int pauth, const char *pname, const char *ppass)
{
  if(icq_ProxyHost)
    free(icq_ProxyHost);
  if(icq_ProxyName)
    free(icq_ProxyName);
  if(icq_ProxyPass)
    free(icq_ProxyPass);
  if(strlen(pname)>255)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
      (*icq_Log)(time(0L), ICQ_LOG_ERROR, "[SOCKS] User name greater than 255 chars\n");
    icq_UseProxy = 0;
    return;
  }
  if(strlen(ppass)>255)
  {
    if(icq_Log && icq_LogLevel>=ICQ_LOG_ERROR)
      (*icq_Log)(time(0L), ICQ_LOG_ERROR, "[SOCKS] User password greater than 255 chars\n");
    icq_UseProxy = 0;
    return;
  }
  icq_UseProxy = 1;
  icq_ProxyHost = strdup(phost);
  icq_ProxyPort = pport;
  icq_ProxyAuth = pauth;
  icq_ProxyName = strdup(pname);
  icq_ProxyPass = strdup(ppass);
}

void icq_UnsetProxy()
{
  icq_UseProxy = 0;
}
