package cs320

object Implementation extends Template {

  def typeCheck(e: Typed.Expr): Typed.Type = T.typeCheck(e, T.TypeEnv(Map(), Map(), Nil))

  def interp(e: Untyped.Expr): Untyped.Value = U.interp(e, Map(), Map())._1

  object T {
    import Typed._

    case class TypeScheme(params: List[Type], body: Type)

    case class TypeEnv(
      tscheme: Map[String, (TypeScheme, Boolean)],
      tbinds: Map[String, TypeDef],
      tvars: List[String]
    ) {
      def addTScheme(name: String, params: List[Type], body: Type, tag: Boolean): TypeEnv = {
        val tsch = TypeScheme(params, body)
        copy(tscheme = tscheme + (name->(tsch, tag)))
      }

      def addTBind(name: String, tdef: TypeDef): TypeEnv = 
        copy(tbinds = tbinds + (name->tdef))
    
      def addTVar(a: String): TypeEnv = {
        if(tvars.contains(a)) copy()
        else copy(tvars = a::tvars)
      }

      def addTyEnv(ty: TypeEnv): TypeEnv = 
        TypeEnv(tscheme ++ ty.tscheme, tbinds ++ ty.tbinds, tvars ++ ty.tvars)
    }

    def check(ts: List[Type], tenv: TypeEnv): Unit = ts match {
        case Nil => 
        case h::t => {
          validType(h, tenv)
          check(t, tenv)
        }
      }

    def validType(ty: Type, tenv: TypeEnv): Type = ty match {
      case AppT(tname, targs) => {
        check(targs, tenv)
        val tdef@TypeDef(x, tparams, variants) = tenv.tbinds.getOrElse(tname,error(s"$ty is a free type"))
        if(targs.length == tparams.length) ty
        else error(s"argument numbers doesn't match")
      }
      case VarT(vname) => {
        if(tenv.tvars.contains(vname)) ty
        else error(s"$vname is a free type")
      }
      case IntT => ty
      case BooleanT => ty
      case UnitT => ty
      case ArrowT(ptypes, rtype) => {
        check(ptypes, tenv)
        validType(rtype, tenv)
        ty
      }
    }

    def mustSame(l: Type, r: Type): Type = {
      if(same(l,r)) l
      else error(s"$l is not equal to $r")
    }

    def same(l: Type, r: Type): Boolean = {
      def isSame(list: List[Boolean]): Boolean = list match {
        case Nil => true
        case h::t => {
          if(h == true) isSame(t)
          else false
        }
      }

      (l, r) match {
        case (IntT, IntT) => true
        case (BooleanT, BooleanT) => true
        case (UnitT, UnitT) => true
        case (AppT(x1, targs1), AppT(x2, targs2)) => {
          if(x1 == x2) {
            if(targs1.length == targs2.length) {
              val cmptable = targs1 zip targs2
              val cmp = cmptable.map({ case (l,r) => same(l,r) })
              isSame(cmp)
            }
            else false
          }
          else false
        }
        case (VarT(x1), VarT(x2)) => {
          if(x1 == x2) true 
          else false
        }
        case (ArrowT(pts1, rts1), ArrowT(pts2, rts2)) => {
          if(pts1.length == pts2.length){
            val cmptable = pts1 zip pts2
            val cmp = cmptable.map({ case (l,r) => same(l,r) })

            isSame(cmp) && same(rts1, rts2)
          }
          else false
        }
        case _ => false
      } 
    }

