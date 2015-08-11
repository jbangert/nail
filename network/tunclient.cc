#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <time.h>

#include <unordered_map> 
#include "parse.hh"


//http://backreference.org/2010/03/26/tuntap-interface-tutorial/
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  const char *clonedev = "/dev/net/tun";

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
} arpentry;
#define ARPSIZE 512
struct phy_iface;
struct ip_iface{
  uint32_t ip;
  uint32_t broadcast;
  phy_iface *phy;
ip_iface(phy_iface *parent, uint32_t _ip, uint32_t _broadcast) : phy(parent), ip(_ip), broadcast(_broadcast){}
    };
typedef struct phy_iface{
int tun_fd;
macaddr mac;

std::unordered_map<uint32_t,arpentry> arp;
struct ip_iface ip; // TODO: Add multi-IP support?
  phy_iface(int fd,macaddr MAC, uint32_t ip_addr,uint32_t broadcast, uint32_t netmask,uint32_t gateway):  tun_fd(fd), ip(this,ip_addr,broadcast){
     memcpy(mac, MAC, sizeof mac);
  }
} phy_iface;

int send_frame(NailArena *arena,ethernet *packet,struct phy_iface *iface){
        const char *buf;
        size_t len;
        struct NailOutStream out;
        NailOutStream_init(&out,4096);
        gen_ethernet(arena,&out,packet);

        write(iface->tun_fd,out.data, out.pos);
        NailOutStream_release(&out);
        return 0; //XXX: Error handling for retransmit? 
}
void prepare_ethernet(ethernet *eth, struct phy_iface *iface){
  eth->vlan = NULL;
        switch(eth->payload.N_type){
        case ARP:
                memcpy(eth->dest,eth->payload.arp.targethost,sizeof(eth->dest));
                memcpy(eth->src,eth->payload.arp.senderhost,sizeof(eth->src));

                //TODO: Prepare VLAN trunking. Is a host ever supposed to see VLAN trunking? 
                break;
        case IPFOUR:
                //eth->dest =  0;//ARP here!
          memset(eth->dest, 0xFF,sizeof eth->dest);// TODO: ARP!!!
          
             memcpy(eth->src,iface->mac,sizeof(eth->src)); 
             break;
             }
}
int send_ipfour(NailArena *arena,ip_iface *i,uint32_t dst, uint32_t src, struct ip_payload *payload){
  ethernet eth;
  eth.payload.N_type = IPFOUR;
  eth.payload.ipfour.packet.ttl = 255;
  eth.payload.ipfour.packet.payload = *payload;
  eth.payload.ipfour.packet.source = src;
  eth.payload.ipfour.packet.dest = dst;
   prepare_ethernet(&eth,i->phy);
  return send_frame(arena, &eth, i->phy);
}
void update_arp(phy_iface *i,uint32_t ip, macaddr host){
  auto entry = i->arp[ip];
  entry.ip = ip;
  memcpy(entry.mac,host, sizeof entry.mac);
  entry.time  = time(NULL);
    
}
void process_arp(NailArena *arena,arpfour *arp, struct phy_iface *i, ethernet *eth){
        if(arp->operation == 1){
                //Request
                if(arp->targetip == i->ip.ip){
                        //Send response, with our MAC
                        ethernet response;
                        response.payload.N_type = ARP;
                        response.payload.arp.operation = 2;
                        response.vlan = NULL;
                        memcpy(response.payload.arp.senderhost,i->mac,sizeof(macaddr));
                        response.payload.arp.senderip = i->ip.ip;
                        memcpy(response.payload.arp.targethost,arp->senderhost,sizeof(macaddr));
                        response.payload.arp.targetip = arp->senderip;
                        prepare_ethernet(&response,i);
                        send_frame(arena,&response,i);
                }
                update_arp(i,arp->senderip,arp->senderhost);
        } else if(arp->operation ==2)
        {                     
                //TODO: anti-spoofing?
                update_arp(i,arp->senderip,arp->senderhost);
        }
}
void process_icmp(NailArena *arena,icmp *icmp, uint32_t sender, struct ip_iface *i){
        if(icmp->data.N_type == ECHOREQUEST){ // Echo request
          ip_payload payload;
          payload.N_type = ICMP;
          payload.icmp = *icmp;
          payload.icmp.data.N_type = ECHORESPONSE;
          send_ipfour(arena,i, sender, i->ip, &payload);
        }
}
#define IP_BROADCAST 0xFFFFFFFF
void process_ip(NailArena *arena,ipfour *ip, struct ip_iface *i){
        //TODO: Handle fragmentation 
        if(ip->packet.dest != i->ip && ip->packet.dest != i->broadcast && ip->packet.dest !=  IP_BROADCAST)
                return;
        switch(ip->packet.payload.N_type){
        case UDP:
          break;
        case ICMP:
          process_icmp(arena, &ip->packet.payload.icmp,ip->packet.source, i);
          break;
        }
}
// We need to send requests somehow
void process_frame(NailArena *arena,ethernet *frame, struct phy_iface *i ){
        const macaddr broadcast = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};
        //TODO: handle VLAN ? 
        if(memcmp(frame->dest,i->mac, sizeof(i->mac)) &&  memcmp(frame->dest,broadcast,sizeof(broadcast))){
                return; //Reject frames not sent towards us 
        }
        switch(frame->payload.N_type){
        case ARP: 
          process_arp(arena,&frame->payload.arp, i, frame);
                break;
        case IPFOUR:
          process_ip(arena,&frame->payload.ipfour,&i->ip);
                break;
        }
}
unsigned char buffer[9000];
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
  macaddr mac = {1,2,3,4,5,6};
  phy_iface i(tun_fd,mac, mkip(192,168,1,3),mkip(192,168,1,255),mkip(255,255,255,0),mkip(192,168,1,1));
  /* Now read data coming from the kernel */
  while(1) {
    
          
    NailArena arena;
    jmp_buf oom;
    if(0!= setjmp(oom)){
      printf("OOM;\n");
      NailArena_release(&arena);
      continue;
    }
    struct ethernet *frame;
    /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
    ssize_t nread = read(tun_fd,buffer,sizeof(buffer));
    if(nread < 0) {
      perror("Reading from interface");
      close(tun_fd);
      exit(1);
    }
    NailArena_init(&arena, 4096,&oom);
    printf("Read %ld bytes from device %s\n", nread, tun_name);
    //    NailMemStream packetstream(buffer, nread);
    frame = p_ethernet(&arena,buffer,nread);
    if(frame){
      process_frame(&arena,frame,&i);
      printf("Frame parsed successfully\n");
    }
    NailArena_release(&arena);
    /* Do whatever with the data */
    
  }
  return 0;
}
