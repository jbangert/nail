/* debug pretty-printing*/
#define HM_DO_PRINT(cast,field,type,val) TOKENPASTE2(cast,_PRINT)(field,type,val)
#define HM_F(cast,type,field,parser)  HM_DO_PRINT(cast,field, type, val->field)
#define HM_PRINT_FORMAT(field,format,value) fprintf(out,"%*s:" format "\n",10+4*indent,#field,value);
#define HM_UINT_PRINT(field,type,value) HM_PRINT_FORMAT(field,"%lu",((unsigned long)value))
#define HM_SINT_PRINT(field,type,value) HM_PRINT_FORMAT(field,"%ld",((long)value))
#define HM_OBJECT_PRINT(field,type,value) if(value){ HM_PRINT_FORMAT(field,"%s","(object){");TOKENPASTE2(print_,type)(value,out,indent+1);} else {HM_PRINT_FORMAT(field,"%s","(null)");}
#define HM_ARRAY(cast,type,field,parser) do{int j;type *ptr;\
                                            for(j=0;j<val->field.count;j++){ \
                                                    HM_DO_PRINT(cast,field,type,val->field.elem[j])}}while(0);
#define HM_OPTIONAL(cast,type,field,parser,default) HM_F(cast,type,field,parser)
#define HM_STRUCT_SEQ(...)                                      \
        void TOKENPASTE2(print_,HM_NAME) (const HM_NAME *val,FILE *out,int indent) {  \
                __VA_ARGS__                                             \
      }
