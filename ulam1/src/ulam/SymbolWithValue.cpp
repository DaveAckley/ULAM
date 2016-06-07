#include "SymbolWithValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolWithValue::SymbolWithValue(Token id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_hasInitVal(false), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(false)
  {
    m_constant.sval = 0;
    m_initial.sval = 0;
  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref) : Symbol(sref), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(sref.m_classArgument)
  {
    m_constant = sref.m_constant;
    m_initial = sref.m_initial;
  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref, bool keepType) : Symbol(sref, keepType), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(sref.m_classArgument)
  {
    m_constant = sref.m_constant;
    m_initial = sref.m_initial;
  }

  SymbolWithValue::~SymbolWithValue()
  { }


  bool SymbolWithValue::isClassParameter()
  {
    return m_classParameter;
  }

  void SymbolWithValue::setClassParameterFlag()
  {
    m_classParameter = true;
  }

  bool SymbolWithValue::isClassArgument()
  {
    return m_classArgument;
  }

  void SymbolWithValue::setClassArgumentFlag()
  {
    m_classArgument = true;
  }

  u32 SymbolWithValue::getPosOffset()
  {
    return 0; //always zero
  }

  bool SymbolWithValue::isReady()
  {
    return m_isReady; //constant value
  }

  bool SymbolWithValue::getValue(s64& val)
  {
    val = m_constant.sval;
    return m_isReady;
  }

  bool SymbolWithValue::getValue(u64& val)
  {
    val = m_constant.uval;
    return m_isReady;
  }

  void SymbolWithValue::setValue(s64 val)
  {
    m_constant.sval = val;
    m_isReady = true;
  }

  void SymbolWithValue::setValue(u64 val)
  {
    m_constant.uval = val;
    m_isReady = true;
  }

  bool SymbolWithValue::hasInitValue()
  {
    return m_hasInitVal;
  }

  bool SymbolWithValue::getInitValue(s64& val)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	val = m_initial.sval;
	return true;
      }
    return false; //was m_hasDefault;
  }

  bool SymbolWithValue::getInitValue(u64& val)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	val = m_initial.uval;
	return true;
      }
    return false; //was m_hasDefault;
  }

  void SymbolWithValue::setInitValue(s64 val)
  {
    m_initial.sval = val;
    m_hasInitVal = true;
    m_isReadyInitVal = true;
  }

  void SymbolWithValue::setInitValue(u64 val)
  {
    m_initial.uval = val;
    m_hasInitVal = true;
    m_isReadyInitVal = true;
  }

  bool SymbolWithValue::isInitValueReady()
  {
    return m_isReadyInitVal;
  }

  void SymbolWithValue::setHasInitValue()
  {
    m_hasInitVal = true;
    m_isReadyInitVal = false;
  }

  bool SymbolWithValue::foldConstantExpression()
  {
    return true; //stub
  }

  void SymbolWithValue::printPostfixValue(File * fp)
  {
    UTI tuti = getUlamTypeIdx();
    bool oktoprint = true;
    u64 val = 0;
    if(isReady())
      val = m_constant.uval;
    else if(hasInitValue() && isInitValueReady())
      val = m_initial.uval;
    else
      oktoprint = false;

    if(oktoprint)
      {
	u32 twordsize =  m_state.getTotalWordSize(tuti); //must be commplete
	s32 tbs = m_state.getBitSize(tuti);
	ULAMTYPE etype = m_state.getUlamTypeByIndex(tuti)->getUlamTypeEnum();

	switch(etype)
	  {
	  case Int:
	    {
	      if(twordsize <= MAXBITSPERINT)
		{
		  s32 sval = _Int32ToCs32((u32) val, tbs);
		  fp->write_decimal(sval);
		}
	      else if(twordsize <= MAXBITSPERLONG)
		{
		  s64 sval = _Int64ToCs64(val, tbs);
		  fp->write_decimal_long(sval);
		}
	      else
		assert(0);
	    }
	    break;
	  case Bool:
	    {
	      bool bval = _Bool64ToCbool(val, tbs);
	      if(bval)
		fp->write("true");
	      else
		fp->write("false");
	    }
	    break;
	  case Unary:
	  case Unsigned:
	  case Bits:
	    {
	      // NO CASTING NEEDED, assume saved in its ulam-native format
	      //oddly write_decimal wants a signed int..use new write_decimal_unsigned
	      if( tbs <= MAXBITSPERINT)
		fp->write_decimal_unsigned(val);
	      else if( tbs <= MAXBITSPERLONG)
		fp->write_decimal_unsignedlong(val);
	      else
		assert(0);
	      fp->write("u");
	    }
	    break;
	  default:
	    assert(0);
	  };
      }
    else
      fp->write("NONREADYCONST");
  } //printPostfixValue

  bool SymbolWithValue::getLexValue(std::string& vstr)
  {
    UTI tuti = getUlamTypeIdx();

    if(!isReady())
      return false;

    u32 twordsize =  m_state.getTotalWordSize(tuti); //must be commplete
    s32 tbs = m_state.getBitSize(tuti);
    ULAMTYPE etype = m_state.getUlamTypeByIndex(tuti)->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	{
	  if(twordsize <= MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) m_constant.uval, tbs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(sval);
	    }
	  else if(twordsize <= MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(m_constant.uval, tbs, MAXBITSPERLONG);
	      vstr = ToLeximitedNumber64(sval);
	    }
	  else
	    assert(0);
	}
	break;
      case Bool:
	{
	  bool bval = _Bool64ToCbool(m_constant.uval, tbs);
	  if(bval)
	    vstr = ToLeximitedNumber(1); //true
	  else
	    vstr = ToLeximitedNumber(0); //false
	}
	break;
      case Unary:
	{
	  s32 pval = _Unary64ToInt64(m_constant.uval, tbs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(pval);
	}
	break;
      case Unsigned:
      case Bits:
	{
	  //oddly write_decimal wants a signed int..
	  if( tbs <= MAXBITSPERINT)
	    vstr = ToLeximitedNumber((u32) m_constant.uval);
	  else if( tbs <= MAXBITSPERLONG)
		vstr = ToLeximitedNumber64(m_constant.uval);
	  else
	    assert(0);
	}
	break;
      default:
	assert(0);
      };
    return true;
  } //getLexValue

  //warning: this change also requires an update to the ST's key.
  void SymbolWithValue::changeConstantId(u32 fmid, u32 toid)
  {
    assert(getId() == fmid);
    setId(toid);
  }

} //end MFM
