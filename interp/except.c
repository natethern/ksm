#include "ksm.h"

#define MAKE_EXCEPT(kind,args) \
  ksm_itype(KSM_FLAG_EXCEPT,(int)kind,(int)args,0)
#define IS_EXCEPT(x) (ksm_flag(x)==KSM_FLAG_EXCEPT)
#define EXCEPT_KIND(x) ((ksm_t)ksm_ival(x,0))
#define EXCEPT_ARGS(x) ((ksm_t)ksm_ival(x,1))
#define EXCEPT_SET_KIND(x,kind) ksm_set_ival(x,0,(int)kind)
#define EXCEPT_SET_ARGS(x,args) ksm_set_ival(x,1,(int)args)
#define EXCEPT_KIND_NO_LOCK(x) ((ksm_t)ksm_ival_no_lock(x,0))
#define EXCEPT_ARGS_NO_LOCK(x) ((ksm_t)ksm_ival_no_lock(x,1))

static
ksm_t mark_except(ksm_t x)
{
  ksm_mark_object(EXCEPT_KIND_NO_LOCK(x));
  ksm_mark_object(EXCEPT_ARGS_NO_LOCK(x));
  return 0;
}

int ksm_is_except(ksm_t x)
{
  return IS_EXCEPT(x);
}

ksm_t ksm_except;

ksm_t ksm_with_handler(ksm_t (*handler)(ksm_t ex, ksm_t args, void *aux),
		       ksm_t fun, void *aux)
{
  ksm_t ex, val;

  ex = MAKE_EXCEPT(ksm_unspec_object, ksm_nil_object);
  ksm_push_jump(ex);
  val = ksm_funcall(fun, ksm_nil_object, ksm_nil_object);
  if( val == ksm_except ){
    ksm_rewind_jumps(ex);
    return (*handler)(EXCEPT_KIND(ex), EXCEPT_ARGS(ex), aux);
  }
  else{
    ksm_pop_jump();
    return val;
  }
}

ksm_t ksm_raise_except(ksm_t kind, ksm_t args)
{
  ksm_t ex;

  ex = ksm_search_jumps(ksm_is_except);
  if( ex == ksm_nil_object )
    ksm_error("can't find exception handler");
  EXCEPT_SET_KIND(ex, kind);
  EXCEPT_SET_ARGS(ex, args);
  return ksm_except;
}

static
ksm_t except_handler(ksm_t ex, ksm_t args, void *aux)
{
  ksm_t handler = aux;
  
  return ksm_funcall(handler, ksm_cons(ex, args), ksm_nil_object);
}

KSM_DEF_BLT(call_with_handler)
     // (call-with-handler handler fun)
     // handler: (lambda (except . args) ...)
     // fun: (lambda () ...)
{
  ksm_t handler, fun;

  KSM_SET_ARGS2(handler, fun);
  return ksm_with_handler(except_handler, fun, handler);
}

KSM_DEF_BLT(exception)
     // (exception except . args)
{
  ksm_t kind;

  KSM_POP_ARG(kind);
  return ksm_raise_except(kind, KSM_ARGS());
}

void ksm_init_except(void)
{
  ksm_install_marker(KSM_FLAG_EXCEPT, mark_except);
  ksm_except = ksm_make_imm("#<except>");
  KSM_ENTER_BLT("call-with-handler", call_with_handler);
  KSM_ENTER_BLT("exception", exception);
}
