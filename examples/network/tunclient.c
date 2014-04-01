#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <time.h>

#include <uthash.h>   

#include "net.nail.h"
#include <hammer/hammer.h>

extern HAllocator system_allocator;
//http://backreference.org/2010/03/26/tuntap-interface-tutorial/
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}
typedef struct arpentry{
        uint32_t ip;
        macaddr mac; // All 0 when unused 
        time_t time;
        UT_hash_handle hh;
} arpentry;
#define ARPSIZE 512
struct ip_iface{
        uint32_t ip;
        uint32_t broadcast;
};
typedef struct phy_iface{
        int tun_fd;
        macaddr mac;

        arpentry *arp; 
        struct ip_iface ip; // TODO: Add multi-IP support?
} phy_iface;
void phy_init(phy_iface *phy, macaddr *mac, uint32_t ip){
        memcpy(phy->mac,mac,sizeof(phy->mac));
        phy->arp = NULL;
        phy->ip.ip = ip; 
        
}
void make_ip(ethernet *eth, struct phy_iface *iface){
}

void send_frame(ethernet *packet,struct phy_iface *iface){
        const char *buf;
        size_t len;
        HBitWriter *writer = h_bit_writer_new(&system_allocator);
        gen_ethernet(writer,packet);
        buf =  h_bit_writer_get_buffer(writer,&len);
        write(iface->tun_fd,buf,len);
        h_bit_writer_free(writer);
}
void prepare_ethernet(ethernet *eth, struct phy_iface *iface){
        switch(eth->payload.N_type){
        case ARP:
                memcpy(eth->dest,eth->payload.ARP.targethost,sizeof(eth->dest));
                memcpy(eth->src,eth->payload.ARP.senderhost,sizeof(eth->src));

                //TODO: Prepare VLAN trunking. Is a host ever supposed to see VLAN trunking? 
                break;
        case IPFOUR:
                //eth->dest =  0;//ARP here!
                memcpy(eth->src,iface->mac,sizeof(eth->src)); 
                break;
        }
}
void update_arp(phy_iface *i,uint32_t ip, macaddr host){
        
}
void process_arp(arpfour *arp, struct phy_iface *i, ethernet *eth){
        if(arp->operation == 1){
                //Request
                if(arp->targetip == i->ip.ip){
                        //Send response, with our MAC
                        ethernet response;
                        response.payload.N_type = ARP;
                        response.payload.ARP.operation = 2;
                        response.vlan = NULL;
                        memcpy(response.payload.ARP.senderhost,i->mac,sizeof(macaddr));
                        response.payload.ARP.senderip = i->ip.ip;
                        memcpy(response.payload.ARP.targethost,arp->senderhost,sizeof(macaddr));
                        response.payload.ARP.targetip = arp->senderip;
                        prepare_ethernet(&response,i);
                        send_frame(&response,i);
                }
                update_arp(i,arp->senderip,arp->senderhost);
        } else if(arp->operation ==2)
        {                     
                //TODO: anti-spoofing?
                update_arp(i,arp->senderip,arp->senderhost);
        }
}
#define IP_BROADCAST 0xFFFFFFFF
void process_ip(ipfour *ip, struct ip_iface *i){
        //TODO: Handle fragmentation 
        if(ip->dest != i->ip && ip->dest != i->broadcast && ip->dest !=  IP_BROADCAST)
                return; 
        //TODO: Deal with the rest of the protocol
}
void process_icmp(icmp *icmp, struct iface *i,ethernet *eth){
        if(icmp->type == 8){ // Echo request
                ethernet reply;
                memcpy(&reply.dest,eth->src,sizeof(macaddr));
                memcpy(&reply.src,eth->dest,sizeof(macaddr));
                reply.vlan = eth->vlan;
                reply.payload.N_type = ICMP;
                reply.payload.ICMP.type = 0; // Echo reply
                reply.payload.ICMP.code = 0;
                reply.payload.ICMP.data = icmp->data;
                send_frame(&reply,i);
        }
}
// We need to send requests somehow
void process_frame(ethernet *frame, struct phy_iface *i ){
        const macaddr broadcast = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};
        //TODO: handle VLAN ? 
        if(memcmp(frame->dest,i->mac, sizeof(i->mac)) &&  memcmp(frame->dest,broadcast,sizeof(broadcast))){
                return; //Reject frames not sent towards us 
        }
        switch(frame->payload.N_type){
        case ARP: 
                process_arp(&frame->payload.ARP, i, frame);
                break;
        case IPFOUR:
                process_ip(&frame->payload.IPFOUR,&i->ip);
                break;
        case ICMP:
                process_icmp(&frame->payload.ICMP,i, frame);
                break;
        }
}
char buffer[9000];
uint32_t mkip(uint32_t a,uint32_t b,uint32_t c,uint32_t d){
        return a<< 24 | b<<16 | c<<8 | d;
}
int main (int argc, char **argv){
  int tun_fd;
  char tun_name[IFNAMSIZ];
  if(argc<2){
          perror("Usage: <tap interface>\n");
          exit(-1);
  }
  /* Connect to the device */
  strcpy(tun_name, argv[1]);
  tun_fd = tun_alloc(tun_name, IFF_TAP | IFF_NO_PI);  /* tun interface */

  if(tun_fd < 0){
    perror("Allocating interface");
    exit(1);
  }
  struct phy_iface i;
  i.tun_fd = tun_fd;
  macaddr mac = {1,2,3,4,5,6};
  phy_init(&i,&mac,mkip(192,168,1,3));
  /* Now read data coming from the kernel */
  while(1) {
    
          
    NailArena arena;
    struct ethernet *frame;
    /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
    size_t nread = read(tun_fd,buffer,sizeof(buffer));
    if(nread < 0) {
      perror("Reading from interface");
      close(tun_fd);
      exit(1);
    }
    NailArena_init(&arena, 4096);
    printf("Read %d bytes from device %s\n", nread, tun_name);
    frame = parse_ethernet(&arena,buffer,nread);
    if(frame){
            process_frame(frame,&i);
            printf("Frame parsed successfully\n");
    }
    NailArena_release(&arena);
    /* Do whatever with the data */
    
  }
  return 0;
}
