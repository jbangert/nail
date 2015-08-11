#include "nailtool.h"
#include <list>
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
  std::ostream &os, &hdr;
  std::stringstream out, header;
  int num_iters;
  void constint(int width, std::string value){
    out << "if(parser_fail(NailOutStream_write(str_current," << value << "," << width << "))) return -1;";
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
      out << "gen_"<< mk_str(p.cref)<<"(str_current);";break;
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
        out << "){return -1;}";
      }
      out << "if(parser_fail(NailOutStream_write(str_current,"<<val<<","<< width << "))) return -1;";
      break;
    }
    case STRUCTURE:{
      std::stringstream fixup;
      std::list<std::string> transform_invocations;
      Scope newscope(scope);
      fixup << "{/*Context-rewind*/\n NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);\n";
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
          std::string type = typedef_type(p,"",&post);
          if(p.pr.N_type != INTEGER){
            assert(!"Cannot generate non-integer dependency fields yet;");
          }
          int width =  boost::lexical_cast<int>(mk_str(p.pr.integer.parser.unsign));
          out << type  << " dep_"<< name << ";";
          newscope.add_dependency_definition(name,type, width);
          assert(post == "");
          out << "NailOutStreamPos rewind_"<< name << "=NailOutStream_getpos(str_current);";
          out << "NailOutStream_write(str_current,0,"<<width<<");";
          fixup << "NailOutStream_reposition(str_current, rewind_"<<name<<");";
          fixup << "NailOutStream_write(str_current,dep_"<<name<<","<<width<<");";
          break;
        }
        case TRANSFORM:{ 
          std::string cfunction = mk_str(field->transform.cfunction);
          std::stringstream declaration;
          FOREACH(stream, field->transform.left){
            out << "NailOutStream str_" << mk_str(*stream) <<";\n";
            out << "if(parser_fail(NailOutStream_init(&str_"<<mk_str(*stream)<<",4096))) {return -1;}\n";
            fixup << "NailOutStream_release(&str_"<<mk_str(*stream)<<");"; 
          }
          //Transforms need to be processed in reverse order
          std::stringstream invocation;
          invocation << "if(parser_fail("<< cfunction << "_generate(tmp_arena";
          declaration << "extern  int " << cfunction << "_generate(NailArena *tmp_arena";
          FOREACH(stream, field->transform.left){           
            declaration  << ",NailOutStream *str_" << mk_str(*stream);
            invocation << ", &str_"  << mk_str(*stream);
          }      
          FOREACH(param, field->transform.right){
            switch(param->N_type){
            case PDEPENDENCY:{
              std::string name (mk_str(param->pdependency));
              invocation << "," << newscope.dependency_ptr(name); 
              declaration << "," << newscope.dependency_type(name) << "* " << name;
            }
              break;
            case PSTREAM:
              invocation << "," <<  newscope.stream_ptr(mk_str(param->pstream));  
              declaration << ",NailOutStream *str_" << mk_str(param->pstream);
              break;
            }
          }
          declaration << ");";
          if(!option::templates)
            header << declaration.str();
          invocation << "))){return -1;}";
          FOREACH(stream, field->transform.left){
            newscope.add_stream_definition(mk_str(*stream));
          }
          invocation << "{ NailOutStreamPos t_rewind_eot =NailOutStream_getpos(str_current);\n";

          FOREACH(param, field->transform.right){
            if(param->N_type != PDEPENDENCY) continue;
            std::string name (mk_str(param->pdependency));
            if(!newscope.is_local_dependency(name)) continue;
            invocation << "NailOutStream_reposition(str_current, rewind_"<<name<<");";
            invocation << "NailOutStream_write(str_current,dep_"<<name<<","<<newscope.dependency_width(name)<<");";
          }
          invocation << " NailOutStream_reposition(str_current, t_rewind_eot);\n }";
          transform_invocations.push_front(invocation.str());
        }
          break;
        }
      }
      fixup <<  "NailOutStream_reposition(str_current, end_of_struct);";
      for(auto &transform: transform_invocations){
        out << transform;
      }
      //TODO:  We need to properly interleave APPLY and TRANSFORM when generating! Transform at the
      //end is wrong in some cases (when the applied parser relies on a context from the transform)
      out << fixup.str();
      out << "}";
    }
      break;
    case APPLY:{              
      out << "{/*APPLY*/";
      out << "NailOutStream  * orig_str = str_current;\n";
      std::string name = mk_str(p.apply.stream);
      out << "str_current =" << scope.stream_ptr(name) << ";";
      generator(p.apply.inner->pr,val,scope);
      out << "str_current = orig_str;";
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
    case SELECTP:{
      
        out << "switch("<< ValExpr("N_type",&val)<<"){\n";
        FOREACH(c, p.selectp.options){
          std::string tag = mk_str(c->tag);
          std::string enum_tag = tag;
          boost::algorithm::to_lower(tag);
          out << "case " << enum_tag << ":{\n";
          ValExpr expr(tag,&val);
          out <<"*" <<scope.dependency_ptr(mk_str(p.selectp.dep)) << "=" << intconstant_value(c->value) << ";";
          generator(c->parser->pr, expr, scope );
          out << "break;}\n";
        }
        out << "}";
        break;
    }
      
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
    case REF: // Fallthrough intentional, and kludgy
    case NAME:
      {
        std::string parameters = parameter_invocation(p.name.parameters,scope);
        out << "if(parser_fail(gen_"<< mk_str(p.name.name) << "(tmp_arena,str_current,&"<< val << parameters<<"))){return -1;}";
      break;
      }
    }
  }

public:
  GenGenerator(std::ostream &_out, std::ostream &_hdr ) : os(_out), hdr(_hdr), num_iters(0){ 
  }  
  void generator( grammar *grammar)
  {
    FOREACH(definition,*grammar){

      if(definition->N_type == CONSTANTDEF){
        std::string name = mk_str(definition->constantdef.name);
        header<<"int gen_"<<name<<"(NailOutStream* str_current);";
        out<<"int gen_"<<name<<"(NailOutStream* str_current){\n";
        generator(definition->constantdef.definition);
        out << "return 0;";
        out << "}";
      }
      else if(definition->N_type==PARSER){
        Scope scope;
        std::string name = mk_str(definition->parser.name);
        std::string params =  parameter_definition(*definition,scope);    
        header << "int gen_" << (name)<<"(NailArena *tmp_arena,NailOutStream *out,"<< name << " * val"<<params<<");" << std::endl;
        out << "int gen_" << (name)<<"(NailArena *tmp_arena,NailOutStream *str_current,"<< name << " * val"<<params<<"){";
        scope.add_stream_parameter("current");
        ValExpr outval("val",NULL,1);
        generator(definition->parser.definition.pr,outval,scope);
        out << "return 0;";
        out << "}";
      }          
    }
    hdr<< header.str() << std::endl;
    os << header.str() << out.str() << std::endl;
  }
};
void emit_generator(std::ostream *out, std::ostream *header, grammar *grammar){
  GenGenerator g(*out,*header);
  g.generator(grammar);
  *out << std::endl;
}

