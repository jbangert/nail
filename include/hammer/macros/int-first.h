#define TOKENPASTE(x, y) x ## y  //http://stackoverflow.com/questions/1597007
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#define HM_UINT_TYPE(x) x
#define HM_SINT_TYPE(x) x
#define HM_OBJECT_TYPE(x) x*
#define HM_F_OBJECT(type,field)   HM_F(HM_OBJECT,type,field,type)
#define HM_F_OBJECT_OPT(type,field) HM_OPTIONAL(HM_OBJECT,type, field,type,NULL)

#define HM_UINT_CAST(type,val) H_CAST_UINT(val)
#define HM_SINT_CAST(type,val) H_CAST_SINT(val)
#define HM_OBJECT_CAST(type,val) H_CAST(type,val)

#define HM_DO_CAST(cast,type,val) TOKENPASTE2(cast,_CAST)(type,val)
#define HM_DO_TYPE(cast,val) TOKENPASTE2(cast,_TYPE)(val)

//FIXME: broken
#define HM_FOREACH(type, name, container) size_t name ## _iter; type *name; for(;(name ##_iter < (container).count) && (name = (container).elem[0]); name = (container).elem[name##_iter],name ## _iter++)
