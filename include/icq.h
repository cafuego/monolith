#ifndef _ICQ_H_
#define _ICQ_H_

#define ICQLIBVER 013

#include <time.h>

#define ICQ_LOG_OFF     0
#define ICQ_LOG_FATAL		1
#define ICQ_LOG_ERROR		2
#define ICQ_LOG_WARNING	3
#define ICQ_LOG_MESSAGE	4

#define STATUS_OFFLINE		  (-1L)
#define STATUS_ONLINE		    0x00
#define STATUS_INVISIBLE	  0x100
#define STATUS_INVISIBLE_2	0x10
#define STATUS_NA           0x05
#define STATUS_FREE_CHAT	  0x20
#define STATUS_OCCUPIED		  0x11
#define STATUS_AWAY         0x01
#define STATUS_DND          0x13

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern unsigned long icq_Status;
extern unsigned char icq_Russian;
extern unsigned char icq_LogLevel;

extern void (*icq_Logged)(void);
extern void (*icq_Disconnected)(void);
extern void (*icq_RecvMessage)(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *msg);
extern void (*icq_RecvURL)(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *url, const char *descr);
extern void (*icq_RecvAdded)(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email);
extern void (*icq_RecvAuthReq)(unsigned long uin, unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, unsigned short year, const char *nick, const char *first, const char *last, const char *email, const char *reason);
extern void (*icq_UserFound)(unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth);
extern void (*icq_SearchDone)(void);
extern void (*icq_UserOnline)(unsigned long uin, unsigned long status, unsigned long ip, unsigned long port, unsigned long real_ip);
extern void (*icq_UserOffline)(unsigned long uin);
extern void (*icq_UserStatusUpdate)(unsigned long uin, unsigned long status);
extern void (*icq_InfoReply)(unsigned long uin, const char *nick, const char *first, const char *last, const char *email, char auth);
extern void (*icq_ExtInfoReply)(unsigned long uin, const char *city, unsigned short country_code, char country_stat, const char *state, unsigned short age, char gender, const char *phone, const char *hp, const char *about);
extern void (*icq_Log)(time_t time, unsigned char level, const char *str);
extern void (*icq_SrvAck)(unsigned short seq);

void icq_SetProxy(const char *phost, unsigned short pport, int pauth, const char *pname, const char *ppass);
void icq_UnsetProxy();
void icq_Init(unsigned long uin, const char *password);
int icq_Connect(const char *hostname, int port);
void icq_Disconnect();
int icq_GetSok();
int icq_GetProxySok();
void icq_HandleServerResponse();  /* should be called when data received */
void icq_HandleProxyResponse(); /* should be called when data arrived on proxy socket */
void icq_Main();
void icq_KeepAlive();
void icq_Login(unsigned long status);
void icq_Logout();
void icq_SendContactList();
void icq_SendVisibleList();
void icq_SendNewUser(unsigned long uin);
unsigned short icq_SendMessage(unsigned long uin, const char *text);
unsigned short icq_SendURL(unsigned long uin, const char *url, const char *descr);
void icq_ChangeStatus(unsigned long status);
unsigned short icq_SendInfoReq(unsigned long uin);
unsigned short icq_SendExtInfoReq(unsigned long uin);
void icq_SendAuthMsg(unsigned long uin);
void icq_SendSearchReq(const char *email, const char *nick, const char* first, const char* last);
void icq_SendSearchUINReq(unsigned long uin);

const char *icq_GetCountryName(int code);
void icq_RegNewUser(const char *pass);

void icq_ContAddUser(unsigned long cuin);
void icq_ContDelUser(unsigned long cuin);
void icq_ContClear();
void icq_ContSetVis(unsigned long cuin);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ICQ_H_ */
