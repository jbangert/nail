#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#define p_arg(x) x.count,(const char*)x.elem

#define printf(...)  ERROR

static unsigned long strntoud(unsigned siz,const char *num){
  unsigned i=0;
  unsigned long ret=0;
  for(;i<siz;i++){
    assert(num[i] >= '0' && num[i] <= '9');
    ret= 10 * ret+ num[i] - '0';
  }
  return ret;
}
class GenGenerator{
  std::ostream &out;
  int num_iters;
  void constint(int width, std::string value){
    out << "if(!stream_output(str_current," << value << "," << width << ")) return 0;";
  }
  void generator(constarray &c){ 
    switch(c.value.N_type){
    case STRING:
      assert(mk_str(c.parser.unsign) == "8");
      FOREACH(ch, c.value.string){
        constint(8,(boost::format("'%c'") % ch).str());
      }
      break;
    case VALUES:{
      int width = boost::lexical_cast<int>(mk_str(c.parser.unsign));
      FOREACH(v,c.value.values){
        constint(width, intconstant_value(*v));
      }
      break;
    }
    }
  }
  void generator(constparser &p){
    switch(p.N_type){
    case CARRAY:
      generator(p.carray); break;
    case CREPEAT:
      generator(*p.crepeat); break; //Emit just one of the elements
    case CINT: {
      int width = boost::lexical_cast<int>(mk_str(p.cint.parser.unsign));
      constint(width, intconstant_value(p.cint.value));
    }
      break;
    case CREF:
      out << "gen_"<< mk_str(p.cref)<<"(out);";break;
    case CSTRUCT:
      FOREACH(field,p.cstruct){
        generator(*field);
      }
      break;
    case CUNION:
      generator(p.cunion.elem[0]);
      break;
    }
  }
  void generator(parserinner &p,Expr &val, const Scope &scope){
    switch(p.N_type){
    case INTEGER:{
      std::stringstream valstr; 
      valstr << val;
      int width = boost::lexical_cast<int>(mk_str(p.integer.parser.unsign));
      if(p.integer.constraint){
        out << "if(";
        constraint(out,valstr.str(),*p.integer.constraint);
        out << "){return 0;}";
      }
      out << "if(!stream_output(str_current,"<<val<<","<< width << ")) return 0;";
      break;
    }
    case STRUCTURE:{
      std::stringstream fixup;
      Scope newscope(scope);
      fixup << "{/*Context-rewind*/\n NailStreamPos  end_of_struct= stream_getpos(str_current);\n";
      FOREACH(field, p.structure){
        switch(field->N_type){
        case CONSTANT:
          generator(field->constant);
          break;
        case FIELD:{
          ValExpr fieldname(mk_str(field->field.name),&val);
          generator(field->field.parser->pr,fieldname,newscope);
          break;
        }
        case DEPENDENCY:{
          //TODO: Handle other fixed-size dependencies, such as structures
          std::string post;
          parser &p = *field->dependency.parser;
          std::string name = mk_str(field->dependency.name);
          if(p.pr.N_type != INTEGER){
            assert(!"Cannot generate non-integer dependency fields yet;");
          }
          int width =  boost::lexical_cast<int>(mk_str(p.pr.integer.parser.unsign));
          out << typedef_type(p,"",&post) << " dep_"<< name << ";";
          assert(post == "");
          out << "NailStreamPos rewind_"<< name << "=stream_getpos(str_current);";
          out << "stream_output(str_current,0,"<<width<<");";
          fixup << "stream_reposition(str_current, rewind_"<<name<<");";
          fixup << "stream_output(str_current,dep_"<<name<<","<<width<<");";
          break;
        }
        case TRANSFORM:{ 
          std::string cfunction = mk_str(field->transform.cfunction);
          FOREACH(stream, field->transform.left){
              out << "NailStream str_" << mk_str(*stream) <<";\n";
              out << "if(NailOutStream_init(&str,4096)) {return NULL;}\n";
              fixup << "NailOutStream_release(&str);"
              scope.add_stream_definition(mk_str(*stream));
          }

          // Transforms need to be invoked in reverse order
          
        }
          break;
        case APPLY:{ 

          out << "{/*APPLY*/";
          out << "NailStream  * orig_str = str_current;\n";
          out << "str_current =" << scope.stream_ptr(mk_str(parser.pr.apply.stream)) << ";";
          
        }
          break;
        }
      }
      fixup <<  "stream_reposition(str_current, end_of_struct);";
      //TODO: Do a proper way of updating these!
      out << fixup.str();
      out << "}";
    }
      break;
    case WRAP:
      if(p.wrap.constbefore){
        FOREACH(c,*p.wrap.constbefore){
          generator(*c);
        }
      }
      generator(p.wrap.parser->pr,val,scope);
      if(p.wrap.constafter){
        FOREACH(c,*p.wrap.constafter){
          generator(*c);
        }
      }      
      break;

    case CHOICE:
      {
        out << "switch("<< ValExpr("N_type",&val)<<"){\n";
        FOREACH(c, p.choice){
          std::string tag = mk_str(c->tag);
          std::string enum_tag = tag;
          boost::algorithm::to_lower(tag);
          out << "case " << enum_tag << ":\n";
          ValExpr expr(tag,&val);
          generator(c->parser->pr, expr, scope );
          out << "break;\n";
        }
        out << "}";
      }      
      break;
    case NUNION:
      {
        //What to do with UNION? Is union a bijection - 
        // Parentheses or the like might be NECESSARY for bijection. For now, fail?
        fprintf(stderr,"Warning, UNION is dangerous\n");
        generator(p.nunion.elem[0]->pr,val,scope);
      }
      break;
    case LENGTH:
      {
        ValExpr count("count", &val);
        ValExpr data("elem", &val);
        ValExpr iter(boost::str(boost::format("i%d") % num_iters++));
        ArrayElemExpr elem(&data,&iter);
        out << "for(int "<< iter<<"=0;"<<iter << "<" << count << ";" << iter << "++){";
        generator(p.length.parser->pr,elem,scope);
        out<< "}";
        out << "dep_" << mk_str(p.length.length) << "=" << count << ";";
      }
      break;
    case APPLY: 
      {
        //assert(!"Implemented");
      }
      break;
    case ARRAY:
      {
        ValExpr count("count", &val);
        ValExpr data("elem", &val);
        
        ValExpr iter(boost::str(boost::format("i%d") % num_iters++));
        ArrayElemExpr elem(&data,&iter);
        out << "for(int "<< iter<<"=0;"<<iter << "<" << count << ";" << iter << "++){";
        switch(p.array.N_type){
        case MANYONE:
          generator(p.array.manyone->pr,elem,scope);break;
        case MANY:
          generator(p.array.many->pr,elem,scope);break;
        case SEPBY:
          out << "if("<<iter<<"!= 0){";
          generator(p.array.sepby.separator);
          out << "}";
          generator(p.array.sepby.inner->pr,elem,scope);break;
        case SEPBYONE:
          out << "if("<<iter<<"!= 0){";
          generator(p.array.sepbyone.separator);
          out << "}";
          generator(p.array.sepbyone.inner->pr,elem,scope);break;

        }
        out << "}";
      }
      break;
    case FIXEDARRAY:{
      ValExpr iter(boost::str(boost::format("i%d") % num_iters++)); 
      ArrayElemExpr elem(&val,&iter);
      out << "for(int "<< iter<<"=0;"<<iter << "<" << intconstant_value(p.fixedarray.length) << ";" << iter << "++){";
      generator(p.fixedarray.inner->pr, elem,scope);
      out << "}\n";
    }
      break; 
    case OPTIONAL:
      {
        out << "if(NULL!="<<val << "){";
        DerefExpr opt(val);
        generator(p.optional->pr,opt,scope);
        out << "}";
      }
      break;
    case REF:
      //TODO: Each of these needs to deal with parameters
      out << "if(!gen_" << mk_str(p.ref.name) << "(str_current,"<< val << ")){return 0;}";
      
      break;
    case NAME:
      out << "if(!gen_"<< mk_str(p.name.name) << "(str_current,&"<< val << ")){return 0;}";
      break;
    }
  }

public:
  GenGenerator(std::ostream &_out ) : out(_out), num_iters(0){ 
  }  
  void generator( grammar *grammar)
  {
    out << "#include <hammer/hammer.h>\n";
    out << "#include <hammer/internal.h>\n";
    FOREACH(definition,*grammar){

      if(definition->N_type == CONSTANTDEF){
        std::string name = mk_str(definition->constantdef.name);
        out<<"int gen_"<<name<<"(NailStream* out);";
      }
      else if(definition->N_type==PARSER){
        std::string name = mk_str(definition->parser.name);
        out << "int gen_" << (name)<<"(NailStream *out,"<< name << " * val);";
      }          
    }
    FOREACH(definition,*grammar){

      if(definition->N_type == CONSTANTDEF){
        std::string name = mk_str(definition->constantdef.name);
        out<<"int gen_"<<name<<"(NailStream* out){\n";
        generator(definition->constantdef.definition);
        out << "return 1;"
        out << "}";
      }
      else if(definition->N_type==PARSER){
        Scope scope;
        std::string name = mk_str(definition->parser.name);
        out << "int gen_" << (name)<<"(NailStream *str_current,"<< name << " * val){";
        scope.add_stream_parameter("current");
        ValExpr outval("val",NULL,1);
        generator(definition->parser.definition.pr,outval,scope);
        out << "return 1;"
        out << "}";
      }          
    }

  }
};
void emit_generator(std::ostream *out, grammar *grammar){
  GenGenerator g(*out);
  g.generator(grammar);
  *out << std::endl;
}

