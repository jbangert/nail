#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

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
    case arrayvalue::STRING:
      assert(mk_str(c.parser.unsign) == "8");
      FOREACH(ch, c.value.string){
        constint(8,(boost::format("'%c'") % ch).str());
      }
      break;
    case arrayvalue::VALUES:{
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
    case constparser::CARRAY:
      generator(p.carray); break;
    case constparser::CREPEAT:
      generator(*p.crepeat); break; //Emit just one of the elements
    case constparser::CINT: {
      int width = boost::lexical_cast<int>(mk_str(p.cint.parser.unsign));
      constint(width, intconstant_value(p.cint.value));
    }
      break;
    case constparser::CREF:
      out << "gen_"<< mk_str(p.cref)<<"(out);";break;
    case constparser::CSTRUCT:
      FOREACH(field,p.cstruct){
        generator(*field);
      }
      break;
    case constparser::CUNION:
      generator(p.cunion.elem[0]);
      break;
    }
  }
  void generator(parserinner &p,Expr &val){
    switch(p.N_type){
    case parserinner::INTEGER:{
      int width = boost::lexical_cast<int>(mk_str(p.integer.parser.unsign));
      out << "h_bit_writer_put(out,"<<val<<","<< width << ");";
      break;
    }
    case parserinner::STRUCTURE:
      FOREACH(field, p.structure){
        switch(field->N_type){
        case structparser::CONSTANT:
          generator(field->constant);
          break;
        case structparser::FIELD:{
          ValExpr fieldname(mk_str(field->field.name),&val);
          generator(field->field.parser->pr,fieldname);
          break;
        }
        case structparser::DEPENDENCY:{
          out << "//XXX: No dependencies in generator yet\n";
          break;
        }
        case structparser::TRANSFORM:{ 
          out  << "//XXX: No transform in generator yet\n";
          //TODO: build an implementation
        }
        }
      }
      //TODO: Do a proper way of updating these!
      out << "{/*Context-rewind*/\n HBitWriter end_of_struct= *out;\n";
      FOREACH(field,p.structure){
        if(field->N_type != structparser::DEPENDENCY)
          continue;
        out<< "NO dependencies in generator yet!\n";
      }
      out << "out->index = end_of_struct.index;\n";
      out << "out->bit_offset = end_of_struct.bit_offset;\n}";
      break;
    case parserinner::WRAP:
      if(p.Wrap.constbefore){
        FOREACH(c,*p.wrap.constbefore){
          generator(*c);
        }
      }
      generator(p.wrap.parser->pr,val);
      if(p.wrap.constafter){
        FOREACH(c,*p.wrap.constafter){
          generator(*c);
        }
      }      
      break;

    case parserinner::CHOICE:
      {
        out << "switch("<< ValExpr("N_type",&val)<<"){\n";
        FOREACH(c, p.choice){
          std::string tag = mk_str(c->tag);
          out << "case " << tag << ":\n";
          ValExpr expr(tag,&val);
          generator(c->parser->pr, expr );
          out << "break;\n";
        }
        out << "}";
      }      
      break;
    case parserinner::NUNION:
      {
        //What to do with UNION? Is union a bijection - 
        // Parentheses or the like might be NECESSARY for bijection. For now, fail?
        fprintf(stderr,"Warning, UNION is dangerous\n");
        generator(p.nunion.elem[0]->PR,val);
      }
      break;
    case parserinner::LENGTH:
      {
        ValExpr count("count", &val);
        ValExpr data("elem", &val);
        ValExpr iter(boost::str(boost::format("i%d") % num_iters++));
        ArrayElemExpr elem(&data,&iter);
        out << "for(int "<< iter<<"=0;"<<iter << "<" << count << ";" << iter << "++){";
        generator(p.length.parser->pr,elem);
        out<< "}";
        out << mk_str(p.LENGTH.length) << "=" << count << ";";
      }
      break;
    case APPLY: 
      {
        assert(!"Implemented");
      }
      break;
    case ARRAY:
      {
        ValExpr count("count", &val);
        ValExpr data("elem", &val);
        
        ValExpr iter(boost::str(boost::format("i%d") % num_iters++));
        ArrayElemExpr elem(&data,&iter);
        out << "for(int "<< iter<<"=0;"<<iter << "<" << count << ";" << iter << "++){";
        switch(p.ARRAY.N_type){
        case MANYONE:
          generator(p.ARRAY.MANYONE->pr,elem);break;
        case MANY:
          generator(p.ARRAY.MANY->pr,elem);break;
        case SEPBY:
          out << "if("<<iter<<"!= 0){";
          generator(p.ARRAY.SEPBY.separator);
          out << "}";
          generator(p.ARRAY.SEPBY.inner->pr,elem);break;
        case SEPBYONE:
          out << "if("<<iter<<"!= 0){";
          generator(p.ARRAY.SEPBYONE.separator);
          out << "}";
          generator(p.ARRAY.SEPBYONE.inner->pr,elem);break;

        }
        out << "}";
      }
      break;
    case FIXEDARRAY:{
      ValExpr iter(boost::str(boost::format("i%d") % num_iters++)); 
      ArrayElemExpr elem(&val,&iter);
      out << "for(int "<< iter<<"=0;"<<iter << "<" << intconstant_value(p.FIXEDARRAY.length) << ";" << iter << "++){";
      generator(p.FIXEDARRAY.inner->pr, elem);
      out << "}\n";
    }
      break; 
    case OPTIONAL:
      {
        out << "if(NULL!="<<val << "){";
        DerefExpr opt(val);
        generator(p.OPTIONAL->pr,opt);
        out << "}";
      }
      break;
    case REF:
      //TODO: Each of these needs to deal with parameters
      out << "gen_" << mk_str(p.REF.name) << "(out,"<< val << ");";
      
      break;
    case NAME:
      out << "gen_"<< mk_str(p.NAME.name) << "(out,&"<< val << ");";
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

      if(definition->N_type == CONST){
        std::string name = mk_str(definition->CONST.name);
        out<<"void gen_"<<name<<"(HBitWriter* out);";
      }
      else if(definition->N_type==PARSER){
        std::string name = mk_str(definition->PARSER.name);
        out << "void gen_" << (name)<<"(HBitWriter *out,"<< name << " * val);";
      }          
    }
    FOREACH(definition,*grammar){

      if(definition->N_type == CONST){
        std::string name = mk_str(definition->CONST.name);
        out<<"void gen_"<<name<<"(HBitWriter* out){\n";
        generator(definition->CONST.definition);
        out << "}";
      }
      else if(definition->N_type==PARSER){
        std::string name = mk_str(definition->PARSER.name);
        out << "void gen_" << (name)<<"(HBitWriter *out,"<< name << " * val){";
        ValExpr outval("val",NULL,1);
        generator(definition->PARSER.definition.PR,outval);
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

