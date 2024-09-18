package cs320

import Value._

object Implementation extends Template {

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

  def interp(expr: Expr, env: Env): Value = expr match{
    case Id(x) => env.getOrElse(x, error())
    case IntE(n) => IntV(n)
    case BooleanE(b) => BooleanV(b)
    case Add(l, r) => intVadd(interp(l, env), interp(r, env))
    case Mul(l, r) => intVmul(interp(l, env), interp(r, env))
    case Div(l, r) => if(interp(r,env)==IntV(0)) error() else intVdiv(interp(l, env), interp(r, env))
    case Mod(l, r) => if(interp(r,env)==IntV(0)) error() else intVmod(interp(l, env), interp(r, env))
    case Eq(l, r) => BooleanVeq(interp(l, env), interp(r, env))
    case Lt(l, r) => BooleanVlt(interp(l, env), interp(r, env))
    case If(c, tb, fb) => interp(c, env) match {
      case BooleanV(true) => interp(tb, env)
      case BooleanV(false) => interp(fb, env)
      case _ => error()
    }
    case TupleE(e) => TupleV(e.map(interp(_, env)))
    case Proj(e, i) => interp(e, env) match {
      case TupleV(v) => if(v.length < i) error() else v(i-1)
      case _ => error()
    }
    case NilE => NilV
    case ConsE(h, t) => interp(t, env) match {
      case NilV => ConsV(interp(h, env), interp(t, env))
      case ConsV(l, r) => ConsV(interp(h, env), interp(t, env))
      case _ => error()
    }
    case Empty(e) => interp(e,env) match {
      case NilV => BooleanV(true)
      case ConsV(l, r) => BooleanV(false)
      case _ => error()
    }
    case Head(e) => interp(e, env) match {
      case ConsV(h, t) => h
      case _ => error()
    }
    case Tail(e) => interp(e, env) match {
      case ConsV(h, t) => t
      case _ => error()
    }
    case Val(n, e, b) => interp(b, env+(n->interp(e, env)))
    case Fun(p, b) => CloV(p, b, env)
    case RecFuns(f, b) => {
      val recop = (fd: FunDef) => CloV(fd.parameters, fd.body, env)
      val fn = f.map(_.name)
      val avals = f.map(recop)
      val nenv = env++fn.zip(avals)
      for(fx<-avals){
        fx.env = nenv
      }
      interp(b, nenv)
    }
    case App(f, a) => interp(f, env) match {
      case CloV(x, b, fenv) => {
        if(x.length == a.length){
          val avals = a.map(interp(_, env))
          interp(b, fenv++(x.zip(avals)))
        }
        else error()
      }
      case _ => error()
    }
    case Test(e, t) => interp(e, env) match {
      case IntV(v) => if(t==IntT) BooleanV(true) else BooleanV(false)
      case BooleanV(v) => if(t==BooleanT) BooleanV(true) else BooleanV(false)
      case TupleV(v) => if(t==TupleT) BooleanV(true) else BooleanV(false)
      case NilV => if(t==ListT) BooleanV(true) else BooleanV(false)
      case ConsV(l, r) => if(t==ListT) BooleanV(true) else BooleanV(false)
      case CloV(p, b, e) => if(t==FunctionT) BooleanV(true) else BooleanV(false)
    }
  }
  def interp(expr: Expr): Value = interp(expr, Map())
}
