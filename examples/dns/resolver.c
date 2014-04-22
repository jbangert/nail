#include "common.h"

int make_question(NailArena *arena,char *domain,int sockfd){
        size_t len;
        dnspacket p;
        NailStream out;
        NailOutStream_init(&out,512);
        memset(&p,0,sizeof(p));
        FILE* rand = fopen("/dev/urandom","r");
        fread(&p.id,2,1,rand);
        p.rd=1;
        narray_alloc(p.questions,arena,1);
        p.questions.elem[0].qtype  = TYPE_A;
        p.questions.elem[0].qclass =1; 
        labels * labels = parse_domain(arena,domain,strlen(domain));
        if(!labels){
                fprintf(stderr,"%s is not a valid domain\n",domain);
                exit(-1);
        }
        p.questions.elem[0].labels.labels = *labels;
        gen_dnspacket(arena,&out,&p);
        char *buf = NailOutStream_buffer(&out,&len);
        send(sockfd,buf,len,0);
        NailOutStream_release(&out);
        return p.id;
}
void print_domain(domain *lbls){
        FOREACH(l,*lbls){
                fprintf(stderr,"%.*s.",l->count,l->elem);
        }
}
void domainresponse(NailArena *arena,answer *a){
        labels *l = parse_labels(arena,a->rdata.elem,a->rdata.count);
        if(!l){ fprintf(stderr,"Invalid RDATA\n"); exit(-1);}
        print_domain(l);
}
void wait_reply(NailArena *arena,int sockfd,int id){
        char buf[1024];
        size_t siz = recv(sockfd,buf,sizeof buf,0);
        dnspacket *dns  = parse_dnspacket(arena,buf,siz);
        if(!dns){
                fprintf(stderr,"Received bad reply\n");
                exit(-1);
        }
        if(dns->id != id){
                fprintf(stderr,"spoofing detected\n");
        }
        if(dns->rcode != 0){
                fprintf(stderr,"Received rcode %d\n", dns->rcode);
        }
        FOREACH(response,dns->responses){
                //TODO: We aren't checking whether this sever is allowed to respond
                print_domain(&response->labels);
                switch(response->rtype){
                case TYPE_NS:
                        printf(":NS:");
                        domainresponse(arena,response);
                        break;
                case TYPE_MX:
                        printf(":MX:");
                        domainresponse(arena,response);
                        break;
                case TYPE_CNAME:
                        printf(":CNAME:"); // keep retrying?
                        domainresponse(arena,response);
                        break;
                case TYPE_A:
                        if(response->rdata.count<4){
                                fprintf(stderr,"Malformed A Rdata\n");
                                exit(-1);
                        }
                        printf(":A:%d.%d.%d.%d",response->rdata.elem[0], response->rdata.elem[1], response->rdata.elem[2], response->rdata.elem[3]);
                        break;
                }
                printf("\n");
        }
        
}
int main(int argc, char**argv)
{
        int sockfd;
        NailArena arena;
        size_t len;
        struct sockaddr_in servaddr,recvaddr;
   NailArena_init(&arena, 4096);
   if (argc != 2)
   {
      printf("usage:  resolver <FQDN> \n");
      exit(1);
   }
   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   bzero(&servaddr,sizeof(recvaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr("8.8.8.8");
   servaddr.sin_port=htons(53);

   connect(sockfd,&servaddr, sizeof(servaddr));
  int id= make_question(&arena,argv[1],sockfd);
  wait_reply(&arena,sockfd,id);
}
