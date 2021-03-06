#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <unistd.h>
#include    <poll.h>
#include    <errno.h>
#include    <signal.h>
#include    <stdarg.h>
#include    <sys/socket.h>
#include    <arpa/inet.h>
#include    <linux/if_packet.h>
#include    <netinet/if_ether.h>
#include    <sys/select.h>
#include    <sys/time.h>
#include    "netutil.h"

#define maxfds(a,b) ((a) > (b) ? (a+1) : (b+1))

PARAM   Param = {{"p8p1","wlp2s0","wlp0s29u1u3",NULL}};
DEVICE  Device[MAXIFNUM+1];

int EndFlag  = 0;
int DebugOut = 1;

int DebugPrintf(char *fmt,...)
{
    if(DebugOut) {
        va_list args;

        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);
    }

    return(0);
}

int DebugPerror(char *msg)
{
    if(DebugOut) {
        fprintf(stderr,"%s : %s\n",msg,strerror(errno));
    }

    return(0);
}
int GetMacAddr(int devicenum, struct ether_header *eh)
{
    char    buf[18];

    memcpy(Device[devicenum].shost, my_ether_ntoa_r(eh->ether_shost,buf,sizeof(buf)), sizeof(buf));

   return 0;
}

int SearchSrcMacAddr(struct ether_header *eh)
{
    char    buf[MAC_LEN];
    int i;

    for(i=0 ; i < MAXIFNUM; i++){
        if(strcmp(Device[i].shost, my_ether_ntoa_r(eh->ether_shost,buf,sizeof(buf))) == 0){
           return -1;
        }
    }
    return 0;
}
int SearchDstMacAddr(struct ether_header *eh)
{
    char    buf[MAC_LEN];
    int     i;

    for(i=0 ; i < MAXIFNUM; i++){
        if(strcmp(Device[i].shost, my_ether_ntoa_r(eh->ether_dhost,buf,sizeof(buf))) == 0){
           return i;
        }
    }
    return -1;
}

int AnalyzePacket(int devicenum, DEVICE *device, u_char *data, int size)
{
    u_char  *ptr;
    int lest;
    struct ether_header *eh;

    ptr=data;
    lest=size;

    if(lest < sizeof(struct ether_header)) {
        DebugPrintf("[sockfd:%d]:lest(%d)<sizeof(struct ether_header)\n",device[devicenum].soc,lest);
        return(-1);
    }
    eh = (struct ether_header *)ptr;
    ptr += sizeof(struct ether_header);
    lest -= sizeof(struct ether_header);
    DebugPrintf("[sockfd:%d]",device[devicenum].soc);

    if(DebugOut) {
        PrintEtherHeader(eh,stderr);
    }

    if(SearchSrcMacAddr(eh) == 0){
        GetMacAddr(devicenum , eh);
    }

    return(SearchDstMacAddr(eh));
}

int SwHub()
{
    int i,size, dstnum;
    u_char  buf[2048];
    int nfds = 0;
    struct timeval t;
    fd_set fds, readfds;

    t.tv_sec = 1;
    t.tv_usec = 0;

    FD_ZERO(&readfds);
    for(i=0; i<MAXIFNUM; i++){
        if(Device[i].soc != 0){
            FD_SET(Device[i].soc, &readfds);
            if(nfds < Device[i].soc){
                nfds = Device[i].soc;
            }
        }
    }
    while(EndFlag==0) {
        memcpy(&fds, &readfds, sizeof(fd_set));
        if(select(nfds+1, &fds, NULL, NULL, &t) < 0){
            perror("select");
            exit(0);
        }
        for(i=0; i < MAXIFNUM; i++) {
            if(FD_ISSET(Device[i].soc, &fds)){
                memset(buf, 0, sizeof(buf));
                if((size = recv(Device[i].soc, buf, sizeof(buf),0))<=0){
                    perror("read");
                }
                else {
                    if((dstnum = AnalyzePacket(i, Device,buf,size)) <= 0) {
                        if((size=write(Device[(!i)].soc,buf,size)) <= 0) {
                            perror("write");
                        }
                    }else{
                        if((size=write(Device[dstnum].soc,buf,size)) <= 0) {
                            perror("write");
                        }
                    }
                }
            }
        }
    }

    return(0);
}

int DisableIpForward()
{
    FILE    *fp;

    if((fp=fopen("/proc/sys/net/ipv4/ip_forward","w"))==NULL) {
        DebugPrintf("cannot write /proc/sys/net/ipv4/ip_forward\n");
        return(-1);
    }
    fputs("0",fp);
    fclose(fp);

    return(0);
}

void EndSignal(int sig)
{
    EndFlag=1;
}

int main(int argc,char *argv[],char *envp[])
{

    /* Deviceのリンクレイヤで受信できるsfdを生成する */
    if(CreateRawSocket(&Param, Device) != 0){
        goto ERR;
    }
 
#if 0
    if((Device[0].soc=InitRawSocket(Param.Device1,PROMISCAS_MODE,ALL_OR_IP))==-1) {
        DebugPrintf("InitRawSocket:error:%s\n",Param.Device1);
        return(-1);
    }
    DebugPrintf("%s OK\n",Param.Device1);

    if((Device[1].soc=InitRawSocket(Param.Device2,PROMISCAS_MODE,ALL_OR_IP))==-1) {
        DebugPrintf("InitRawSocket:error:%s\n",Param.Device1);
        return(-1);
    }
    DebugPrintf("%s OK\n",Param.Device2);
#endif
    /* カーネルによるIPフォワーディングを停止する*/
    DisableIpForward();

    /* シグナルハンドラにEndSignalを登録する */
    signal(SIGINT,EndSignal);
    signal(SIGTERM,EndSignal);
    signal(SIGQUIT,EndSignal);

    /* 他tty終了時にプログラムが停止するのを防ぐためSIGPIPEなどを無視する。 */
    signal(SIGPIPE,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);

    DebugPrintf("swhub start\n");
    /* SwitchingHub処理を開始 */
    SwHub();
    DebugPrintf("swhub end\n");

ERR:
    CloseRawSocket(Device);
#if 0
    close(Device[0].soc);
    close(Device[1].soc);
#endif

    return(0);
}
