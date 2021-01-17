#ifndef __web_Utils_h__
#define __web_Utils_h__

void appendFile(const char * path, const char * message);
void SSE_add_char(const char *c);
void set_SSE_broadcast_flag(bool flag);
void SSEBroadcastTxt(String txt);
void web_setup(void);
void web_loop(void);

#endif // def(__web_Utils_h__)