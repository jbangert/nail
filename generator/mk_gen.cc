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
    out << "h_bit_writer_put(out," << value << "," << width << ");\n";
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
      int width = boost::lexical_cast<int>(mk_str(p.integer.parser.unsign));
      out << "h_bit_writer_put(out,"<<val<<","<< width << ");";
      break;
    }
    case STRUCTURE:{
      std::stringstream fixup;
      fixup << "{/*Context-rewind*/\n HBitWriter end_of_struct= *out;\n";
      FOREACH(field, p.structure){
        switch(field->N_type){
        case CONSTANT:
          generator(field->constant);
          break;
        case FIELD:{
          ValExpr fieldname(mk_str(field->field.name),&val);
          generator(field->field.parser->pr,fieldname,scope);
          break;
        }
        case DEPENDENCY:{
          //Reserve space, which is not yet possible? 
          std::string post;
          parser &p = *field->dependency.parser;
          std::string name = mk_str(field->dependency.name);
          if(p.pr.N_type != INTEGER){
            assert(!"Cannot generate non-integer dependency fields yet;");
          }
          int width =  boost::lexical_cast<int>(mk_str(p.pr.integer.parser.unsign));
          out << typedef_type(p,"",&post) << " dep_"<< name << ";";
          assert(post == "");
          out << "HBitWriter rewind_"<< name << "=*out;";
          out << "h_bit_writer_put(out,0,"<<width<<");";
          fixup << "out->index = rewind_ " << name << ".index;";
          fixup << "out->bit_offset = rewind_" << name <<".bit_offset;";
          fixup << "h_bit_writer_put(out,dep_"<<name<<","<<width<<");";
          break;
        }
        case TRANSFORM:{ 
          std::string cfunction = mk_str(field->transform.cfunction);
          FOREACH(stream, field->transform.left){
              out << "NailStream str_" << mk_str(*stream) <<";\n";
              // str_current cannot appear on the left
          }
          out  << "//XXX: No transform in generator yet\n";
          //TODO: build an implementation
        }
        }
      }
      fixup <<  "out->index = end_of_struct.index;\n";
      fixup << "out->bit_offset = end_of_struct.bit_offset;\n}";
      //TODO: Do a proper way of updating these!
      out << fixup.str();
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
      out << "gen_" << mk_str(p.ref.name) << "(out,"<< val << ");";
      
      break;
    case NAME:
      out << "gen_"<< mk_str(p.name.name) << "(out,&"<< val << ");";
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
        out<<"void gen_"<<name<<"(HBitWriter* out);";
      }
      else if(definition->N_type==PARSER){
        std::string name = mk_str(definition->parser.name);
        out << "void gen_" << (name)<<"(HBitWriter *out,"<< name << " * val);";
      }          
    }
    FOREACH(definition,*grammar){

      if(definition->N_type == CONSTANTDEF){
        std::string name = mk_str(definition->constantdef.name);
        out<<"void gen_"<<name<<"(HBitWriter* out){\n";
        //        generator(definition->constantdef.definition);
        out << "}";
      }
      else if(definition->N_type==PARSER){
        std::string name = mk_str(definition->parser.name);
        out << "void gen_" << (name)<<"(HBitWriter *out,"<< name << " * val){";
        ValExpr outval("val",NULL,1);
        //        generator(definition->parser.definition.pr,outval);
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

