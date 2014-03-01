#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <string.h>



#include "generated.h"

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
char *dns_respond(size_t *len,struct dnspacket *query)
{
        HArena *arena = h_new_arena(&system_allocator,0);
        HBitWriter *writer = h_bit_writer_new(&system_allocator);
        dnspacket *response = h_arena_malloc(arena,sizeof(*response));
        char *retval;
        int i;
        response->id = query->id;
        response->qr = 1;
        response->rd = query->rd;
        response->aa = 1;
        response->ra = 0; /* We don't do recursion*/
        response->questions  = query->questions;
        // We respond CNAME spargelze.it to everyone 
        narray_alloc(response->responses,arena,query->questions.count);
        for(i=0;i<query->questions.count;i++){
                answer *rr= &response->responses.elem[i];
                question *q =  &response->questions.elem[i];

                rr->labels = q->labels;
                rr->rtype = 5; //DNS
                rr->class = 255; //ANY 
                rr->ttl = 60;
                {
                        /* dirty parser nesting. TODO: upgrade grammar*/
                        labels labels;
                        HBitWriter *label_buffer = h_bit_writer_new(&system_allocator);
                        size_t count;
                        narray_alloc(labels,arena,2);
                        narray_string(labels.elem[0].label,"spargelze");
                        narray_string(labels.elem[1].label,"it");
                        gen_labels(label_buffer,&labels);
                        rr->rdata.elem = h_bit_writer_get_buffer(label_buffer, &count);
                        rr->rdata.count = count;
                }

        }
        gen_dnspacket(writer,response);
        return h_bit_writer_get_buffer(writer,len);
}
int main(int argc, char** argv) {
        
  int sock = start_listening();

  uint8_t packet[8192]; // static buffer for simplicity
  ssize_t packet_size;
  struct sockaddr_in remote;
  socklen_t remote_len;
  while (1) {
          NailArena arena;
          struct dns_message *message;
          char *response;
          size_t len;
          remote_len = sizeof(remote);
          packet_size = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr*)&remote, &remote_len);
          // dump the packet...
          /* for (int i = 0; i < packet_size; i++)
             printf(".%02hhx", packet[i]);
             
             printf("\n");*/
          NailArena_init(&arena,4096);
          message = parse_dnspacket(&arena,packet, packet_size);
          if (!message) {
                  printf("Invalid packet; ignoring\n");
                  continue;
          }
//          print_dnspacket(message,stderr,0);
          response = dns_respond(&len,message );
          assert(response);
          sendto(sock, response, len, 0, (struct sockaddr*)&remote, remote_len);
          NailArena_release(&arena);

}
return 0;
}