    def validRecDef(rd: RecDef, tenv: TypeEnv): RecDef = rd match {
      case Lazy(x, typ, e) => {
        validType(typ, tenv)
        mustSame(typeCheck(e, tenv), typ)
        rd
      }
      case RecFun(x, tparams, params, rty, bd) => {
        val tps = tparams.map({ y => 
          if(tenv.tvars.contains(y)) error(s"type variable should not be in TypeEnv")
          else y
          })
        def getTyEnv(typs: List[String], curtenv: TypeEnv): TypeEnv = typs match {
          case Nil => curtenv
          case h::tail => getTyEnv(tail, curtenv.addTVar(h))
        }
        val t0env = getTyEnv(tps, tenv)
        val ptys = params.map({ case (x,y) => (x,validType(y, t0env)) })
        validType(rty, t0env)

        def mapTenv(tys: List[(String, Type)], curtenv: TypeEnv): TypeEnv = tys match {
          case Nil => curtenv
          case (xi, ti)::tail => mapTenv(tail, curtenv.addTScheme(xi, Nil, ti, false))
        }
        
        val tMenv = mapTenv(ptys, t0env)
        mustSame(rty, typeCheck(bd, tMenv))
        rd
      }
      case TypeDef(x, tparams, vs) => {
        val tps = tparams.map({ y => 
          if(tenv.tvars.contains(y)) error(s"type variable should not be in TypeEnv")
          else y
          })
        def getTyEnv(typs: List[String], curtenv: TypeEnv): TypeEnv = typs match {
          case Nil => curtenv
          case h::tail => getTyEnv(tail, curtenv.addTVar(h))
        }
        val nenv = getTyEnv(tps, tenv)

        vs.map({ case Variant(name, ps) => 
          Variant(name, ps.map({ p => validType(p, nenv)}))
        }) 
        rd
      }
    }

    def TypeEnvbyRecDef(rd: RecDef): TypeEnv = rd match {
      case Lazy(x, typ, e) => {
        val tenv = TypeEnv(Map(), Map(), Nil)
        tenv.addTScheme(x, Nil, typ, false)
      }
      case RecFun(x, tparams, params, rty, bd) => {
        val tenv = TypeEnv(Map(), Map(), Nil)
        val ptys = params.map({ case (x,y) => y })
        val tvars = tparams.map({ x => VarT(x) })
        tenv.addTScheme(x, tvars, ArrowT(ptys,rty), false)
      }
      case TypeDef(x, tparams, vs) => {
        val tenv = TypeEnv(Map(), Map(), Nil)
        val t0env = tenv.addTBind(x, TypeDef(x, tparams, vs))
        val tvars = tparams.map({ x => VarT(x) })
        def getTyEnvs(vs: List[Variant], curtenv: TypeEnv): TypeEnv = vs match {
          case Nil => curtenv
          case Variant(w, ps)::tail => ps match {
            case Nil => 
              getTyEnvs(tail, curtenv.addTScheme(w, tvars, AppT(x,tvars), false))
            case h::t => 
              getTyEnvs(tail, curtenv.addTScheme(w, tvars, ArrowT(ps, AppT(x,tvars)), false))
          }
        }
        getTyEnvs(vs, t0env)
      }
    }

    def substitute(ty: Type, tvars: List[Type], tsub: List[Type]): Type = {
      ty match {
        case AppT(tname, targs) => {
          val newtargs = targs.map(substitute(_, tvars, tsub))
          AppT(tname, newtargs)
        }
        case ArrowT(pts, rts) => {
          val newpts = pts.map(substitute(_, tvars, tsub))
          val newrts = substitute(rts, tvars, tsub)
          ArrowT(newpts, newrts)
        }
        case VarT(x) => {
          val subtable = tvars zip tsub
          def find(tofind: String, subt: List[(Type, Type)]): Type = subt match {
            case Nil => VarT(tofind)
            case (vart, typ)::tail => {
              vart match {
                case VarT(name) => {
                  if(tofind == name) typ
                  else find(tofind, tail)
                }
                case _ => error(s"$vart should be VarT")
              }
            }
          }
          find(x, subtable)
        }
        case _ => ty
      }    
    }     

    def typeCheckCase(c: Case, ws: List[Variant], vs: List[String], tys: List[Type], tenv: TypeEnv): Type = {
      val w = c.variant
      val xs = c.names
      val bd = c.body
      if(vs.length == tys.length) {
        def findvariant(ws: List[Variant]): List[Type] = ws match {
          case Nil => error(s"variants should contain variant name of $w")
          case h::tail => {
            if(h.name == w) h.params
            else findvariant(tail)
          }
        }
        val wparams = findvariant(ws)

        if(xs.length == wparams.length){
          val vtys = vs.map(VarT(_))
          val subparams = wparams.map(substitute(_,vtys,tys))
          val howmap = xs zip subparams
          def mapping(maps: List[(String, Type)], curtenv: TypeEnv): TypeEnv = maps match {
            case Nil => curtenv
            case (x, t)::tail => mapping(tail, curtenv.addTScheme(x, Nil, t, false))
          } 

          val tHenv = mapping(howmap, tenv)
          typeCheck(bd, tHenv)
        }

        else error(s"argument numbers doesn't match") 
      }

      else error(s"numbers of type variables and types must be same")
    }

