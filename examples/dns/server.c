#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <string.h>
#include <hammer/hammer.h>
#include "grammar.h"
#define N_MACRO_IMPLEMENT
#include "grammar.h"

int main(int argc, char** argv) {
    
  // set up a listening socket...
        // int sock = start_listening();

  uint8_t packet[8192]; // static buffer for simplicity
  ssize_t packet_size;
  struct sockaddr_in remote;
  socklen_t remote_len;

  while(1){
          if(feof(stdin))
                  break;
          packet_size = fread(packet, 1, sizeof(packet), stdin);
          
          const struct dns_message *message = parse_dns_message(packet, packet_size);
          if (!message) {
                  printf("Invalid packet; ignoring\n");
                  continue;
          }
          printf("Parsed\n");

          print_dns_message(message,stderr,0);
  }
}
#if 0

  while (1) {
    remote_len = sizeof(remote);
     packet_size = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr*)&remote, &remote_len);
    // dump the packet...
    for (int i = 0; i < packet_size; i++)
      printf(".%02hhx", packet[i]);
    
    printf("\n");

    const struct dns_domain *message = parse_domain(packet, packet_size);
    if (!message) {
      printf("Invalid packet; ignoring\n");
      continue;
    }
    printf("Parsed");
    (void)message;
    for (size_t i = 0; i < message->header.question_count; i++) {
      struct dns_question *question = &message->questions[i];
      printf("Recieved %s %s request for ", CLASS_STR[question->qclass], TYPE_STR[question->qtype]);
      for (size_t j = 0; j < question->qname.qlen; j++)
	printf("%s.", question->qname.labels[j].label);
      printf("\n");
      
    }
    printf("%p\n", content);




    
    // Not much time to actually implement the DNS server for the talk, so here's something quick and dirty. 
    // Traditional response for this time of year...
    uint8_t response_buf[4096];
    uint8_t *rp = response_buf;
    // write out header...
    *rp++ = message->header.id >> 8;
    *rp++ = message->header.id & 0xff;
    *rp++ = 0x80 | (message->header.opcode << 3) | message->header.rd;
    *rp++ = 0x0; // change to 0 for no error...
    *rp++ = 0; *rp++ = 1; // QDCOUNT
    *rp++ = 0; *rp++ = 1; // ANCOUNT
    *rp++ = 0; *rp++ = 0; // NSCOUNT
    *rp++ = 0; *rp++ = 0; // ARCOUNT
    // encode the first question...
    {
      struct dns_question *question = &message->questions[0];
      format_qname(&question->qname, &rp);
      *rp++ = (question->qtype >> 8) & 0xff;
      *rp++ = (question->qtype     ) & 0xff;
      *rp++ = (question->qclass >> 8) & 0xff;
      *rp++ = (question->qclass     ) & 0xff;

      // it's a cname...
      format_qname(&question->qname, &rp);
      *rp++ = 0; *rp++ = 5;
      *rp++ = (question->qclass >> 8) & 0xff;
      *rp++ = (question->qclass     ) & 0xff;
      *rp++ = 0; *rp++ = 0; *rp++ = 0; *rp++ = 0; // TTL.
      //const char cname_rd[14] = "\x09spargelze\x02it";
      *rp++ = 0; *rp++ = 14;
      memcpy(rp, "\x09spargelze\x02it", 14);
      rp += 14;
    }
    // send response.
    sendto(sock, response_buf, (rp - response_buf), 0, (struct sockaddr*)&remote, remote_len);

  }
  return 0;
}
#endif

