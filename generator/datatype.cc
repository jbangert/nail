#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <list>
class CDataModel{
  std::stringstream out;
  std::ostream &outs;
  std::list<std::string> global_enum;
  std::string int_type(const intp &intp ){
    int length = boost::lexical_cast<int>(mk_str(intp.sign));
    if(length<=8) length = 8;
    else if(length<=16) length=16;
    else if(length<=32) length=32;
    else if(length<=64) length=64;
    else throw std::range_error("Integer longer than 64 bits");
    return (boost::format("%s%d_t") % (intp.N_type == UNSIGN ? "uint":"int" )% length).str();
  }

  void emit_array(const parser &inner, const std::string name = ""){

    out << "struct "<< name <<"{\n";
    emit_type(inner);
    out << "*elem;\n size_t count;\n";
    out << "}" ;

  }
  void emit_type( const parser &outer,const std::string name =""){
    const parserinner &p = outer.pr;
    switch(p.N_type){
    case INTEGER:
      out << int_type(p.integer.parser);
      break;
    case STRUCTURE:
      out<<"struct " << name << "{\n";
      FOREACH(field,p.structure){
        if(field->N_type != FIELD)
          continue;
        emit_type(*field->field.parser);
        out << " " << mk_str(field->field.name) << ";\n";
      }
      out << "}" << std::endl;
      break;
    case WRAP:
      emit_type(*p.wrap.parser);
      break;
    case SELECTP:{
      out << "struct " << name << "{\n";
      FOREACH(option, p.selectp.options){
        global_enum.push_front(mk_str(option->tag));
      }
      out<< "enum N_types N_type;\n";
      out << "union {\n";
      FOREACH(option, p.selectp.options){
        std::string tag = mk_str(option->tag);
        boost::algorithm::to_lower(tag);
        emit_type(*option->parser);
        out << " "<< tag << ";\n";
      }
      out<< "};\n}";
      break;
    }
    case CHOICE:{
      out << "struct " << name<< "{\n";
      if(0){//TODO: global enum option 
        out<< " enum  {";
        int idx=0;
        FOREACH(option, p.choice){
          std::string tag = mk_str(option->tag);
          if(idx++ >0) 
            out << ',';
          out << tag;
        }
        out << "} N_type; \n";
      }  else{
        FOREACH(option, p.choice){
          global_enum.push_front(mk_str(option->tag));
        }
        out<< "enum N_types N_type;\n";
      }
      out << "union {\n";
      FOREACH(option, p.choice){
        std::string tag = mk_str(option->tag);
        boost::algorithm::to_lower(tag);
        emit_type(*option->parser);
        out << " "<< tag << ";\n";
      }
      out<< "};\n}";
    }
      break;
  
    case ARRAY:
      switch(p.array.N_type){
      case MANY:
      case MANYONE:
        emit_array(*p.array.many,name);
        break;
      case SEPBY:
      case SEPBYONE:
        emit_array(*p.array.sepby.inner,name);
        break;
      }
      break;
    case FIXEDARRAY:
      emit_type(*p.fixedarray.inner);
      out << "["<< intconstant_value(p.fixedarray.length) << "]";
      break;
    case OPTIONAL:
      emit_type(*p.optional);
      out << "*";
      break;
    case LENGTH:
      emit_array(*p.length.parser,name);
      break;

    case NUNION:
      {
        std::string first;
        int idx=0;
        FOREACH(elem, p.nunion){
          //  std::stringstream tmp;
          //emit_type(tmp,**elem);
          if(idx++==0)
            emit_type(**elem);
          //            first = tmp.str();
          else{
            //TODO: Perform in-depth checking
            //         if(first!= tmp.str()){
            //        throw std::runtime_error(boost::str(boost::format("UNION elements do not induce the same type\n %s \n %s") % tmp.str() % first)); //Be more
            //descriptive
            //    }
          }
        }
        out << first;
      }
      break;
    case REF:
      out << mk_str(p.ref.name)<<"* "; // TODO: Make ref more powerful, featuring ALIAS
      break;
    case NAME:
      out << mk_str(p.name.name);
      break;
    case APPLY:
      emit_type(*p.apply.inner); 
      break;
    }
  }
  void emit_type_definition( const parser &p,const std::string name)
  {
    switch(p.pr.N_type){
    case STRUCTURE:
    case ARRAY:
    case CHOICE:
    case SELECTP:
      emit_type(p,name);
      out << ";" << std::endl;
      break;
    case WRAP:
      emit_type_definition(*p.pr.wrap.parser,name);
      break;
    default:
      break; //Other definitions don't need a type
    }
  }
  void emit_type( const definition &def){
    if(def.N_type != PARSER)
      return;
    std::string name(mk_str(def.parser.name));
    emit_type_definition(def.parser.definition,name);
  }
public:
  void emit_parser( grammar *grammar){
    try{
      assert(outs.good());
      outs << "#include <stdint.h>\n #include <string.h>\n#include <assert.h>\n";
      //Emit forward declarations
      FOREACH(definition,*grammar){
        if(definition->N_type!=PARSER)
          continue;
        std::string name(mk_str(definition->parser.name));
        std::string post; 
        std::string typedef_t( typedef_type(definition->parser.definition,name,&post));
        out << "typedef "<<typedef_t<< " " << name << post <<";" << std::endl;
      }
      FOREACH(definition, *grammar){
        emit_type(*definition);
      }
      out<< std::endl;

      outs << "enum N_types {_NAIL_NULL";
      for(std::string e: global_enum){
        outs << "," << e;
      }
      outs << "};\n";
      outs << out.str()<< std::endl;
    } catch(const std::exception &e){
      std::cerr << "Exception while generating parser" << e.what()<<std::endl;
      exit(-1);
    }
  }

  CDataModel(std::ostream *os) : outs(*os){}
  
};
void emit_header(std::ostream *out, grammar *grammar){
  CDataModel data(out);
  data.emit_parser(grammar);
  *out  <<   std::string((char *)parser_template_h,parser_template_h_len);
  FOREACH(def, *grammar){
    if(def->N_type == PARSER){
      std::string name= mk_str(def->parser.name);
      if(option::cpp)
        *out << name << "*parse_" << name << "(NailArena *arena, NailStream *data);\n";
      else
        *out << name << "*parse_" << name << "(NailArena *arena, const uint8_t *data, size_t size);\n";
    }
  }

}