    def typeCheck(expr: Expr, tenv: TypeEnv): Type = expr match {

      case Id(x, targs) => {
        check(targs, tenv)
        if(tenv.tscheme.contains(x)) {
          val tsch@(TypeScheme(ps, bd), tg) = tenv.tscheme.apply(x)
          if(targs.length == ps.length) {
            substitute(bd, ps, targs) 
          }
          else error(s"argument numbers doesn't match")
        }
        else error(s"$x is a free type")
      }
      case IntE(n) => IntT
      case BooleanE(b) => BooleanT
      case UnitE => UnitT
      case Add(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        IntT
      }
      case Mul(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        IntT
      }
      case Div(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        IntT
      }
      case Mod(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        IntT
      }
      case Eq(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        BooleanT
      }
      case Lt(l, r) => {
        mustSame(typeCheck(l, tenv), IntT)
        mustSame(typeCheck(r, tenv), IntT)
        BooleanT
      }
      case Sequence(l, r) => {
        typeCheck(l, tenv)
        typeCheck(r, tenv)
      }
      case If(c, t, f) => {
        if(same(typeCheck(c, tenv), BooleanT)) {
          mustSame((typeCheck(t, tenv)), typeCheck(f, tenv))
        }
        else error(s"Type $c is not Boolean")
      }
      case Val(mut, x, typ, e, bd) => {
        val t1 = typeCheck(e, tenv)
        typ match {
          case Some(ty1) => {
            validType(ty1, tenv)
            if(same(ty1,t1)){
              val nenv = tenv.addTScheme(x, Nil, t1, mut)
              typeCheck(bd, nenv)
            }
            else error(s"$ty1 and $t1 doesn't match")
          }
          case None => {
            val nenv = tenv.addTScheme(x, Nil, t1, mut)
            typeCheck(bd, nenv)
          }
        }
      }
      case RecBinds(defs, bd) => {
        val t0env = tenv
        def getTyEnv(dfs: List[RecDef], curtenv: TypeEnv): TypeEnv = dfs match {
          case Nil => curtenv
          case h::tail => {
            h match {
              case TypeDef(name, tpars, vats) => {
                if(t0env.tbinds.contains(name)) error(s"$name should not be in TypeEnv")
              }
              case _ => 
            }
            val nenv = curtenv.addTyEnv(TypeEnvbyRecDef(h))
            getTyEnv(tail, nenv)
          }
        }
        val tNenv = getTyEnv(defs, t0env)
        defs.map({ d => validRecDef(d, tNenv) })
        val retty = typeCheck(bd, tNenv)
        validType(retty, t0env)
      }
      case Fun(params, bd) => {
        val t0env = tenv
        val tys = params.map({ case (x,y) => y })
        tys.map({ t => validType(t, t0env)})
        
        def getTyEnv(ps: List[(String, Type)], curtenv: TypeEnv): TypeEnv = ps match {
          case Nil => curtenv
          case (x,t)::tail => getTyEnv(tail, curtenv.addTScheme(x, Nil, t, false))
        }
        
        val tNenv = getTyEnv(params, t0env)
        val rts = typeCheck(bd, tNenv)
        ArrowT(tys, rts)
      }
      case Assign(x, e) => {
        if(tenv.tscheme.contains(x)) {
          val xtsch@(TypeScheme(ps, bd), tg) = tenv.tscheme.apply(x)
          if(ps.length == 0) {  
            if(tg) {
              mustSame(typeCheck(e, tenv), bd)
              UnitT
            }
            else error(s"tag $tg must be Var")
          }
          else error(s"There must be no type parameter")
        }
        else error(s"$x is not in the TypeEnv")
      }
      case App(f, as) => {
        val fty = typeCheck(f, tenv)
        fty match {
          case ArrowT(pts, rts) => {
            if(as.length == pts.length) {
              val fapp = pts zip as
              fapp.map({ case(p, a) => mustSame(p, typeCheck(a, tenv)) })
              rts
            }
            else error(s"parameters and arguments doesn't match")
          }
          case _ => error(s"$fty must be a function type")
        }
      }

      case Match(e, cs) => {
        val ty = typeCheck(e, tenv)
        ty match {
          case AppT(x, targs) => {
            val tdef@TypeDef(names, tparams, ws) = tenv.tbinds.getOrElse(x, error(s"can't find $x in TypeEnv"))
            if(targs.length == tparams.length) {
              if(cs.length == ws.length) {
                val casetys = cs.map(typeCheckCase(_, ws, tparams, targs, tenv))
                val ty1 = casetys.head
                casetys.map(mustSame(_, ty1))
                ty1
              }
              else error(s"cases and variants don't match")
            }
            else error(s"arguments and parameters don't match")
          }
          case _ => error(s"$ty must be a type application")
        }
      }
    }
  }

