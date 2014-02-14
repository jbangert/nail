%include { #include <sstream>
#include <assert.h>
        typedef std::stringstream sstr;
#define eql_assert(x,y) if(x!=y) assert(!fprintf(stderr,"%s should be %s\n",x.c_str(),y))
        std::ostream & operator<<(std::ostream &out, const std::stringstream *s){
                out << s->str();
                return out;
        }
 }

%token_type {std::stringstream *}
%extra_argument { std::string *outp }

file(A) ::= statements (B). { A = B; *outp = A->str();}


int(A) ::= UINT INT(B). {A=new sstr(); *A<<"h_bits("<<B<<",false)"; }
constint(A) ::= int(len) EQUAL INT(value). {A=new sstr(); *A<<"N_CONSTANT(h_int_range("<<len <<","<<value <<","<<value<<"))";}
        
constrainedint(A) ::= int(len) BAR INT(lower) DOTS INT(upper). {A=new sstr(); *A<<"h_int_range("<<len<<","<<lower <<","<<upper<<")";}
commaset(A) ::=  commaset(b) COMMA INT(value). {A=new sstr(); *A<< value<< "," << b; }
commaset(A) ::= INT(value). {A=value;}
constrainedint(A) ::= int(len) BAR LBRACKET commaset(b) RBRACKET. {eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"h_in((char []){"<<b << "}, strlen((char[]){"<<b<<"}))";}
constrainedint(A) ::=  int(len) BAR NEG LBRACKET commaset(b) RBRACKET. {eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"h_not_in((char []){"<<b<< "}, strlen((char[]){"<<b<<"}))";}

constfield(A)::=  MANY int(len) EQUAL STRING(value).{eql_assert(len->str(),"h_bits(8,false)"); A=new sstr(); *A<<"N_CONSTANT(h_tok("<<value<<"))";}
constfield(a)::= constint(b). {a= b ;}
constfield(A)::= CONSTIDENTIFIER(name). {A=new sstr(); *A<< "N_EMBED("<<name<<")";}
field(a) ::= constfield(b). {a=new sstr(); *a<< "N_CONSTANT("<<b<<")";}
field(a) ::= VARIDENTIFIER(name) parser(p). {a=new sstr(); *a<<"N_FIELD("<<name<<","<< p <<")";}
struct_contents(a) ::= struct_contents(c) field(b).  {a=new sstr(); *a << c << b;}
struct_contents(a) ::= field(b). { a=b;}

parser(x) ::= LCURL struct_contents(c) RCURL. {x=new sstr(); *x<< "N_STRUCT("<<c<<")";}
constfields(A) ::= constfields(B) constfield(C). {A=new sstr(); *A<< B<<" "<< C;}
constfields(A) ::= constfield(C). {A=C;}
parser(X)  ::= LEDGE constfields(before) parser(parser) constfields(after) REDGE. {X=new sstr(); *X<< "N_WRAP(" << before << "," << parser << ","<< after << ")";}
parser(X)  ::= LEDGE parser(parser) constfields(after) REDGE. {X=new sstr(); *X<< "N_WRAP(," << parser << ","<< after << ")";}
parser(X) ::= LEDGE constfields(before) parser(parser) REDGE.{X=new sstr(); *X<< "N_WRAP(" << before << "," << parser << "," << ")";}


option(X) ::= CONSTIDENTIFIER(name) EQUAL parser(p). {X=new sstr(); *X<< "N_OPTION("<<name<<","<<p<<")";}
option(X) ::= VARIDENTIFIER(name) EQUAL parser(p). {X=new sstr(); *X<< "N_OPTION("<<name<<","<<p<<")";}
options(X) ::= options(A) option(B). {X=new sstr(); *X<<A<<B;}
options(X) ::= option(B). {X=B;}                
parser(X)::= CHOOSE LCURL options(opt) RCURL. {X=new sstr(); *X<<"N_CHOICE("<< opt<<")";}
parser(X) ::= MANY parser(p). {X=new sstr(); *X<<"N_ARRAY("<<p<<",h_many)";}
parser(X) ::= MANY1 parser(p). {X=new sstr(); *X<<"N_ARRAY("<<p<<",h_many1)";}
parser(X) ::= SEPBY parser(seperator) parser(p). {X=new sstr(); *X<<"N_SEPBY("<<p<<","<<seperator<<")";}
parser(X) ::= SEPBY1 parser(seperator) parser(p). {X=new sstr(); *X<<"N_SEPBY1("<<p<<","<<seperator<<")";}
parser(X) ::= constint(a). {X=new sstr(); *X<<"N_UINT(uint8_t,"<<a<<")";}
parser(X) ::= constrainedint(A). {X=new sstr(); *X<<"N_UINT(uint8_t,"<<A<<")";}
parser(X) ::= int(A).{X=new sstr(); *X<<"N_UINT(uint8_t,"<<A<<")";}
parser(X) ::= OPTIONAL parser(p). {X=new sstr(); *X<<"N_OPTIONAL("<<p<<")";}
parser(X) ::= REF VARIDENTIFIER(r). {X=new sstr(); *X<<"N_REF("<<r<<")";}
parser(X) ::= VARIDENTIFIER(r). {X=new sstr(); *X<<"N_EMBED("<<r<<")";}
punion(X) ::= parser(p1) OR parser(p2). {X=new sstr(); *X << p1 << "," <<  p2;}
parser(X) ::= punion(p). {X=new sstr(); *X<<"N_UNION("<<p<<")";}
parser(X) ::= LPAREN parser(p) RPAREN. {X=p;}


statement(X) ::= VARIDENTIFIER(name) EQUAL parser(def). {X=new sstr(); *X<<"N_DEFPARSER("<<name<<","<<def<<");";}
statement(X) ::= CONSTIDENTIFIER(name) EQUAL parser(def). {X=new sstr(); *X<<"N_DEFPARSER("<<name<<","<<def<<")";}
statements(X) ::= statements(A) statement(B). {X=new sstr(); *X<<A<<B<<std::endl;}
statements(X) ::= statement(A). {X=new sstr(); *X<<A<<std::endl;}
