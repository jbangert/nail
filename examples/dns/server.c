
#include "common.h"


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
int domain_cmp(domain *a, labels *b){
        if(a->count != b->count)
                return 1;
        for(int i=0;i<a->count;i++){
                if(a->elem[i].count != b->elem[i].label.count)// This is because we don't have
                                                                    // transforms in the wrap comparison
                        return 1;
                if(memcmp(a->elem[i].elem,  b->elem[i].label.elem, a->elem[i].count))
                        return 1;
        }
        return 0;
}
int hashsize(zone *zon){
        return 3*zon->count/2;
}
unsigned long domain_hash(domain *a){ // djb2
        unsigned long hash = 5381;
        FOREACH(label, *a){
                FOREACH(ch,*label){
                        hash = ((hash<<5) + hash) + *ch; 
                }
        }
        return hash;
}
struct hashentry{
        unsigned long hash;
        definition *def;
};
struct hashentry *zone_hashtable(zone *zon){
        struct hashentry * table= calloc(sizeof(struct hashentry),hashsize(zon)); //COLLISIONS
        FOREACH(domain,*zon){
          unsigned long hash = domain_hash((struct domain *)domain); //lazy hack, they are the same struct 
                int entry = hash%hashsize(zon);
                while(table[entry].def!=NULL) entry = (entry + 1) % hashsize(zon);
                table[entry].hash = hash;
                table[entry].def= domain;
        }
        return table;
}
//responses are a bit hackish - should really put those into the grammar too
void domain_response(NailArena *tmp_arena,answer *rr, domain *dom){
        labels *l =(labels *)dom;
        NailOutStream stream;
        NailOutStream_init(&stream,256); // TODO: Allocate streams from arena, and/or free this one somewhere!
        gen_labels(tmp_arena,&stream,l);
        size_t count;
        rr->rdata.elem = (uint8_t*)NailOutStream_buffer(&stream, &count);
        rr->rdata.count = count;
        //TODO: We need to free this
}
int natoi(char *s, int n)
{
    int x = 0;
    while(n--)
    {
    	x = x * 10 + (s[0] - '0');		
    	s++;
    }
    return x;
}
int dns_respond(NailOutStream *stream,NailArena * arena,struct dnspacket *query, zone *zone, struct hashentry *hashtable)
{
        dnspacket response;// = n_malloc(arena,sizeof(*response));
        char *retval;

        response.id = query->id;
        response.qr = 1;
        response.opcode = 0;
        response.rd = query->rd;
        response.aa = 1;
        response.tc = 0; /*XXX: TODO: Do truncation*/
        response.ra = 0; /* We don't do recursion*/
        response.questions  = query->questions;
        response.additional.count = 0;
        response.authority.count = 0;
        response.rcode = 0;
        narray_alloc(response.responses,arena,query->questions.count);
        for(int i=0;i<query->questions.count;i++){
                question *q =  &response.questions.elem[i];
                unsigned long hash = domain_hash((domain *) &q->labels.labels);
                int entry = hash%hashsize(zone);
                for(;hashtable[entry].def != NULL;entry = (entry+1)%hashsize(zone) ){
                        definition *j = hashtable[entry].def;
                        if(hashtable[entry].hash != hash)
                                continue;
                        if(domain_cmp(&j->hostname, &q->labels.labels))
                                continue;
                        if(!((j->rr.N_type == CNAME) ||
                           (q->qtype == QTYPE_ANY) ||
                             (q->qtype == TYPE_A && j->rr.N_type ==A) ||
                             (q->qtype == TYPE_MX && j->rr.N_type== MX)||
                             (q->qtype == TYPE_NS && j->rr.N_type== NS)))
                                continue;
                        answer *rr= &response.responses.elem[i];
                        rr->class =1;// IP
                        rr->labels = q->labels;
                        rr->ttl = 60;
                        switch(j->rr.N_type){
                        case A:
                                rr->rtype = TYPE_A;
                                //definitely put this in the grammar?
                                rr->rdata.count = 4;
                                rr->rdata.elem = n_malloc(arena,4);
                                if(j->rr.a.count < 4)
                                        continue;
                                for(int i=0;i<4;i++)
                                        rr->rdata.elem[i] = natoi(j->rr.a.elem[i].elem,j->rr.a.elem[i].count);
                                goto success;
                        case NS:
                                rr->rtype = TYPE_NS;
                                domain_response(arena,rr,&j->rr.ns);
                                goto success;
                        case MX:
                                rr->rtype = TYPE_MX;
                                domain_response(arena,rr,&j->rr.ns);
                                goto success;
                        case CNAME:
                                rr->rtype = TYPE_CNAME;
                                domain_response(arena,rr,&j->rr.cname);
                                goto success;
                        }
                }
                response.responses.count = 0;
                response.rcode = 3;// NXDOMAIN
                break;
        success:;
        }
        return gen_dnspacket(arena, stream,&response);
}
#define ZONESIZ 4096*1024
void memstream(NailStream *str, char *buf,size_t size){
        str->data = buf;
        str->size = size;
        str->pos = 0;
        str->bit_offset =0;
}
int main(int argc, char** argv) {
  uint8_t packet[8192]; // static buffer for simplicity
  ssize_t packet_size;
  struct sockaddr_in remote;
  socklen_t remote_len;
  if(argc<2) exit(-1);
  char *zonebuf = malloc(ZONESIZ);
  FILE *zonefil = fopen(argv[1],"r");
  jmp_buf oom;

  NailArena permanent;
  if(0!= setjmp(oom)){
    printf("OOM during init\n");
    exit(-1);
  }
  NailArena_init(&permanent,4096,&oom);
  if(!zonebuf || !zonefil) exit(-1);
  remote_len = fread(zonebuf,1,ZONESIZ,zonefil);
  zone * zon = parse_zone(&permanent,zonebuf, remote_len);
  if(!zon) {fprintf(stderr,"Cannot parse zone\n"); exit(-1);}
  free(zonebuf);
  fclose(zonefil);
  printf("Parsed zone;\n");
  struct hashentry *hashtable = zone_hashtable(zon);
  int sock = start_listening();
  while (1) {
    NailArena arena;
    NailOutStream out;
    struct dnspacket *message;
    const char *response;
    size_t len;
    jmp_buf err;
    remote_len = sizeof(remote);
    packet_size = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr*)&remote, &remote_len);
    if(0 != setjmp(err)){
      printf("OOM\n");
      NailArena_release(&arena);
      continue;
    }
    NailArena_init(&arena,4096,&err);
    
    NailOutStream_init(&out,4096);
    message = parse_dnspacket(&arena,packet, packet_size);
    if (!message) {
      printf("Invalid packet; ignoring\n");
      continue;
    }
    assert(dns_respond(&out, &arena,message,zon,hashtable ) == 0);
    response = NailOutStream_buffer(&out,&len);
    sendto(sock, response, len, 0, (struct sockaddr*)&remote, remote_len);
    NailArena_release(&arena);
    NailOutStream_release(&out);
}
return 0;
}