  object U {
    import Untyped._

    type Sto = Map[Addr, Value]

    def addVop(op: (BigInt,BigInt) => BigInt): (Value, Value) => IntV = (_, _) match {
      case(IntV(x), IntV(y)) => IntV(op(x, y))
      case(x, y) => error(s"not both numbers: $x, $y")
    }

    val intVAdd = addVop(_ + _)
    val intVMul = addVop(_ * _)
    
    def divVop(op: (BigInt,BigInt) => BigInt): (Value, Value) => IntV = (_, _) match {
      case(IntV(x), IntV(y)) => {
        if(y == 0) error(s"can't divide by 0")
        else IntV(op(x, y))
      }
      case(x, y) => error(s"not both numbers: $x, $y")
    } 

    val intVDiv = divVop(_ / _)
    val intVMod = divVop(_ % _)

    def cmpVop(op: (BigInt,BigInt) => Boolean): (Value, Value) => BooleanV = (_, _) match {
      case(IntV(x), IntV(y)) => BooleanV(op(x, y))
      case(x, y) => error(s"not both numbers: $x, $y")
    }

    val intVEq = cmpVop(_ == _)
    val intVLt = cmpVop(_ < _)

    def maxAddr(sto: Sto): Addr =
      sto.keySet.+(0).max

    def malloc(sto: Sto): Addr =
      maxAddr(sto) + 1

    def EnvbyRecDef(rd: RecDef, sto: Sto): (Env, Sto) = {
      rd match {
        case Lazy(x, e) => {
          val addr = malloc(sto)
          (Map(x->addr), sto+(addr->UnitV))
        }
        case RecFun(x, ps, bd) => {
          val addr = malloc(sto)
          (Map(x->addr), sto+(addr->UnitV))
        }
        case TypeDef(ws) => {
          def getenv(ws: List[Variant], env: Env, cursto: Sto): (Env, Sto) = ws match {
            case Nil => (env, cursto)
            case Variant(x, empty)::t => {
                val addr = malloc(cursto)
                getenv(t, env+(x->addr), cursto+(addr->UnitV))
            }
          }
          getenv(ws, Map(), sto)
        }
      }
    } 

    def StobyRecDef(rd: RecDef, env: Env): Sto = rd match {
      case Lazy(x, e) => {
        val addr = env(x)
        Map(addr -> ExprV(e, env))
      }
      case RecFun(x, ps, bd) => {
        val addr = env(x)
        Map(addr -> CloV(ps, bd, env))
      }
      case TypeDef(ws) => {
        def getsto(ws: List[Variant], sto: Sto): Sto = ws match {
          case Nil => sto
          case Variant(x, empty)::t => {
            if(empty) {
              getsto(t, sto+(env(x)->VariantV(x, Nil)))
            }
            else {
              getsto(t, sto+(env(x)->ConstructorV(x)))
            }
          }
        }
        getsto(ws, Map())
      }
    }

