#include <stdlib.h>
#include "NodeVarRef.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRef::NodeVarRef(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state)
  {
    Node::setStoreIntoAble(TBOOL_HAZY);
  }

  NodeVarRef::NodeVarRef(const NodeVarRef& ref) : NodeVarDecl(ref) { }

  NodeVarRef::~NodeVarRef() { }

  Node * NodeVarRef::instantiate()
  {
    return new NodeVarRef(*this);
  }

  void NodeVarRef::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
  } //updateLineage

  bool NodeVarRef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeVarRef::checkAbstractInstanceErrors()
  {
    //unlike NodeVarDecl, an abstract class can be a reference!
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    if(nut->getUlamTypeEnum() == Class)
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);
	if(csym->isAbstract())
	  {
	    std::ostringstream msg;
	    msg << "Instance of Abstract Class ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " used with reference variable symbol name '";
	    msg << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	  }
      }
  } //checkAbstractInstanceErrors

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarRef::printPostfix(File * fp)
  {
    NodeVarDecl::printPostfix(fp);
  } //printPostfix

  void NodeVarRef::printTypeAndName(File * fp)
  {
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(vuti);
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    fp->write(" ");
    if(vut->getUlamTypeEnum() != Class)
      {
	fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
	fp->write("&"); //<--the only difference!!!
      }
    else
      fp->write(vut->getUlamTypeNameBrief().c_str()); //includes any &

    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(vuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }
  } //printTypeAndName

  const char * NodeVarRef::getName()
  {
    return NodeVarDecl::getName(); //& ??
  }

  const std::string NodeVarRef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarRef::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType();
    nuti = m_state.getUlamTypeAsDeref(nuti);
    //cast RHS if necessary and safe
    //insure lval is same bitsize/arraysize
    // if classes, safe to cast a subclass to any of its superclasses
    FORECAST rscr = CAST_CLEAR;
    if(UlamType::compare(nuti, newType, m_state) != UTIC_SAME)
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	UlamType * newt = m_state.getUlamTypeByIndex(newType);
	rscr = newt->safeCast(nuti);

	if((nut->getUlamTypeEnum() == Class))
	  {
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Incompatible class types ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << " and ";
		msg << newt->getUlamTypeNameBrief().c_str();
		msg << " used to initalize reference '" << getName() <<"'";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    //primitives must be exactly the same size;
	    // even "clear" when complete yet not the same is "bad".
	    if(rscr != CAST_CLEAR)
	      {
		std::ostringstream msg;
		msg << "Reference variable " << getName() << "'s type ";
		msg << nut->getUlamTypeNameBrief().c_str();
		msg << ", and its initial value type ";
		msg << newt->getUlamTypeNameBrief().c_str();
		msg << ", are incompatible";
		if(rscr == CAST_HAZY)
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		else
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
      }
    return rscr;
  } //safeToCastTo

  UTI NodeVarRef::checkAndLabelType()
  {
    UTI it = NodeVarDecl::checkAndLabelType();

    //assert((it == Nav) || (it == Hzy) || m_state.getUlamTypeByIndex(it)->isReference());

    if(m_state.okUTItoContinue(it) && (!m_state.getUlamTypeByIndex(it)->isReference()))
      {
	std::ostringstream msg;
	msg << "Variable reference '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "', is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    ////requires non-constant, non-funccall value
    //NOASSIGN REQUIRED (e.g. for function parameters) doesn't have to have this!
    if(m_nodeInitExpr)
      {
	UTI eit = m_nodeInitExpr->getNodeType();
	if(eit == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(eit == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //possibly still hazy
	    m_state.setGoAgain();
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	//check isStoreIntoAble
	//if(m_nodeInitExpr->isAConstant() || m_nodeInitExpr->isFunctionCall())
	TBOOL istor = m_nodeInitExpr->getStoreIntoAble();
	Node::setStoreIntoAble(istor);

	if(istor != TBOOL_TRUE)
	  {
	    std::ostringstream msg;
	    msg << "Storage expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", must be storeintoable";
	    if(istor == TBOOL_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_state.setGoAgain();
		it = Hzy;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav; //short-circuit
	      }
	  }
      }
    else
      {
	if(it == Nav)
	  Node::setStoreIntoAble(TBOOL_FALSE);
	else if(it == Hzy)
	  Node::setStoreIntoAble(TBOOL_HAZY);
	else
	  Node::setStoreIntoAble(TBOOL_TRUE);
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeVarRef::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert(0); //refs can't be data members
  } //packBitsInOrderOfDeclaration

  void NodeVarRef::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return NodeVarDecl::calcMaxDepth(depth, maxdepth, base);
  } //calcMaxDepth

  void NodeVarRef::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarDecl::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavNodes

  EvalStatus NodeVarRef::eval()
  {
    assert(m_varSymbol);

    assert(m_varSymbol->getAutoLocalType() != ALT_AS); //NodeVarRefAuto

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(m_nodeInitExpr);

    evalNodeProlog(0); //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr

    EvalStatus evs = m_nodeInitExpr->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.popArg();
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses
    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(m_nodeInitExpr->getNodeType()); //for future virtual function call eval uses

    m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..

    evalNodeEpilog();
    return NORMAL;
  } //eval

  EvalStatus NodeVarRef::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  void NodeVarRef::genCode(File * fp, UlamValue & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);

    if(m_nodeInitExpr)
      {
	// get the right-hand side, stgcos
	// can be same type (e.g. element, quark, or primitive),
	// or ancestor quark if a class.
	m_nodeInitExpr->genCodeToStoreInto(fp, uvpass);

	UTI vuti = m_varSymbol->getUlamTypeIdx(); //i.e. this ref node
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);

	if(m_state.isAtom(vuti))
	  return genCodeAtomRefInit(fp, uvpass);

	Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	UTI stgcosuti = stgcos->getUlamTypeIdx();
	UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);


	if(!stgcosut->isScalar() && !vut->isScalar())
	  return genCodeArrayRefInit(fp, uvpass);

	if(!stgcosut->isScalar() && vut->isScalar())
	  return genCodeArrayItemRefInit(fp, uvpass);

	Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
	UTI cosuti = cos->getUlamTypeIdx();
	UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

	ULAMCLASSTYPE vclasstype = vut->getUlamClass();
	ULAMTYPE vetype = vut->getUlamTypeEnum();

	assert(vetype == cosut->getUlamTypeEnum());

	m_state.indent(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");

	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("("); //pass ref in constructor (ref's not assigned with =)
	if(stgcos->isDataMember()) //can't be an element or atom
	  {
	    fp->write(m_state.getHiddenArgName());
	    fp->write(", ");
	    fp->write_decimal_unsigned(cos->getPosOffset()); //relative of
	    fp->write("u");
	  }
	else
	  {
	    fp->write(stgcos->getMangledName().c_str());
	    if(cos->isDataMember())
	      {
		fp->write(", ");
		fp->write_decimal_unsigned(cos->getPosOffset()); //relative off
		fp->write("u");
	      }
	    else
	      {
		//local var
		if((vclasstype == UC_NOTACLASS) && !m_state.isAtom(vuti))
		  {
		    fp->write(", 0u"); //relative
		  }
		else if(vclasstype == UC_QUARK)
		  {
		    fp->write(", 0u"); //left-justified
		  }
	      }
	  }

	if((vclasstype != UC_NOTACLASS) && (vetype != UAtom))
	  {
	    fp->write(", &");
	    fp->write(stgcosut->getUlamTypeMangledName().c_str()); //effself
	    fp->write("<EC>::THE_INSTANCE");
	  }
	fp->write(");\n");
      } //storage

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCode

  void NodeVarRef::genCodeAtomRefInit(File * fp, UlamValue & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);

    assert(m_nodeInitExpr);

    s32 tmpVarNum = uvpass.getPtrSlotIndex(); //tmp containing atomref

    UTI vuti = m_varSymbol->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    UTI puti = uvpass.getUlamValueTypeIdx();
    if(m_state.isPtr(puti))
      puti = uvpass.getPtrTargetType();
    assert(m_state.isAtom(vuti) && m_state.isAtom(puti));

    m_state.indent(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)

    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      fp->write(m_state.getTmpVarAsString(puti, tmpVarNum, TMPBITVAL).c_str());
    else
      {
	Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
	fp->write(stgcos->getMangledName().c_str());
      }
    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCodeAtomRefInit

  void NodeVarRef::genCodeArrayRefInit(File * fp, UlamValue & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);
    assert(m_nodeInitExpr);

    // get the right-hand side, stgcos
    // can be same type (e.g. element, quark, or primitive),
    // or ancestor quark if a class.
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);
    UTI scalarcosuti = m_state.getUlamTypeAsScalar(cosuti);
    UlamType * scalarcosut = m_state.getUlamTypeByIndex(scalarcosuti);

    UTI vuti = m_varSymbol->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(!vut->isScalar()); //entire array

    ULAMCLASSTYPE vclasstype = vut->getUlamClass();
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    assert(vetype == cosut->getUlamTypeEnum());

    m_state.indent(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)
    if(stgcos->isDataMember()) //can't be an element or atom
      {
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");
	fp->write_decimal_unsigned(cos->getPosOffset()); //relative off
	fp->write("u");
      }
    else
      {
	//local
	fp->write(stgcos->getMangledName().c_str());
	if(cos->isDataMember())
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(cos->getPosOffset()); //relative off
	    fp->write("u");
	  }
	else
	  {
	    //local
	    if((vclasstype == UC_NOTACLASS) && (vetype != UAtom) )
	      {
		fp->write(", 0u, "); //rel off to right-just prim
	      }
	    else if(vetype == UAtom)
	      {
		fp->write(", uc");
	      }
	    else if(vclasstype == UC_QUARK)
	      {
		fp->write(", ");
		fp->write("0u, &"); //left just
		fp->write(scalarcosut->getUlamTypeMangledName().c_str());
		fp->write("<EC>::THE_INSTANCE");
	      }
	    else if(vclasstype == UC_ELEMENT)
	      {
		fp->write(", &");
		fp->write(scalarcosut->getUlamTypeMangledName().c_str());
		fp->write("<EC>::THE_INSTANCE");
	      }
	    else
	      assert(0);
	  }
      } //storage

    fp->write(");\n");

    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCodeArrayRefInit

  void NodeVarRef::genCodeArrayItemRefInit(File * fp, UlamValue & uvpass)
  {
    //reference always has initial value, unless func param
    assert(m_varSymbol->isAutoLocal());
    assert(m_varSymbol->getAutoLocalType() != ALT_AS);
    assert(m_nodeInitExpr);

    // get the right-hand side, stgcos
    // can be same type (e.g. element, quark, or primitive),
    // or ancestor quark if a class.
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);

    Symbol * cos = m_state.m_currentObjSymbolsForCodeGen.back();
    UTI cosuti = cos->getUlamTypeIdx();
    UlamType * cosut = m_state.getUlamTypeByIndex(cosuti);

    UTI vuti = m_varSymbol->getUlamTypeIdx(); //i.e. this ref node
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    assert(vut->isScalar()); //item of array

    ULAMCLASSTYPE vclasstype = vut->getUlamClass();
    ULAMTYPE vetype = vut->getUlamTypeEnum();
    assert(vetype == cosut->getUlamTypeEnum());

    m_state.indent(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");

    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("("); //pass ref in constructor (ref's not assigned with =)
    if(stgcos->isDataMember()) //can't be an element or atom
      {
	fp->write(m_state.getHiddenArgName());
	fp->write(", ");
	fp->write_decimal_unsigned(cos->getPosOffset()); //relative off
	fp->write("u + (");
	fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
	fp->write(" * ");
	fp->write_decimal_unsigned(stgcosut->getBitSize());
	fp->write("u)");
      }
    else
      {
	//local
	fp->write(stgcos->getMangledName().c_str());
	if(cos->isDataMember())
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(cos->getPosOffset()); //relative off
	    fp->write("u + (");
	    fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
	    fp->write(" * ");
	    fp->write_decimal_unsigned(stgcosut->getBitSize());
	    fp->write("u)");
	  }
	else
	  {
	    //local
	    if((vclasstype == UC_NOTACLASS) && (vetype != UAtom) )
	      {
		fp->write(", (");
		fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
		fp->write(" * ");
		fp->write_decimal_unsigned(cosut->getBitSize());
		fp->write("u)"); //relative t3651
	      }
	    else if(vetype == UAtom)
	      {
		fp->write(", uc");
	      }
	    else if(vclasstype == UC_QUARK)
	      {
		fp->write(", ");
		fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex(), uvpass.getPtrStorage()).c_str());
		fp->write(" * ");
		fp->write_decimal_unsigned(stgcosut->getBitSize());
	      }
	  }
      } //storage

    if((vclasstype != UC_NOTACLASS) && (vetype != UAtom))
      {
	UTI derefuti = m_state.getUlamTypeAsDeref(vuti); //vuti is scalar!
	UlamType * derefut = m_state.getUlamTypeByIndex(derefuti);

	fp->write(", &"); //left just
	fp->write(derefut->getUlamTypeMangledName().c_str()); //effself
	fp->write("<EC>::THE_INSTANCE");
      }
    fp->write(");\n");
    m_state.m_currentObjSymbolsForCodeGen.clear(); //clear remnant of rhs ?
  } //genCodeArrayItemRefInit

} //end MFM
