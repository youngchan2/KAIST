package cs320

import Value._

object Implementation extends Template {
  sealed trait Handler
  case object NilHandler extends Handler
  case class ExceptionHandler(expr: Expr, env: Env, k: Cont, handler: Handler) extends Handler

  def intVop(op: (BigInt, BigInt)=>BigInt):(Value, Value) => IntV = {
    case (IntV(x), IntV(y)) => IntV(op(x, y))
    case (x, y) => error()
  }
  val intVadd = intVop(_+_)
  val intVmul = intVop(_*_)
  val intVdiv = intVop(_/_)
  val intVmod = intVop(_%_)
  
  def binaryop(op: (BigInt, BigInt)=>Boolean):(Value, Value) => BooleanV = {
    case (IntV(x), IntV(y)) => BooleanV(op(x, y))
    case (_, _) => error()
  }
  val BooleanVeq = binaryop(_==_)
  val BooleanVlt = binaryop(_<_)

  def tupleop(op:(Expr, Env, Handler, Cont) => Value, env: Env, hd: Handler, e: List[Expr], list : List[Value], k: Cont): Value = e match{
      case Nil => k(TupleV(list.reverse))
      case h::t => op(h, env, hd, hv => tupleop(op, env, hd, t, hv :: list, k))
  }

  def tupletolist(v: Value): List[Value] = v match{
    case TupleV(v) => v
    case _ => error()
  }

  def interp(expr: Expr, env: Env, h: Handler, k: Cont): Value = expr match{
    case Id(x) => k(env.getOrElse(x, error()))
    case IntE(n) => k(IntV(n))
    case BooleanE(b) => k(BooleanV(b))
    case Add(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => k(intVadd(lv, rv))))
    case Mul(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => k(intVmul(lv, rv))))
    case Div(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => if(rv == IntV(0)) error() else k(intVdiv(lv, rv))))
    case Mod(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => if(rv == IntV(0)) error() else k(intVmod(lv, rv))))
    case Eq(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => k(BooleanVeq(lv, rv))))
    case Lt(l, r) => interp(l, env, h, lv => interp(r, env, h, rv => k(BooleanVlt(lv, rv))))
    case If(c, tb, fb) => interp(c, env, h, cv => 
      cv match{
        case BooleanV(true) => interp(tb, env, h, k)
        case BooleanV(false) => interp(fb, env, h, k)
        case _ => error()
      })
    case TupleE(e) => tupleop(interp, env, h, e, Nil, k)
    case Proj(e, i) => interp(e, env, h, ev => ev match{
      case TupleV(v) => if(v.length < i) error() else k(v(i-1))
      case _ => error()
    })
    case NilE => k(NilV)
    case ConsE(head, t) => interp(head, env, h, hv =>
      interp(t, env, h, tv => tv match{
        case NilV => k(ConsV(hv, tv))
        case ConsV(l, r) => k(ConsV(hv, tv))
        case _ => error()
      }))
    case Empty(e) => interp(e,env, h, ev => ev match{
      case NilV => k(BooleanV(true))
      case ConsV(l, r) => k(BooleanV(false))
      case _ => error()
    }) 
    case Head(e) => interp(e, env, h, ev => ev match{
      case ConsV(h, t) => k(h)
      case _ => error()
    })
    case Tail(e) => interp(e, env, h, ev => ev match{
      case ConsV(h, t) => k(t)
      case _ => error()
    }) 
    case Val(x, e, b) => interp(e, env, h, ev => interp(b, env+(x->ev), h, k))
    case Vcc(x, e) => interp(e, env+(x->ContV(k)), h, k)
    case Fun(p, b) => k(CloV(p, b, env))
    case RecFuns(f, b) => {
      val recop = (fd: FunDef) => CloV(fd.parameters, fd.body, env)
      val fn = f.map(_.name)
      val avals = f.map(recop)
      val nenv = env++fn.zip(avals)
      for(fx<-avals){
        fx.env = nenv
      }
      interp(b, nenv, h, k)
    }
    case App(f, a) => interp(f, env, h, fv => 
      interp(TupleE(a), env, h, av => fv match{
        case CloV(x, b, fenv) =>
          if(x.length == a.length)
            interp(b, fenv++(x.zip(tupletolist(av))), h, k)
          else
            error()
        case ContV(ka) => 
          if(a.length == 1){
            ka(tupletolist(av)(0))
          }
          else error()
        case _ => error()
      }))
    case Test(e, t) => interp(e, env, h, ev => ev match{
      case IntV(v) => if(t==IntT) k(BooleanV(true)) else k(BooleanV(false))
      case BooleanV(v) => if(t==BooleanT) k(BooleanV(true)) else k(BooleanV(false))
      case TupleV(v) => if(t==TupleT) k(BooleanV(true)) else k(BooleanV(false))
      case NilV => if(t==ListT) k(BooleanV(true)) else k(BooleanV(false))
      case ConsV(l, r) => if(t==ListT) k(BooleanV(true)) else k(BooleanV(false))
      case CloV(p, b, e) => if(t==FunctionT) k(BooleanV(true)) else k(BooleanV(false))
      case ContV(ka) => if(t==FunctionT) k(BooleanV(true)) else k(BooleanV(false))
    })
    case Throw(e) => interp(e, env, h, ev => h match{
      case ExceptionHandler(eh, envh, kh, hh) =>
        interp(eh, envh, hh, ehv => ehv match{
          case CloV(x, b, fenv) =>
            if(x.length == 1)
              interp(b, fenv+(x(0)->ev), hh, kh)
            else
              error()
          case ContV(ka) => ka(ev)
          case _ => error()
        })
      case _ => error()
    })
    case Try(e1, e2) => {
      val nh = ExceptionHandler(e2, env, k, h)
      interp(e1, env, nh, k)
    }
  }
  def interp(expr: Expr): Value = interp(expr, Map(), NilHandler, x=>x)
}