%include { #include <sstream>
#include <assert.h>
        typedef std::stringstream sstr;
#define eql_assert(x,y) if(x!=y) assert(!fprintf(stderr,"%s should be %s\n",x.c_str(),y))
        std::ostream & operator<<(std::ostream &out, const std::stringstream *s){
                out << s->str();
                return out;
        }
 }
%syntax_error {fprintf(stderr, "Syntax error new token %s", TOKEN->str().c_str());}
%token_type {std::stringstream *}
%extra_argument { std::string *outp }

file(A) ::= statements (B). { A = B; *outp = A->str();}


int(A) ::= UINT INT(B). {A=new sstr(); *A<<"h_bits("<<B<<",false)"; }
constint(A) ::= int(len) EQUAL INT(value). {A=new sstr(); *A<<"N_UINT(uint8_t,h_int_range("<<len <<","<<value <<","<<value<<"))";}
        
constrainedint(A) ::= int(len) BAR INT(lower) DOTS INT(upper). {A=new sstr(); *A<<"h_int_range("<<len<<","<<lower <<","<<upper<<")";}
commaset(A) ::=  commaset(b) COMMA INT(value). {A=new sstr(); *A<< value<< "," << b; }
commaset(A) ::= INT(value). {A=value;}
constrainedint(A) ::= int(len) BAR LBRACKET commaset(b) RBRACKET. {eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"h_in((char []){"<<b << "}, strlen((char[]){"<<b<<"}))";}
constrainedint(A) ::=  int(len) BAR NEG LBRACKET commaset(b) RBRACKET. {eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"h_not_in((char []){"<<b<< "}, strlen((char[]){"<<b<<"}))";}
constparser(A)::=  MANY int(len) EQUAL STRING(value).{eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"h_tok("<<value<<")";}
constparser(a)::= constint(b). {a= b ;}
constparser(A)::= CONSTIDENTIFIER(name). {A=new sstr(); *A<< "N_PARSER("<<name<<")";}
constparser(A)::= MANY constparser(b). {A=new sstr(); *A<<"N_ARRAY(h_many,"<<b<<")";}
        
constparser(A)::= constparser(a) OR constint(b). {A=new sstr(); *A<<"N_CHOICE(N_OPTION(U1,"<<a<<") N_OPTION(U2,"<<b<<"))";}
constparser(A)::= LCURL constfields(a) RCURL. {A=new sstr(); *A<<"N_STRUCT("<<a<<")";}
constfields(A) ::= constfields(B) constparser(C). {A=new sstr(); *A<< B<<" N_CONSTANT("<< C<<")";}
        constfields(A) ::= constparser(C). {A= new sstr(); *A<<"N_CONSTANT("<<C<<")";}
                
field(a) ::= constparser(b). {a=new sstr(); *a<< "N_CONSTANT(" << b<< ")";}
field(a) ::= VARIDENTIFIER(name) parser(p). {a=new sstr(); *a<<"N_FIELD("<<name<<","<< p <<")";}
struct_contents(a) ::= struct_contents(c) field(b).  {a=new sstr(); *a << c << b;}
struct_contents(a) ::= field(b). { a=b;}

parser(x) ::= LCURL struct_contents(c) RCURL. {x=new sstr(); *x<< "N_STRUCT("<<c<<")";}
parser(X)  ::= LEDGE constfields(before) parser(parser) constfields(after) REDGE. {X=new sstr(); *X<< "N_WRAP(" << before << "," << parser << ","<< after << ")";}
parser(X)  ::= LEDGE parser(parser) constfields(after) REDGE. {X=new sstr(); *X<< "N_WRAP(," << parser << ","<< after << ")";}
parser(X) ::= LEDGE constfields(before) parser(parser) REDGE.{X=new sstr(); *X<< "N_WRAP(" << before << "," << parser << "," << ")";}


option(X) ::= CONSTIDENTIFIER(name) EQUAL parser(p). {X=new sstr(); *X<< "N_OPTION("<<name<<","<<p<<")";}
option(X) ::= VARIDENTIFIER(name) EQUAL parser(p). {X=new sstr(); *X<< "N_OPTION("<<name<<","<<p<<")";}
options(X) ::= options(A) option(B). {X=new sstr(); *X<<A<<B;}
options(X) ::= option(B). {X=B;}                
parser(X) ::= CHOOSE LCURL options(opt) RCURL. {X=new sstr(); *X<<"N_CHOICE("<< opt<<")";}
parser(X) ::= MANY parser(p). {X=new sstr(); *X<<"N_ARRAY(h_many,"<<p<<")";}
parser(X) ::= MANY1 parser(p). {X=new sstr(); *X<<"N_ARRAY(h_many,"<<p<<")";}
parser(X) ::= SEPBY constparser(seperator) parser(p). {X=new sstr(); *X<<"N_SEPBY("<<seperator<<","<<p<<")";}
parser(X) ::= SEPBY1 constparser(seperator) parser(p). {X=new sstr(); *X<<"N_SEPBY1("<<seperator<<","<<p<<")";}
parser(X) ::= constrainedint(A). {X=new sstr(); *X<<"N_UINT(uint8_t,"<<A<<")";}
parser(X) ::= int(A).{X=new sstr(); *X<<"N_UINT(uint8_t,"<<A<<")";}
parser(X) ::= OPTIONAL parser(p). {X=new sstr(); *X<<"N_OPTIONAL("<<p<<")";}
parser(X) ::= REF VARIDENTIFIER(r). {X=new sstr(); *X<<"N_REF("<<r<<")";}
parser(X) ::= VARIDENTIFIER(r). {X=new sstr(); *X<<"N_PARSER("<<r<<")";}
punion(X) ::= parser(p1) OR parser(p2). {X=new sstr(); *X << "N_OPTION(PICK1,"<<p1 << ") N_OPTION(PICK2," <<  p2 << ")";}
parser(X) ::= punion(p). {X=new sstr(); *X<<"N_CHOICE("<<p<<")";}
parser(X) ::= LPAREN parser(p) RPAREN. {X=p;}

statement(X) ::= VARIDENTIFIER(name) EQUAL parser(def). {X=new sstr(); *X<<"N_DEFPARSER("<<name<<","<<def<<")";}
statement(X) ::= CONSTIDENTIFIER(name) EQUAL constparser(def). {X=new sstr(); *X<<"N_DEFPARSER("<<name<<","<<def<<")";}
statements(X) ::= statements(A) statement(B). {X=new sstr(); *X<<A<<B<<std::endl;}
statements(X) ::= statement(A). {X=new sstr(); *X<<A<<std::endl;}
