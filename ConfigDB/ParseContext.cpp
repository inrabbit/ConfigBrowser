#include "ParseContext.h"
#include "BaseDB.h"
#include "Equation.h"
#include "CdbException.h"
#include "TokenAnaly.h"
#include <cassert>

// --------------------------------------------------

void SubsetContext::setRootDB(BaseDB *pDB)
{
	assert(pDB != NULL);
	m_pDB = pDB;
}

void SubsetContext::createSubset(const char *pName)
{
	assert(m_pDB != NULL);

	CDB_ERR error;
	UnivType& SubsetObj = m_pDB->getUnivType(pName, error);
	switch(error){
	case CDB_OK:
		if(SubsetObj.isSubset()){
			m_pDB = SubsetObj.getSubsetDB();
		}else if(!SubsetObj.isValid()){
			m_pDB = m_pDB->createSubset(&SubsetObj);
		}else{
			// 既に変数がsubset以外として使われている
			throw CdbException(CDB_KEY_COLLISION, pName);
		}
		break;
	case CDB_NOTFOUND:
		{
			UnivType &newObj = m_pDB->addNew(pName);	// キー名称の重複チェックは行わないことに注意
			m_pDB = m_pDB->createSubset(&newObj);
		}
		break;
	default:
		throw CdbException(error);
	}
}

void SubsetContext::createSubset(UnivType *pObj)
{
	m_pDB = m_pDB->createSubset(pObj);
}

void SubsetContext::setSubset(UnivType *pSubset)
{
	m_pSubset = pSubset;

	// 【注意】ファイル読み込み中のため、数式である場合には、isValid()評価をまだできない
	// 2010/04/17 関数定義式の引数は初期値emptyなので、これも不定状態とする
	if(pSubset->isReference() || pSubset->isEquation() || (!pSubset->isEquation() && !pSubset->isValid()) || pSubset->isEmpty()){
		// pSubsetが不定(変数、数式、未定義である）な場合
		m_pDB = NULL;
	}else if(pSubset->isSubset()){
		// pSubsetが確定したサブセットである場合
		m_pDB = pSubset->getSubsetDB();
	}else{
		// pSubsetがサブセットとなる可能性がない場合
		throw CdbException(CDB_NOT_SUBSET);
	}
}

UnivType& SubsetContext::getSubset()
{
	UnivType& subset = addNew();
	subset.setSubset(m_pDB, m_pDB != RequireUpdateObj::getDB());
	return subset;
}

UnivType& SubsetContext::addNew(const char *pName, CDB_ERR& error)
{
	assert(m_pDB != NULL);
	return m_pDB->addNew(pName, error);
}

UnivType& SubsetContext::addNew(const char *pName)
{
	assert(m_pDB != NULL);
	return m_pDB->addNew(pName);
}

UnivType& SubsetContext::addNew()
{
	assert(m_pDB != NULL);
	return m_pDB->addNew();
}

CDB_ERR SubsetContext::rename(UnivType *pTarget, const char *pName)
{
	assert(m_pDB != NULL);
	return m_pDB->rename(pTarget, pName);
}

UnivType& SubsetContext::runtimeGetFromSubset(const char *pName)
{
	// BaseDBポインタが最後に有効であった時点まで遡る
	SubsetContext *pFixedSubset = this;
	while(pFixedSubset->m_pDB == NULL){
		pFixedSubset = pFixedSubset->GetPrevContext();
	}

	UnivType& obj = pFixedSubset->m_pDB->addNew();
	Equation *pEquation = new Equation;
	obj.setEquation(pEquation);

	pEquation->setOperation(Equation::OP_SUBSET_ACCESS);
	pEquation->setNumArguments(2);
	pEquation->getArgumentAt(0).setConstReference(m_pSubset);
	pEquation->getArgumentAt(1).setString(pName);

	return obj;
}

UnivType& SubsetContext::getUnsafeUnivType(const char *pName)
{
	if(m_pDB != NULL){
		return m_pDB->getUnsafeUnivType(pName, m_bSubsetOwnerSearch);
	}else{
		return runtimeGetFromSubset(pName);
	}	
}

// --------------------------------------------------

void BraceContext::setDefault()
{
	m_currentBrace = TK_INVALID;
	m_countTerm = INVALID;
	m_countPrimitive = INVALID;
	m_braceLevel = 0;
}

// --------------------------------------------------

void IndexContext::saveCurrentTokenPos(TokenAnaly& token)
{
	m_saveTokenPos = token.getCurrentTokenPos();
}

void IndexContext::loadTokenPos(TokenAnaly& token)
{
	int delta = token.getCurrentTokenPos() - m_saveTokenPos;
	if(delta > TOKEN_MAX_BACK) throw CdbException(CDB_EXCEED_MAX_TOKEN_BACK, delta);
	token.putBack(delta);
}
