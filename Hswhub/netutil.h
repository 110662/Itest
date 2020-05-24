#include    <sys/socket.h>
#include    <netinet/if_ether.h>

#define PROMISCAS_MODE  0
#define ALL_OR_IP       1   /* ALL:0, IP:1 */
#define IFNAME_LEN      10
#define MAXIFNUM  3
#define MAC_LEN 18

typedef struct  {
    char    *Device[MAXIFNUM+1];
} PARAM;

/* DEVICE構造体 : NICのソケットディスクリプタを保持する構造体*/
typedef struct{
    int soc;
    char *ifname;
    char shost[MAC_LEN];
    char dhost[MAC_LEN];
} DEVICE;

extern char *my_ether_ntoa_r(u_char *hwaddr,char *buf,socklen_t size);
extern int PrintEtherHeader(struct ether_header *eh,FILE *fp);
extern int InitRawSocket(char *device,int promiscFlag,int ipOnly);
extern int CreateRawSocket(PARAM *iflist, DEVICE *socklist);
extern void CloseRawSocket(DEVICE *socklist);


