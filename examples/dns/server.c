#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <string.h>
#include <hammer/hammer.h>
#include "server.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"

extern HAllocator system_allocator;
#define narray_alloc(arr, aren, cnt) arr.count = cnt; arr.elem= h_arena_malloc(aren,cnt * sizeof(arr.elem[0]))
#define narray_string(arr,string) arr.count = strlen(string); arr.elem = string;

///
// Main Program for a Dummy DNS Server, from hammer/example
///

int start_listening() {
  // return: fd
  int sock;
  struct sockaddr_in addr;

  sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    err(1, "Failed to open listning socket");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    err(1, "Bind failed");
  return sock;
}
const char *dns_respond(size_t *len,struct dns_message *query)
{
        HArena *arena = h_new_arena(&system_allocator,0);
        HBitWriter *writer = h_bit_writer_new(&system_allocator);
        dns_message *response = h_arena_malloc(arena,sizeof(*response));
        char *retval;
        int i;
        response->header.id = query->header.id;
        response->header.qr = 1;
        response->header.rd = query->header.rd;
        response->header.aa = 1;
        response->header.ra = 0; /* We don't do recursion*/
        response->header.question_count = query->questions.count;
        response->header.answer_count = query->questions.count; 
        response->questions  = query->questions;
        // We respond CNAME spargelze.it to everyone 
        narray_alloc(response->rr,arena,query->questions.count);
        for(i=0;i<query->questions.count;i++){
                dns_question *q =  &response->questions.elem[i];
                dns_response *rr = &response->rr.elem[i];
                rr->labels = q->labels;
                rr->rtype = 5; //DNS
                rr->class = 255; //ANY 
                rr->ttl = 60;
                {
                        /* dirty parser nesting. TODO: upgrade grammar*/
                        dns_labels labels;
                        HBitWriter *label_buffer = h_bit_writer_new(&system_allocator);
                        size_t count;
                        narray_alloc(labels,arena,2);
                        narray_string(labels.elem[0],"spargelze");
                        narray_string(labels.elem[1],"it");
                        assert(gen_dns_labels(label_buffer,&labels));
                        rr->rdata.elem = h_bit_writer_get_buffer(label_buffer, &count);
                        rr->rdata.count = count;
                }

        }
        if(!gen_dns_message(writer,response))
                return NULL;
        return h_bit_writer_get_buffer(writer,len);
}
int main(int argc, char** argv) {
        
  int sock = start_listening();

  uint8_t packet[8192]; // static buffer for simplicity
  ssize_t packet_size;
  struct sockaddr_in remote;
  socklen_t remote_len;
  while (1) {
          struct dns_message *message;
          const char *response;
          size_t len;
          remote_len = sizeof(remote);
          packet_size = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr*)&remote, &remote_len);
          // dump the packet...
          /* for (int i = 0; i < packet_size; i++)
             printf(".%02hhx", packet[i]);
             
             printf("\n");*/
          
          message = parse_dns_message(packet, packet_size);
          if (!message) {
                  printf("Invalid packet; ignoring\n");
                  continue;
          }
          print_dns_message(message,stderr,0);
          response = dns_respond(&len,message );
          assert(response);
          sendto(sock, response, len, 0, (struct sockaddr*)&remote, remote_len);

}
return 0;
}