    def interp(expr: Expr, env: Env, sto: Sto): (Value, Sto) = expr match {
      case Id(x) => {
        val addr = env.getOrElse(x, error(s"free identifier $x"))
        val v = sto.getOrElse(addr, error(s"free address $addr"))
        v match {
          case ExprV(e, venv) => {
            val (v1, s1) = interp(e, venv, sto)
            (v1, s1 + (addr -> v1))
          }
          case _ => (v, sto)
        }
      }
      case IntE(n) => (IntV(n), sto)
      case BooleanE(b) => (BooleanV(b), sto)
      case UnitE => (UnitV, sto)
      case Add(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVAdd(lv, rv)
        (result, rs)
      }
      case Mul(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVMul(lv, rv)
        (result, rs)
      }
      case Div(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVDiv(lv, rv)
        (result, rs)
      }
      case Mod(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVMod(lv, rv)
        (result, rs)
      }
      case Eq(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVEq(lv, rv)
        (result, rs)
      }
      case Lt(l, r) => {
        val (lv, ls) = interp(l, env, sto)
        val (rv, rs) = interp(r, env, ls)
        val result = intVLt(lv, rv)
        (result, rs)
      }
      case Sequence(l,r) => {
        val (v1, s1) = interp(l, env, sto)
        interp(r, env, s1)
      }
      case If(c, t, f) => { 
        val (v1, s1) = interp(c, env, sto)
        v1 match {
          case BooleanV(b) => b match {
            case true => interp(t, env, s1)
            case false => interp(f, env, s1)
          }
          case _ => error(s"conditoinal statement must be boolean")
        }
      }

      case Val(x, e, bd) => {
        val (v1, s1) = interp(e, env, sto)
        val addr = malloc(s1)
        interp(bd, env + (x->addr), s1 + (addr->v1))
      }

      case RecBinds(defs, bd) => {
        def createnv(dfs: List[RecDef], curenv: Env, cursto: Sto): Env = dfs match {
          case Nil => curenv
          case h::t => {
            val (tempenv, tempsto) = EnvbyRecDef(h, cursto)
            val nenv = curenv ++ tempenv
            createnv(t, nenv, tempsto)
          }
        }
        val envN = createnv(defs, Map(), sto)

        val nenv = env ++ envN

        def storesto(dfs: List[RecDef], cursto: Sto): Sto = dfs match {
          case Nil => cursto
          case h::t => {
            val tempsto = StobyRecDef(h, nenv)
            val nsto = cursto ++ tempsto
            storesto(t, nsto)
          }
        }
        val tempsto = storesto(defs, Map())
        val nsto = sto ++ tempsto
        interp(bd, nenv, nsto)
      }
      case Fun(params, bd) => (CloV(params, bd, env), sto)
      case Assign(x, e) => {
        val addr = env.getOrElse(x, error(s"free identifier $x"))
        val (v, s) = interp(e, env, sto)
        (UnitV, s+(addr->v))
      }
      case App(f, as) => {
        val (v, s) = interp(f, env, sto)

        def argsinterp(args: List[Expr], list: List[Value], cursto: Sto): (List[Value], Sto) = args match {
          case Nil => (list, cursto)
          case h::t => {
            val (vi, si) = interp(h, env, cursto)
            argsinterp(t, vi::list, si)
          }
        }

        val (tempvlist, sN) = argsinterp(as, Nil, s)

        val vlist = tempvlist.reverse

        v match {
          case CloV(xs, bd, fenv) => {
            if(as.length == xs.length) {
              val mapping = xs zip vlist
              def update(uplist: List[(String, Value)], curenv: Env, cursto: Sto): (Env, Sto) = uplist match {
                case Nil => (curenv, cursto)
                case (hx, hv)::t => {
                  val addr = malloc(cursto)
                  update(t, curenv+(hx->addr), cursto+(addr->hv))
                }
              }
              val (nenv, nsto) = update(mapping, fenv, sN)
              interp(bd, nenv, nsto)
            }
            else error(s"arguments and parameters don't match")
          }
          case ConstructorV(x) => (VariantV(x, vlist), sN)
          case _ => error(s"$v should be either a closure or a constructor")
        } 
      }
      case Match(e, cs) => {
        val (v, s) = interp(e, env, sto)
        v match {
          case VariantV(x, vlist) => {
            def find(name: String, cs: List[Case]): (Expr, Env, Sto) = cs match {
              case Nil => error(s"no such case for $x")
              case Case(cname, xs, bd)::t => {
                if(cname == name) {
                  if(vlist.length == xs.length){
                    val mapping = xs zip vlist
                    def update(uplist: List[(String,Value)], curenv: Env, cursto: Sto): (Env, Sto) = uplist match {
                      case Nil => (curenv, cursto)
                      case (hx, hv)::t => {
                        val addr = malloc(cursto)
                        update(t, curenv+(hx->addr), cursto+(addr->hv))
                      }
                    }
                    val (envM, stoM) = update(mapping, env, sto)
                    (bd, envM, stoM)
                  }
                  else error(s"variant of case doesn't match")
                }
                else find(name, t)
              }
            }
            val (expr, nenv, nsto) = find(x, cs)
            interp(expr, nenv, nsto)
          }
          case _ => error(s"$v must be a variant value")
        } 
      }
    }
  }
}
