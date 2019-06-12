#include "Parser.h"
#include "ParseContext.h"
#include "BaseDB.h"
#include "UnivType.h"
#include "NamedUnivType.h"
#include "TokenAnaly.h"
#include "Equation.h"
#include "IndexConv.h"
#include "CdbException.h"
#include "WrappedVector.h"
#include <cassert>

// ------------------------------------------------------------

Parser::Parser(TokenAnaly& token, BaseDB& database) : m_token(token), m_RootDB(database)
{
	m_pCustomFuncArguments = NULL;
	m_pSubsetContext = NULL;
	m_pBraceContext = NULL;
	m_pSentenceContext = NULL;
	m_pIndexContext = NULL;
	m_pCatContext = NULL;
}

Parser::~Parser()
{
}

void Parser::execute()
{
	BraceContext brace_context(m_pBraceContext);
	SubsetContext subset_context(m_pSubsetContext);
	m_pSubsetContext->setRootDB(&m_RootDB);	// �������[�gDB�i�O���[�o�����O��ԁj�̐ݒ�

	try{
		// �g�[�N������ɂȂ�܂ŉ�͂��s��
		while(getNextToken() != TK_EMPTY){
			putBackToken();
			topScript();
		}
	}
	catch(CdbException& e){
		e.addFileLine(m_token.getCurrentLine());
		throw e;
	}
}

void Parser::interpret(UnivType& obj)
{
	BraceContext brace_context(m_pBraceContext);
	SubsetContext subset_context(m_pSubsetContext);
	m_pSubsetContext->setRootDB(&m_RootDB);	// �������[�gDB�i�O���[�o�����O��ԁj�̐ݒ�

	try{
		topRight(obj);
	}
	catch(CdbException& e){
		e.addFileLine(m_token.getCurrentLine());
		throw e;
	}

	// TODO : 120117 �ȑO�̕]�����ʂ��㏑�������Ƃ��ɁA�����ł������p����Ă��Ȃ��������O�Ȃ��I�u�W�F�N�g���S�~�Ƃ��Ďc���肪����
	// �ǂ�������Q�Ƃ���ĂȂ����͍̂폜����@�\���K�v
}

bool Parser::generateReference(UnivType& ref)
{
	return true;
}

// ------------------------------------------------------------

enum TokenKind Parser::getNextToken()
{
	return m_token.getNext();
}

bool Parser::checkNextToken(enum TokenKind kind)
{
	if(m_token.getNext() == kind){
		return true;
	}else{
		m_token.putBack();
		return false;
	}
}

bool Parser::assertNextToken(enum TokenKind kind)
{
	if(m_token.getNext() == kind){
		return true;
	}else{
		throw CdbException(CDB_SYNTAX_ERROR);
		return false;
	}
}

void Parser::getCurrentTokenContent(UnivType& obj)
{
	obj.move(m_token.getCurrentContent());
}

void Parser::putBackToken()
{
	m_token.putBack();
}

UnivType& Parser::getObjectByName(const char *pName)
{
	CDB_ERR error = CDB_OK;
	if(m_pCustomFuncArguments != NULL){
		void *p = NULL;
		error = m_pCustomFuncArguments->getPtr(pName, p);
		if(error == CDB_OK) return *static_cast<UnivType *>(p);
	}

	try{
		return m_pSubsetContext->getUnsafeUnivType(pName);
	}
	catch(CdbException& e){
		if(e.getErrorCode() != CDB_NOTFOUND){
			e.addVariableName(pName);
			throw e;
		}
	}

	UnivType& new_obj = m_pSubsetContext->addNew(pName, error);
	if(error != CDB_OK) throw CdbException(error);
	return new_obj;
}

// ------------------------------------------------------------

// �X�N���v�g��̓g�b�v
void Parser::topScript()
{
	enum TokenKind eKind = getNextToken();
	switch(eKind){
	case TK_SUBSET:
		subsetDefine();
		while(checkNextToken(TK_COMMA)){
			subsetDefine();
		}
		break;
	default:
		putBackToken();
		topEquation();
	}
}

// �T�u�Z�b�g��`
void Parser::subsetDefine()
{
	// ���̎��_��subset�L�[���[�h�̎��̃g�[�N���̏�ԂɂȂ��Ă��邱��

	// ���̓T�u�Z�b�g���̂�����������łȂ���΂Ȃ�Ȃ�
	enum TokenKind eKind = getNextToken();
	if(eKind != TK_SYMBOL){
		throw CdbException(CDB_SYNTAX_ERROR);
	}
	UnivType SubsetName;
	getCurrentTokenContent(SubsetName);

	// �T�u�Z�b�g���쐬���A����ɐ؂�ւ���
	SubsetContext context(m_pSubsetContext);
	m_pSubsetContext->createSubset(SubsetName);

	// ������ . �������ꍇ�́A���̊K�w�̃T�u�Z�b�g���`����
	if(checkNextToken(TK_DOT)){
		subsetDefine();
	}else{
		// ���������� { �������ꍇ�̂݁A���̓ǂݍ��݂��s��
		if(checkNextToken(TK_BRACE_OPEN)){

			// } ������܂ŉ�͂��s��
			while(eKind = m_token.getNext(), eKind != TK_BRACE_CLOSE){
				if(eKind == TK_EMPTY){
					throw CdbException(CDB_SYNTAX_ERROR);
				}
				m_token.putBack();
				topScript();
			}
		}
	}
}

// ����̓g�b�v
// �y���Ӂz
// reference�����́A���ӁE�E�ӂ̍\�z�ɂ����ĎQ�Ƃ����
// const�����́A���ӎ����̍\�z�ɂ����Ă̂ݎQ�Ƃ����i�E�ӂł͎Q�Ƃ��Ȃ��j
void Parser::topEquation()
{
	enum TokenKind eKind = getNextToken();
	switch(eKind){
	case TK_VARIABLE:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = false;
			m_pSentenceContext->m_isConstant = false;

			// ���ӎ����Q�ƃI�u�W�F�N�g���\�z
			UnivType& obj = topLeft();
			// = �����݂��Ă��邱��
			assertNextToken(TK_SUBST);
			// �E�ӎ��I�u�W�F�N�g���\�z
			topRight(obj);
		}
		break;
	case TK_REFERENCE:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = true;
			m_pSentenceContext->m_isConstant = false;	// �Q�Ƃɂ����Ē萔�����͊֌W�Ȃ����A�Ƃ肠����false�Ƃ��Ă���

			// ���ӎ����Q�ƃI�u�W�F�N�g���\�z
			UnivType& obj = topLeft();
			// = �����݂��Ă��邱��
			assertNextToken(TK_SUBST);
			// �E�ӎ��I�u�W�F�N�g���\�z
			topRight(obj);

#if 0	// 090805 ���ۂɂǂ���������̂��͖���ł��邪�A���ʎZ�o�^�̐����ɑ΂��Ă�reference�ł̓G���[���o������
			// ���ۂɕϐ��ւ̃��t�@�����X�ɂȂ��Ă��Ȃ�������G���[
			if(!obj.isFullReference()){
				throw CdbException(CDB_CANNOT_REFERENCE);
			}
#endif
		}
		break;
	case TK_FUNC:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = false;
			m_pSentenceContext->m_isConstant = true;	// TODO : const�����̉e���͈͂����m�łȂ�

			// ���������邽�߂̃R���e�i���쐬
			assert(m_pCustomFuncArguments == NULL);
			m_pCustomFuncArguments = new BaseDB;
			// ���ӎ����J�X�^���֐��I�u�W�F�N�g���\�z
			UnivType& eqn = topFunc();
			// = �����݂��Ă��邱��
			assertNextToken(TK_SUBST);
			// �E�ӎ��I�u�W�F�N�g���\�z
			topRight(eqn);
			// �����p�R���e�i���폜
			delete m_pCustomFuncArguments;
			m_pCustomFuncArguments = NULL;
		}
		break;
	default:	// �ϐ��̓f�t�H���g�Œ萔�����Ƃ���i�����I��const�C���q�ɈӖ��͂Ȃ��j
		putBackToken();
	case TK_CONST:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = false;
			m_pSentenceContext->m_isConstant = true;

			// ���ӎ����Q�ƃI�u�W�F�N�g���\�z
			UnivType& obj = topLeft();
			// = �����݂��Ă��邱��
			assertNextToken(TK_SUBST);
			// �E�ӎ��I�u�W�F�N�g���\�z
			topRight(obj);
		}
		break;
	}

	// �����}�[�N�i�Z�~�R�����j�̃`�F�b�N
	if(!checkNextToken(TK_SEMICOLON)){
		// 090806 �����̃Z�~�R�����́A�Ȃ��Ă��G���[��\�����Ȃ�
		//throw CdbException(CDB_MISSING_LINE_END_MARK);
	}
}

UnivType& Parser::topLeft()
{
	SentenceContext context(m_pSentenceContext);
	m_pSentenceContext->m_isInsideRight = false;

	UnivType& obj = m_pSubsetContext->addNew();
	matrixLeft(obj);
	return obj;
}

UnivType& Parser::topFunc()
{
	assertNextToken(TK_SYMBOL);
	UnivType FuncName;
	getCurrentTokenContent(FuncName);
	
	// �J�X�^���֐��I�u�W�F�N�g���쐬
	UnivType& FuncObj = m_pSubsetContext->addNew((const char *)FuncName);
	Equation *pEquation = new Equation;
	pEquation->setOperation(Equation::OP_CUSTOM_FUNC);
	FuncObj.setEquation(pEquation);
	// �萔������t��
	FuncObj.setConstant(m_pSentenceContext->m_isConstant);
	
	assert(m_pCustomFuncArguments != NULL);
	
	// ��������argObjs�ɓo�^����
	WrappedVector<UnivType *> argObjs;
	
	enum TokenKind eKind = getNextToken();
	if(eKind == TK_BRACKET_RND_OPEN){
		eKind = getNextToken();
		for(; eKind != TK_BRACKET_RND_CLOSE; eKind = getNextToken()){
			if(eKind != TK_SYMBOL){
				throw CdbException(CDB_SYNTAX_ERROR);
			}
			UnivType *pArgName = new UnivType;
			getCurrentTokenContent(*pArgName);
			argObjs.push_back(pArgName);

			eKind = getNextToken();
			if(eKind != TK_COMMA){
				putBackToken();
				break;
			}
		}

		assertNextToken(TK_BRACKET_RND_CLOSE);
	}else{
		putBackToken();
	}
	
	int nArgs = argObjs.size();
	pEquation->setNumArguments(1 + nArgs);	// �ŏ��̂P�����͐���������

	// �����ϐ��c�a�ɃJ�X�^���֐��I�u�W�F�N�g�̈����ւ̎Q�Ƃ�ݒ肷��
	for(int i = 0; i < nArgs; i++){
		const char *pArgName = argObjs[i]->getString();
		//m_pCustomFuncArguments->addNew(pArgName).setReference(&pEquation->getArgumentAt(1 + i));
		m_pCustomFuncArguments->addNew(pArgName).setPointer(&pEquation->getArgumentAt(1 + i));
		delete argObjs[i];
	}

	return pEquation->getArgumentAt(0);	// ������Ԃ�
}

void Parser::topRight(UnivType& obj)
{
	SentenceContext context(m_pSentenceContext);
	m_pSentenceContext->m_isInsideRight = true;

	CatContext context2(m_pCatContext);	// CatContext�́A�P��̃g�b�v���x���Ăяo���ɂ��A1�x����context�؂�ւ����Ȃ�

	// obj�ɒ�`���e��ݒ肷�邽�߂ɁA��U�萔��������������
	bool isConstant = obj.isConstant();
	obj.setConstant(false);

	branch(obj);

	// �萔���������̂悤�ɐݒ肵�Ȃ���
	obj.setConstant(isConstant);
}

// �X�N���v�g�s�̉E�ӎ��g�b�v�́A�����̃Z�~�R������rowCat�ƊԈႦ�Ȃ����߂ɁAvector����n�߂�
// 2010/04/04 �����̃Z�~�R�����ƍ������Ȃ����߂ɁA�g�b�v���x������Z�~�R�����𗘗p�ł��Ȃ�����
void Parser::branch(UnivType& obj)
{
	if(checkNextToken(TK_IF)){
		Equation *pEquation = new Equation;
		obj.setEquation(pEquation);
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_IF : Equation::OP_IF_CONST);
		pEquation->setNumArguments(3);
		condition(pEquation->getArgumentAt(0));
		assertNextToken(TK_THEN);
		if(m_pBraceContext->m_braceLevel == 0){
			vector(pEquation->getArgumentAt(1));
		}else{
			matrix(pEquation->getArgumentAt(1));
		}
		UnivType *pNoMatch = &pEquation->getArgumentAt(2);
		while(!checkNextToken(TK_ELSE)){
			assertNextToken(TK_ELSIF);
			Equation *pChildEq = new Equation;
			pNoMatch->setEquation(pChildEq);
			pChildEq->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_IF : Equation::OP_IF_CONST);
			pChildEq->setNumArguments(3);
			condition(pChildEq->getArgumentAt(0));
			assertNextToken(TK_THEN);
			if(m_pBraceContext->m_braceLevel == 0){
				vector(pChildEq->getArgumentAt(1));
			}else{
				matrix(pChildEq->getArgumentAt(1));
			}
			pNoMatch = &pChildEq->getArgumentAt(2);
		}
		if(m_pBraceContext->m_braceLevel == 0){
			vector(*pNoMatch);
		}else{
			matrix(*pNoMatch);
		}
	}else if(checkNextToken(TK_SWITCH)){
		Equation *pEquation = new Equation;
		obj.setEquation(pEquation);
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_SWITCH : Equation::OP_SWITCH_CONST);
		matrix(pEquation->addSingleArgument());
		UnivType& default_val = pEquation->addSingleArgument();
		while(!checkNextToken(TK_OTHERWISE)){
			assertNextToken(TK_CASE);
			matrix(pEquation->addSingleArgument());
			assertNextToken(TK_THEN);
			branch(pEquation->addSingleArgument());
		}
		branch(default_val);
	}else{
		if(m_pBraceContext->m_braceLevel == 0){
			vector(obj);
		}else{
			matrix(obj);
		}
	}
}

void Parser::condition(UnivType& obj)
{
	expression(obj);
	enum TokenKind eKind = getNextToken();
	if(eKind != TK_CND_EQUAL &&
		eKind != TK_CND_NEQUAL &&
		eKind != TK_CND_LARGE &&
		eKind != TK_CND_SMALL &&
		eKind != TK_CND_ELARGE &&
		eKind != TK_CND_ESMALL){
		putBackToken();
		return;
	}

	if(m_pSentenceContext->m_isReference){
		throw CdbException(CDB_REFERENCE_TO_AROP);
	}

	Equation *pEquation = new Equation;
	pEquation->setNumArguments(2);
	pEquation->getArgumentAt(0).move(obj);
	obj.setEquation(pEquation);
	expression(pEquation->getArgumentAt(1));

	switch(eKind){
	case TK_CND_EQUAL:
		pEquation->setOperation(Equation::OP_EQUAL);
		break;
	case TK_CND_NEQUAL:
		pEquation->setOperation(Equation::OP_NEQUAL);
		break;
	case TK_CND_LARGE:
		pEquation->setOperation(Equation::OP_LARGE);
		break;
	case TK_CND_SMALL:
		pEquation->setOperation(Equation::OP_SMALL);
		break;
	case TK_CND_ELARGE:
		pEquation->setOperation(Equation::OP_ELARGE);
		break;
	case TK_CND_ESMALL:
		pEquation->setOperation(Equation::OP_ESMALL);
		break;
	default:
		assert(false);
	}
}

void Parser::matrix(UnivType& obj)
{
	m_pCatContext->reset();
	vector(obj);
	if(checkNextToken(TK_SEMICOLON)){
#if 0	// 1�̕����L�q�ʂ����Ȃ�������������������
		Equation *pEquation = new Equation;
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_CAT_ROW : Equation::OP_CAT_ROW_CONST);
		UnivType& arg0 = pEquation->addSingleArgument();
		arg0.move(obj);
		obj.setEquation(pEquation);
		UnivType& arg1 = pEquation->addSingleArgument();
		arg1 = m_pCatContext->getNested();
		do{
			m_pCatContext->reset();
			vector(pEquation->addSingleArgument());
			pEquation->addSingleArgument()->setBool(m_pCatContext->getNested());
		}while(checkNextToken(TK_SEMICOLON));
#else
		WrappedVector<UnivType *> elements;
		UnivType *pUnivType = new UnivType;
		pUnivType->move(obj);
		elements.push_back(pUnivType);
		pUnivType = new UnivType(m_pCatContext->getNested());
		elements.push_back(pUnivType);
		do{
			m_pCatContext->reset();
			pUnivType = new UnivType;
			vector(*pUnivType);
			elements.push_back(pUnivType);
			pUnivType = new UnivType(m_pCatContext->getNested());
			elements.push_back(pUnivType);
		}while(checkNextToken(TK_SEMICOLON));
		Equation *pEquation = new Equation;
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_CAT_ROW : Equation::OP_CAT_ROW_CONST);
		int n = elements.size();
		pEquation->setNumArguments(n);
		for(int i = 0; i < n; i++){
			pUnivType = elements.getAt(i);
			pEquation->getArgumentAt(i).move(*pUnivType);
			delete pUnivType;
		}
		obj.setEquation(pEquation);
#endif
	}
}

void Parser::vector(UnivType& obj)
{
	m_pCatContext->reset();
	condition(obj);
	if(checkNextToken(TK_COMMA)){
#if 0	// 1�̕����L�q�ʂ����Ȃ�������������������
		Equation *pEquation = new Equation;
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_CAT_COLUMN : Equation::OP_CAT_COLUMN_CONST);
		UnivType& arg0 = pEquation->addSingleArgument();
		arg0.move(obj);
		obj.setEquation(pEquation);
		UnivType& arg1 = pEquation->addSingleArgument();
		arg1 = m_pCatContext->getNested();
		do{
			m_pCatContext->setNested(false);
			condition(pEquation->addSingleArgument());
			pEquation->addSingleArgument()->setBool(m_pCatContext->getNested());
		}while(checkNextToken(TK_COMMA));
#else
		WrappedVector<UnivType *> elements;
		UnivType *pUnivType = new UnivType;
		pUnivType->move(obj);
		elements.push_back(pUnivType);
		pUnivType = new UnivType(m_pCatContext->getNested());
		elements.push_back(pUnivType);
		do{
			m_pCatContext->reset();
			pUnivType = new UnivType;
			condition(*pUnivType);
			elements.push_back(pUnivType);
			pUnivType = new UnivType(m_pCatContext->getNested());
			elements.push_back(pUnivType);
		}while(checkNextToken(TK_COMMA));
		Equation *pEquation = new Equation;
		pEquation->setOperation(m_pSentenceContext->m_isReference ? Equation::OP_CAT_COLUMN : Equation::OP_CAT_COLUMN_CONST);
		int n = elements.size();
		pEquation->setNumArguments(n);
		for(int i = 0; i < n; i++){
			pUnivType = elements.getAt(i);
			pEquation->getArgumentAt(i).move(*pUnivType);
			delete pUnivType;
		}
		obj.setEquation(pEquation);
#endif

		// vector�ɂ�Nested�����s�����ꍇ�ɂ́Amatrix�ɂ����Ă͎��s���Ȃ�
		m_pCatContext->reset();
	}
}

void Parser::expression(UnivType& obj)
{
	// �ŏ��̍��̓ǂݍ���
	m_pBraceContext->m_countTerm = 0;
	term(obj);

	enum TokenKind eKind = getNextToken();
	putBackToken();
	if((eKind == TK_PLUS) || (eKind == TK_MINUS)
	|| (eKind == TK_BIT_AND) || (eKind == TK_BIT_OR)/* || (eKind == TK_BIT_XOR)*/
	|| (eKind == TK_LOGICAL_AND) || (eKind == TK_LOGICAL_OR)/* || (eKind == TK_LOGICAL_XOR)*/){
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);
		}
		UnivType first;
		first.move(obj);
		expression2nd(obj, first);
	}
}

void Parser::expression2nd(UnivType& obj, UnivType& first)
{
	WrappedVector<UnivType *> elements;
	UnivType *pUnivType = new UnivType;
	pUnivType->move(first);
	elements.push_back(pUnivType);

	enum TokenKind eKind = getNextToken();
	enum Equation::EqnOperation eOperation;
	int nRestArgs = -1;	// �c���Ă�������̐��i-1�͖������j
	switch(eKind){
	case TK_PLUS:
		eOperation = Equation::OP_PLUS;
		break;
	case TK_MINUS:
		eOperation = Equation::OP_MINUS;
		nRestArgs = 1;
		break;
	case TK_BIT_AND:
		eOperation = Equation::OP_BIT_AND;
		break;
	case TK_BIT_OR:
		eOperation = Equation::OP_BIT_OR;
		break;
//	case TK_BIT_XOR:
//		eOperation = Equation::OP_BIT_XOR;
//		nRestArgs = 1;
//		break;
	case TK_LOGICAL_AND:
		eOperation = Equation::OP_LOGICAL_AND;
		break;
	case TK_LOGICAL_OR:
		eOperation = Equation::OP_LOGICAL_OR;
		break;
//	case TK_LOGICAL_XOR:
//		eOperation = Equation::OP_LOGICAL_XOR;
//		nRestArgs = 1;
//		break;
	default:
		eOperation = Equation::OP_INVALID;
	}

	enum TokenKind eNextKind;
	for(eNextKind = eKind; (eNextKind == eKind) && (nRestArgs != 0); eNextKind = getNextToken(), nRestArgs--){
		// 2�Ԗڈȍ~�̍��̓ǂݍ���
		m_pBraceContext->m_countTerm++;

		pUnivType = new UnivType;
		term(*pUnivType);
		elements.push_back(pUnivType);
	}
	putBackToken();
	int n = elements.size();
	Equation *pEquation = new Equation;
	pEquation->setOperation(eOperation);
	pEquation->setNumArguments(n);
	for(int i = 0; i < n; i++){
		pUnivType = elements.getAt(i);
		pEquation->getArgumentAt(i).move(*pUnivType);
		delete pUnivType;
	}
	if((eNextKind == TK_PLUS) || (eNextKind == TK_MINUS)
	|| (eNextKind == TK_BIT_AND) || (eNextKind == TK_BIT_OR)/* || (eNextKind == TK_BIT_XOR)*/
	|| (eNextKind == TK_LOGICAL_AND) || (eNextKind == TK_LOGICAL_OR)/* || (eNextKind == TK_LOGICAL_XOR)*/){
		first.setEquation(pEquation);
		expression2nd(obj, first);
	}else{
		obj.setEquation(pEquation);
	}
}

void Parser::term(UnivType& obj)
{
	// �ŏ��̊�{�v�f�̓ǂݍ���
	m_pBraceContext->m_countPrimitive = 0;
	unaryOp(obj);

	enum TokenKind eKind = getNextToken();
	putBackToken();
	if((eKind == TK_MULT) || (eKind == TK_DIV) || (eKind == TK_DOT_MULT) || (eKind == TK_DOT_DIV) || (eKind == TK_MOD)){
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);
		}
		UnivType first;
		first.move(obj);
		term2nd(obj, first);
	}
}

void Parser::term2nd(UnivType& obj, UnivType& first)
{
	WrappedVector<UnivType *> elements;
	UnivType *pUnivType = new UnivType;
	pUnivType->move(first);
	elements.push_back(pUnivType);

	enum TokenKind eKind = getNextToken();
	enum Equation::EqnOperation eOperation;
	int nRestArgs = -1;	// �c���Ă�������̐��i-1�͖������j
	switch(eKind){
	case TK_MULT:
		eOperation = Equation::OP_MULT;
		break;
	case TK_DIV:
		eOperation = Equation::OP_DIV;
		nRestArgs = 1;
		break;
	case TK_DOT_MULT:
		eOperation = Equation::OP_DOT_MULT;
		break;
	case TK_DOT_DIV:
		eOperation = Equation::OP_DOT_DIV;
		nRestArgs = 1;
		break;
	case TK_MOD:
		eOperation = Equation::OP_MOD;
		nRestArgs = 1;
		break;
	default:
		eOperation = Equation::OP_INVALID;
	}

	enum TokenKind eNextKind;
	for(eNextKind = eKind; (eNextKind == eKind) && (nRestArgs != 0); eNextKind = getNextToken(), nRestArgs--){
		// 2�Ԗڈȍ~�̊�{�v�f�̓ǂݍ���
		m_pBraceContext->m_countPrimitive++;

		pUnivType = new UnivType;
		unaryOp(*pUnivType);
		elements.push_back(pUnivType);
	}
	putBackToken();
	int n = elements.size();
	Equation *pEquation = new Equation;
	pEquation->setOperation(eOperation);
	pEquation->setNumArguments(n);
	for(int i = 0; i < n; i++){
		pUnivType = elements.getAt(i);
		pEquation->getArgumentAt(i).move(*pUnivType);
		delete pUnivType;
	}
	if((eNextKind == TK_MULT) || (eNextKind == TK_DIV) || (eNextKind == TK_DOT_MULT) || (eNextKind == TK_DOT_DIV) || (eNextKind == TK_MOD)){
		if(eKind == TK_MOD) throw CdbException(CDB_VAGUE_PRIORITY, "%");	// %�̌�ɑ���term�̉��Z�������Ă͂Ȃ�Ȃ�
		first.setEquation(pEquation);
		term2nd(obj, first);
	}else{
		obj.setEquation(pEquation);
	}
}

void Parser::unaryOp(UnivType& obj)
{
	enum TokenKind eKind = getNextToken();
	if((eKind == TK_LOGICAL_NOT) || (eKind == TK_BIT_NOT)){
		Equation *pEquation = new Equation;
		pEquation->setOperation((eKind == TK_LOGICAL_NOT) ? Equation::OP_LOGICAL_NOT : Equation::OP_BIT_NOT);
		pEquation->setNumArguments(1);
		subsetAccess(pEquation->getArgumentAt(0));
		obj.setEquation(pEquation);
		return;
	}
	putBackToken();
	subsetAccess(obj);

	eKind = getNextToken();
	if(eKind == TK_DOT_APOS){
		// �s��̓]�u
		UnivType &arg = m_pSubsetContext->addNew();
		arg.move(obj);
		IndexConv *pIndexConv = new IndexConv;
		pIndexConv->setTranspose(true);
		if(m_pSentenceContext->m_isReference){
			obj.setFullReference(&arg, pIndexConv);
		}else{
			obj.setConstReference(&arg, pIndexConv);
		}	

	}else if(eKind == TK_APOS){
		// �s��̋���
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);
		}
		UnivType &arg = m_pSubsetContext->addNew();
		arg.move(obj);
		IndexConv *pIndexConv = new IndexConv;
		pIndexConv->setTranspose(true);

		Equation *pEquation = new Equation;
		pEquation->setOperation(Equation::OP_CONJ);
		pEquation->setNumArguments(1);
		if(m_pSentenceContext->m_isReference){
			pEquation->getArgumentAt(0).setFullReference(&arg, pIndexConv);
		}else{
			pEquation->getArgumentAt(0).setConstReference(&arg, pIndexConv);
		}
		obj.setEquation(pEquation);

	}else if(eKind == TK_POWER){
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);
		}
		UnivType &arg1 = m_pSubsetContext->addNew();
		UnivType &arg2 = m_pSubsetContext->addNew();
		arg1.move(obj);
		subsetAccess(arg2);

		Equation *pEquation = new Equation;
		pEquation->setOperation(Equation::OP_POWER);
		pEquation->setNumArguments(2);
		pEquation->getArgumentAt(0).setConstReference(&arg1);
		pEquation->getArgumentAt(1).setConstReference(&arg2);

		obj.setEquation(pEquation);

	}else if(eKind == TK_DOT_POWER){
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);
		}
		UnivType &arg1 = m_pSubsetContext->addNew();
		UnivType &arg2 = m_pSubsetContext->addNew();
		arg1.move(obj);
		subsetAccess(arg2);

		Equation *pEquation = new Equation;
		pEquation->setOperation(Equation::OP_DOT_POWER);
		pEquation->setNumArguments(2);
		pEquation->getArgumentAt(0).setConstReference(&arg1);
		pEquation->getArgumentAt(1).setConstReference(&arg2);

		obj.setEquation(pEquation);

	}else if(eKind == TK_COLON){
		// �����P�̓�������
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);	// TODO : �{����reference�ł���悤�ɂ����������󖳗��ł���i�֐��Ɠ������߁j
		}
		UnivType arg1, arg2;
		arg1.move(obj);
		{
			// �����t���̐������ʂȂ��ł��̂܂܏�����悤��
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_countTerm = 0;
			m_pBraceContext->m_countPrimitive = 0;

			subsetAccess(arg2);
		}

		Equation *pEquation = new Equation;
		pEquation->setOperation(Equation::OP_SERIES);
		obj.setEquation(pEquation);

		if(getNextToken() == TK_COLON){
			// ��������
			UnivType arg3;
			{
				// �����t���̐������ʂȂ��ł��̂܂܏�����悤��
				BraceContext context(m_pBraceContext);
				m_pBraceContext->m_countTerm = 0;
				m_pBraceContext->m_countPrimitive = 0;

				subsetAccess(arg3);
			}

			pEquation->setNumArguments(3);
			pEquation->getArgumentAt(0).move(arg1);
			pEquation->getArgumentAt(1).move(arg2);
			pEquation->getArgumentAt(2).move(arg3);
		}else{
			putBackToken();
			pEquation->setNumArguments(2);
			pEquation->getArgumentAt(0).move(arg1);
			pEquation->getArgumentAt(1).move(arg2);
		}
	}else{
		putBackToken();
	}
}

void Parser::subsetAccess(UnivType& obj)
{
	bool isRefSymbol = subsetDeclare(obj);

	// �T�u�Z�b�g�̃A�N�Z�X���g��Ȃ��ꍇ
	if(!checkNextToken(TK_DOT)){
		return;
	}

	const UnivType *pRefer = NULL;
	// primitive()�ɂ���ĕϐ��ɖ��ʂȎQ�Ƃ��Ȃ���Ă���ꍇ�́A�������Ē��ڂ��̎Q�Ɛ�ւ̎Q�Ƃ𒣂�
	if(isRefSymbol){
		assert(obj.isReference());
		// �Q�Ɛ�̃I�u�W�F�N�g���擾����
		IndexConv *pIndexConv;
		obj.getReference(pRefer, pIndexConv);

		if(pIndexConv == NULL){
			// obj�̎Q�Ƃ���������
			obj.releaseReference();
		}else{
			// �Ӗ��̂���Q�Ƃł��邽�߁A�����ɍēx�Q�Ƃ��쐬����悤�ɂ���
			pRefer = NULL;
		}

		isRefSymbol = false;	// �֐��Ăяo�����̓C���f�b�N�X�Q�ƂƂȂ邽�߁A�ϐ��ւ̒P���Q�ƂƂȂ�Ȃ�
	}
	// obj���̂��Q�Ɛ�Ƃ��邽�߂ɁAobj�̓��e���ړ����ĕۑ����Ă���
	if(pRefer == NULL){
		// �Q�Ƃ���鑤�ƂȂ�I�u�W�F�N�g��V�K�쐬�����R���e�i�Ɉړ�����
		UnivType& ReferredObj = m_pSubsetContext->addNew();
		ReferredObj.move(obj);
		pRefer = &ReferredObj;
	}

	// �T�u�Z�b�g��؂�ւ��čċA�Ăяo��
	{
		SubsetContext context(m_pSubsetContext);
		m_pSubsetContext->disableOwnerSearch();	// �u.�v�ɂ��A�N�Z�X�ł͐e�����͍s��Ȃ�
		m_pSubsetContext->setSubset(const_cast<UnivType *>(pRefer));	// �yTODO�zsetSubset()������const���͂����Ȃ��Ă͂����Ȃ��̂��v����
		subsetAccess(obj);
	}
}

bool Parser::subsetDeclare(UnivType& obj)
{
	// subset�L�[���[�h�łȂ��ꍇ
	if(!checkNextToken(TK_SUBSET)){
		return func(obj);
	}

	// subset�u���b�N�̊J�n { �����Ă��邱��
	assertNextToken(TK_BRACE_OPEN);

	// �T�u�Z�b�g���쐬���A����ɐ؂�ւ���
	SubsetContext context(m_pSubsetContext);
	m_pSubsetContext->createSubset(&obj);

	// } ������܂ŉ�͂��s��
	enum TokenKind eKind;
	while(eKind = m_token.getNext(), eKind != TK_BRACE_CLOSE){
		if(eKind == TK_EMPTY){
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		m_token.putBack();
		topScript();
	}

	return false;
}

// �߂�l�Fobj�����O�t���ϐ��ւ̒P���Q�Ƃł��邩�ǂ���
bool Parser::func(UnivType& obj)
{
	// �g�ݍ��݊֐��̔���
	// ���̃g�[�N����TK_SYMBOL���A���̎��� ( �ł���ꍇ�ɁA�V���{�����g�ݍ��݊֐����̂ł��邩���m���߂�

	if(getNextToken() == TK_SYMBOL){
		UnivType name;
		getCurrentTokenContent(name);
		const char *pName = (const char *)name;
		if(checkNextToken(TK_BRACKET_RND_OPEN)){
			Equation::EqnOperation eOp = Equation::findEmbeddedFuncByName(pName);
			if(eOp != Equation::OP_INVALID){
				// �g�ݍ��݊֐��ł���ꍇ
				if(m_pSentenceContext->m_isReference){
					throw CdbException(CDB_REFERENCE_TO_FUNC);
				}

				// �g�ݍ��݊֐��Ăяo���I�u�W�F�N�g���쐬
				Equation *pEquation = new Equation;
				obj.setEquation(pEquation);
				pEquation->setOperation(eOp);

				// �����̒ǉ�
				if(!checkNextToken(TK_BRACKET_RND_CLOSE)){
					do{
						// ��������͂��Ēǉ�
						UnivType& arg = pEquation->addSingleArgument();
						condition(arg);
					}while(checkNextToken(TK_COMMA));
					assertNextToken(TK_BRACKET_RND_CLOSE);	// ���ʂ����Ă��邱��
				}

				// �����I�Ɉ�����ǉ�����K�v��������ʂȊ֐��ł���ꍇ
				if(Equation::isRequireHiddenArguments(eOp)){
					UnivType& arg = pEquation->addSingleArgument();
					arg.move(m_pSubsetContext->getSubset());
				}

				return false;
			}else{
				putBackToken();
			}
		}
	}

	putBackToken();

	// �g�ݍ��݊֐��łȂ������ꍇ
	bool isRefSymbol = primitive(obj);

	enum TokenKind eKind;
	while((eKind = getNextToken()) != TK_EMPTY){

		// ���[�U�[��`�֐��ł��z��C���f�b�N�X�ł��Ȃ��ꍇ
		if((eKind != TK_BRACKET_RND_OPEN) && (eKind != TK_BRACKET_SQR_OPEN)){
			putBackToken();
			break;
		}

		const UnivType *pRefer = NULL;
		// primitive()�ɂ���ĕϐ��ɖ��ʂȎQ�Ƃ��Ȃ���Ă���ꍇ�́A�������Ē��ڂ��̎Q�Ɛ�ւ̎Q�Ƃ𒣂�
		if(isRefSymbol){
			assert(obj.isReference());
			// �Q�Ɛ�̃I�u�W�F�N�g���擾����
			IndexConv *pIndexConv;
			obj.getReference(pRefer, pIndexConv);

			if(pIndexConv == NULL){
				// obj�̎Q�Ƃ���������
				obj.releaseReference();
			}else{
				// �Ӗ��̂���Q�Ƃł��邽�߁A�����ɍēx�Q�Ƃ��쐬����悤�ɂ���
				pRefer = NULL;
			}

			isRefSymbol = false;	// �֐��Ăяo�����̓C���f�b�N�X�Q�ƂƂȂ邽�߁A�ϐ��ւ̒P���Q�ƂƂȂ�Ȃ�
		}
		// obj���̂��Q�Ɛ�Ƃ��邽�߂ɁAobj�̓��e���ړ����ĕۑ����Ă���
		if(pRefer == NULL){
			// �Q�Ƃ���鑤�ƂȂ�I�u�W�F�N�g��V�K�쐬�����R���e�i�Ɉړ�����
			UnivType& ReferredObj = m_pSubsetContext->addNew();
			ReferredObj.move(obj);
			pRefer = &ReferredObj;
		}

		// ���[�U�[��`�֐��̌Ăяo���ł���ꍇ
		if(eKind == TK_BRACKET_RND_OPEN){
			if(m_pSentenceContext->m_isReference){
				throw CdbException(CDB_REFERENCE_TO_FUNC);
			}

			// �J�X�^���֐��Ăяo���I�u�W�F�N�g���쐬
			Equation *pEquation = new Equation;
			obj.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_CUSTOM_FUNC_CALL);

			// �ŏ��̈����́A�J�X�^���֐��I�u�W�F�N�g�ł���
			UnivType& custom_func = pEquation->addSingleArgument();
			custom_func.setConstReference(pRefer);

			// �����̒ǉ�
			if(!checkNextToken(TK_BRACKET_RND_CLOSE)){
				do{
					// ��������͂��Ēǉ�
					UnivType& arg = pEquation->addSingleArgument();
					condition(arg);
				}while(checkNextToken(TK_COMMA));
				assertNextToken(TK_BRACKET_RND_CLOSE);	// ���ʂ����Ă��邱��
			}
		}

		// �͈͎w��C���f�b�N�X�t���Q�Ƃł���ꍇ
		if(eKind == TK_BRACKET_SQR_OPEN){

			// IndexConv�I�u�W�F�N�g���쐬
			IndexConv *pIndexConv = new IndexConv;

			// �ŏ��̈����i�s�C���f�b�N�X�w��A���̓x�N�g���w��j
			arrayIndexRight(pIndexConv, pRefer, DIM_VEC);

			// �R���}������΁A��C���f�b�N�X�w���ǂݍ���
			if(checkNextToken(TK_COMMA)){
				arrayIndexRight(pIndexConv, pRefer, DIM_COL);
				pIndexConv->setVectorAccess(false);
			}else{
				// �R���}���Ȃ���΃x�N�g���w��
				pIndexConv->setVectorAccess(true);
			}

			// �C���f�b�N�X�t���̕ϐ��Q��
			if(m_pSentenceContext->m_isReference){
				obj.setFullReference(const_cast<UnivType *>(pRefer), pIndexConv);
			}else{
				obj.setConstReference(pRefer, pIndexConv);
			}

			assertNextToken(TK_BRACKET_SQR_CLOSE);	// ���ʂ����Ă��邱��	
		}
	}

	return isRefSymbol;
}

// �߂�l�Fobj�����O�t���ϐ��ւ̒P���Q�Ƃł��邩�ǂ���
bool Parser::primitive(UnivType& obj)
{
	enum TokenKind eKind = getNextToken();
	switch(eKind){
	case TK_STRING:
	case TK_INT:
	case TK_UINT:
	case TK_REAL:
	case TK_POINTER:
	case TK_BINARY:
		getCurrentTokenContent(obj);
		break;
	case TK_BOOL_TRUE:
		obj = true;
		break;
	case TK_BOOL_FALSE:
		obj = false;
		break;
	case TK_BEGIN:
		// TK_BEGIN, TK_END�͔z��C���f�b�N�X���݂̂ŗL��
		if(m_pIndexContext && m_pIndexContext->m_isInsideArrayIndex && m_pSentenceContext->m_isInsideRight){	// ���̊֐��ł́A�E�ӎ����݂̂���舵��
			// �v�f�̊J�n�C���f�b�N�X�͕K���[���ł���
			obj = 0;
		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_END:
		// TK_BEGIN, TK_END�͔z��C���f�b�N�X���݂̂ŗL��
		if(m_pIndexContext && m_pIndexContext->m_isInsideArrayIndex && m_pSentenceContext->m_isInsideRight){	// ���̊֐��ł́A�E�ӎ����݂̂���舵��
			// �v�f�����擾���鐔���I�u�W�F�N�g�𐶐�

			// �����f�N�������g���邽�߂̐������쐬
			Equation *pEquation = new Equation;
			obj.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_MINUS);
			pEquation->setNumArguments(2);
			pEquation->getArgumentAt(1) = 1;
			UnivType& NumElements = pEquation->getArgumentAt(0);

			// �v�f�����擾���鐔�����쐬
			pEquation = new Equation;
			NumElements.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_ARRAY_SIZE);
			pEquation->setNumArguments(3);
			pEquation->getArgumentAt(0).setConstReference(m_pIndexContext->m_pTargetArray);	// �T�C�Y�擾�Ώۂ̕ϐ�
			pEquation->getArgumentAt(1) = m_pIndexContext->m_arrayDimensionId;
			pEquation->getArgumentAt(2) = m_pIndexContext->m_pIndexConv->getVectorAccess();	// �x�N�g���A�N�Z�X���ǂ�����n��

		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_BRACKET_RND_OPEN:
		if((m_pBraceContext->m_currentBrace != TK_INVALID) && (m_pBraceContext->m_currentBrace != TK_BRACKET_RND_OPEN)){
			// �َ�̊��ʂň͂܂�Ă����ꍇ
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACKET_RND_CLOSE)){
			// �󃊃X�g
			obj.clear();
		}else{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_braceLevel++;
			m_pBraceContext->m_currentBrace = TK_BRACKET_RND_OPEN;
			topRight(obj);
		}
		assertNextToken(TK_BRACKET_RND_CLOSE);
		break;
	case TK_BRACKET_SQR_OPEN:
		if((m_pBraceContext->m_currentBrace != TK_INVALID) && (m_pBraceContext->m_currentBrace != TK_BRACKET_SQR_OPEN)){
			// �َ�̊��ʂň͂܂�Ă����ꍇ
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACKET_SQR_CLOSE)){
			// �󃊃X�g
			obj.clear();
		}else{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_braceLevel++;
			m_pBraceContext->m_currentBrace = TK_BRACKET_SQR_OPEN;
			topRight(obj);
		}
		assertNextToken(TK_BRACKET_SQR_CLOSE);
		break;
	case TK_BRACE_OPEN:
		if((m_pBraceContext->m_currentBrace != TK_INVALID) && (m_pBraceContext->m_currentBrace != TK_BRACE_OPEN)){
			// �َ�̊��ʂň͂܂�Ă����ꍇ
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACE_CLOSE)){
			// �󃊃X�g
			obj.clear();
		}else{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_braceLevel++;
			m_pBraceContext->m_currentBrace = TK_BRACE_OPEN;
			topRight(obj);
		}
		assertNextToken(TK_BRACE_CLOSE);
		break;
	case TK_SCALAR_EQN:
		// �X�J���[�]�����ł́A�X�J���[�l�����]���ł��Ȃ�
		expression(obj);
		assertNextToken(TK_SCALAR_EQN);
		break;
	case TK_PLUS:
		// �P���́{ (�ŏ��̍��̍ŏ��̊�{�v�f�ł̂ݗL��)
		if((m_pBraceContext->m_countTerm == 0) && (m_pBraceContext->m_countPrimitive == 0)){
			m_pBraceContext->m_countTerm = INVALID;	// �P���{���A�����Ďg���Ȃ��悤�ɂ���

			// ��{�I�ɁA�������Ȃ�
			term(obj);

			m_pBraceContext->m_countTerm = 0;
		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_MINUS:
		// �P���́| (�ŏ��̍��̍ŏ��̊�{�v�f�ł̂ݗL��)
		if((m_pBraceContext->m_countTerm == 0) && (m_pBraceContext->m_countPrimitive == 0)){
			m_pBraceContext->m_countTerm = INVALID;	// �P���{���A�����Ďg���Ȃ��悤�ɂ���

			if(m_pSentenceContext->m_isReference){
				throw CdbException(CDB_REFERENCE_TO_AROP);
			}

			Equation *pEquation = new Equation;
			pEquation->setOperation(Equation::OP_UNARY_MINUS);
			pEquation->setNumArguments(1);
			term(pEquation->getArgumentAt(0));
			obj.setEquation(pEquation);

			m_pBraceContext->m_countTerm = 0;
		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_SYMBOL:
		{
			UnivType name;
			getCurrentTokenContent(name);
			const char *pName = (const char *)name;

			// �ʏ�̕ϐ��Q��
			UnivType& NamedObj = getObjectByName(pName);
			if(m_pSentenceContext->m_isReference){
				obj.setFullReference(&NamedObj);
			}else{
				obj.setConstReference(&NamedObj);
			}
		}
		break;
	default:
		throw CdbException(CDB_SYNTAX_ERROR);
	}

	return eKind == TK_SYMBOL;	// �g�[�N�����V���{���ł���΁Aobj�͖��O�t���ϐ��ւ̒P���Q�ƂƂȂ�
}

void Parser::matrixLeft(UnivType& obj)
{
	if(m_pBraceContext->m_currentBrace == TK_INVALID){
		vectorLeft(obj);
	}else{
		UnivType *pLastObj;
		IndexConv *pLastIndexConv;
		int nRow = 0;
		do{
			IndexConv *pIndexConv = new IndexConv;
			pIndexConv->setConvTypeMatrix(DIM_ROW, ICT_TABLE_SINGLE);
			pIndexConv->getIndexTableMatrix(DIM_ROW) = nRow;
			UnivType& childObj = m_pSubsetContext->addNew();
			if(m_pSentenceContext->m_isReference){
				childObj.setFullReference(&obj, pIndexConv);
			}else{
				childObj.setConstReference(&obj, pIndexConv);
			}
			vectorLeft(childObj);
			pLastObj = &childObj;
			pLastIndexConv = pIndexConv;
			nRow++;
		}while(checkNextToken(TK_SEMICOLON));

		if(nRow == 1){
			// �v�f���ŏI�I�ɂP�����Ȃ������ꍇ�A��Ɋւ���v�f�w��𖳌��Ƃ���
			if(m_pSentenceContext->m_isReference){
				pLastObj->releaseReference();	// �Q�Ɖ����AIndexConv�I�u�W�F�N�g�폜
				pLastObj->setFullReference(&obj);
			}else{
				// const���ݒ肳��Ă����炢�������������
				bool isConstant = pLastObj->isConstant();
				pLastObj->setConstant(false);

				pLastObj->setConstReference(&obj);

				// const��ݒ肵�Ȃ���
				pLastObj->setConstant(isConstant);
			}
		}else{
			// �Ō�̗v�f���ŏI�v�f�܂łƂ���
			pLastIndexConv->setConvTypeMatrix(DIM_ROW, ICT_SERIES);
			pLastIndexConv->getIndexSeriesBeginMatrix(DIM_ROW) = nRow - 1;
			pLastIndexConv->getIndexSeriesEndMatrix(DIM_ROW) = INDEX_END;
		}
	}
}

void Parser::vectorLeft(UnivType& obj)
{
	if(m_pBraceContext->m_currentBrace == TK_INVALID){
		primitiveLeft(obj);
	}else{
		UnivType *pLastObj;
		IndexConv *pLastIndexConv;
		int nCol = 0;
		do{
			IndexConv *pIndexConv = new IndexConv;
			pIndexConv->setConvTypeMatrix(DIM_COL, ICT_TABLE_SINGLE);
			pIndexConv->getIndexTableMatrix(DIM_COL) = nCol;
			UnivType& childObj = m_pSubsetContext->addNew();
			if(m_pSentenceContext->m_isReference){
				childObj.setFullReference(&obj, pIndexConv);
			}else{
				childObj.setConstReference(&obj, pIndexConv);
			}
			primitiveLeft(childObj);
			pLastObj = &childObj;
			pLastIndexConv = pIndexConv;
			nCol++;
		}while(checkNextToken(TK_COMMA));

		if(nCol == 1){
			// �v�f���ŏI�I�ɂP�����Ȃ������ꍇ�A�s�Ɋւ���v�f�w��𖳌��Ƃ���
			if(m_pSentenceContext->m_isReference){
				pLastObj->releaseReference();	// �Q�Ɖ����AIndexConv�I�u�W�F�N�g�폜
				pLastObj->setFullReference(&obj);
			}else{
				// const���ݒ肳��Ă����炢�������������
				bool isConstant = pLastObj->isConstant();
				pLastObj->setConstant(false);

				pLastObj->setConstReference(&obj);

				// const��ݒ肵�Ȃ���
				pLastObj->setConstant(isConstant);
			}
		}else{
			// �Ō�̗v�f���ŏI�v�f�܂łƂ���
			pLastIndexConv->setConvTypeMatrix(DIM_COL, ICT_SERIES);
			pLastIndexConv->getIndexSeriesBeginMatrix(DIM_COL) = nCol - 1;
			pLastIndexConv->getIndexSeriesEndMatrix(DIM_COL) = INDEX_END;
		}
	}
}

void Parser::primitiveLeft(UnivType& obj)
{
	enum TokenKind eKind = getNextToken();
	switch(eKind){
	case TK_BRACKET_RND_OPEN:
		// ����A����̊��ʂł��l�X�g�Q�ƈ����Ƃ���
		{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_currentBrace = TK_BRACKET_RND_OPEN;
			matrixLeft(obj);
		}
		assertNextToken(TK_BRACKET_RND_CLOSE);
		break;
	case TK_BRACKET_SQR_OPEN:
		// ����A����̊��ʂł��l�X�g�Q�ƈ����Ƃ���
		{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_currentBrace = TK_BRACKET_SQR_OPEN;
			matrixLeft(obj);
		}
		assertNextToken(TK_BRACKET_SQR_CLOSE);
		break;
	case TK_BRACE_OPEN:
		// ����A����̊��ʂł��l�X�g�Q�ƈ����Ƃ���
		{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_currentBrace = TK_BRACE_OPEN;
			matrixLeft(obj);
		}
		assertNextToken(TK_BRACE_CLOSE);
		break;
	case TK_SYMBOL:
		{
			UnivType name;
			getCurrentTokenContent(name);
			const char *pName = (const char *)name;
			if(checkNextToken(TK_BRACKET_RND_OPEN)){
				// �͈͎w��C���f�b�N�X�t��
				UnivType *pNamedObj;
				Equation *pEquation = NULL;

				try{
					UnivType& NamedObj = m_pSubsetContext->getUnsafeUnivType(pName);
					if(!NamedObj.isEquation()) throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
					pEquation = NamedObj.getEquation();
					pNamedObj = &NamedObj;
				}
				catch(CdbException& e){
					// �o�^��������Ȃ��ȊO�̃G���[�͓����Ȃ���
					if(e.getErrorCode() != CDB_NOTFOUND) throw e;

					// ������V�K�쐬
					CDB_ERR error;
					UnivType& NewNamedObj = m_pSubsetContext->addNew(pName, error);
					if(error != CDB_OK) throw CdbException(error);
					pEquation = new Equation;
					NewNamedObj.setEquation(pEquation);
					pEquation->setOperation(Equation::OP_CAT_PARTIAL_REF);
					pNamedObj = &NewNamedObj;
				}

				// IndexConv�I�u�W�F�N�g���쐬
				IndexConv *pIndexConv = new IndexConv;

				// �ŏ��̈����i�s�C���f�b�N�X�w��A���̓x�N�g���w��j
				arrayIndexLeft(pIndexConv, pNamedObj, DIM_VEC);

				// �R���}������΁A��C���f�b�N�X�w���ǂݍ���
				if(checkNextToken(TK_COMMA)){
					arrayIndexLeft(pIndexConv, pNamedObj, DIM_COL);
					pIndexConv->setVectorAccess(false);
				}else{
					// �R���}���Ȃ���΃x�N�g���w��
					pIndexConv->setVectorAccess(true);
				}

				assertNextToken(TK_BRACKET_RND_CLOSE);

				// ��`����ǉ�
				// �y����z�����ɂ����Đ����̈����ɑ΂���const�Q�Ɩ���full�Q�ƂŒǉ����Ă����ƁA
				// Equation::OP_CAT_PARTIAL_REF�������I�ɎQ�ƃ^�C�v�𔻒肵�āA�v�f�Ԃ̎Q�Ƃ��쐬����
				// �y���Ӂz�t�ɁAEquation::OP_CAT_PARTIAL_REF�̈����ɑ΂��ẮA�K���Q�Ƃ�ݒ肵�Ȃ���΂Ȃ�Ȃ�
				UnivType& newDef = pEquation->addSingleArgument();
				if(m_pSentenceContext->m_isReference){
					newDef.setFullReference(&obj, pIndexConv);
				}else{
					newDef.setConstReference(&obj, pIndexConv);
				}
			}else{
				// �͈͎w��C���f�b�N�X�Ȃ�
				CDB_ERR error = m_pSubsetContext->rename(&obj, pName);
				if(error != CDB_OK){
					if(error == CDB_KEY_COLLISION){
						// ���ɓo�^����Ă���ϐ�������`��������
						UnivType& temp = m_pSubsetContext->getUnsafeUnivType(pName);
						if(!temp.isValid()){
							temp.setFullReference(&obj);	// TODO : invalid�Ȗ��O���������āAobj�����l�[�����Ă�����
							// �萔������t��
							temp.setConstant(m_pSentenceContext->m_isConstant);
							obj.setConstant(m_pSentenceContext->m_isConstant);	// 20110123 �t���Q�Ƃł͒萔�����͎Q�Ɛ�ɏ]������ 
						}else{
							// ���ɓ����̂Œ�`����Ă�����
							throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
						}
					}else{
						assert(false);
					}
				}else{
					// �萔������t��
					obj.setConstant(m_pSentenceContext->m_isConstant);
				}
			}
		}
		break;
	default:
		throw CdbException(CDB_SYNTAX_ERROR);
	}
}

void Parser::arrayIndexRight(IndexConv *pIndexConv, const UnivType *pArray, int nDim)
{
	// �R���e�L�X�g�̐؂�ւ�
	IndexContext context(m_pIndexContext);
	m_pIndexContext->m_isInsideArrayIndex = true;
	m_pIndexContext->m_pIndexConv = pIndexConv;
	m_pIndexContext->m_pTargetArray = pArray;
	m_pIndexContext->m_arrayDimensionId = nDim;
	assert(m_pSentenceContext->m_isInsideRight == true);

	enum TokenKind eKind = getNextToken();
	if(eKind == TK_COLON){
		pIndexConv->setConvTypeMatrix(nDim, ICT_NONE);
		return;
	}else{
		putBackToken();
	}

	UnivType& begin = pIndexConv->getIndexSeriesBeginMatrix(nDim);
	UnivType& delta = pIndexConv->getIndexSeriesDeltaMatrix(nDim);
	UnivType& end = pIndexConv->getIndexSeriesEndMatrix(nDim);
	m_pIndexContext->saveCurrentTokenPos(m_token);
	primitive(begin);
	eKind = getNextToken();
	if(eKind == TK_COLON){	// �J�n�C���f�b�N�X�ƏI���C���f�b�N�X�ɂ���Ԏw��
		primitive(end);
		eKind = getNextToken();
		if(eKind == TK_COLON){	// ���Ԋu�C���f�b�N�X�w��
			delta.move(end);
			primitive(end);
		}else{
			putBackToken();
		}
		pIndexConv->setConvTypeMatrix(nDim, ICT_SERIES);
		return;
	}

	m_pIndexContext->loadTokenPos(m_token);
	expression(pIndexConv->getIndexTableMatrix(nDim));
	pIndexConv->setConvTypeMatrix(nDim, ICT_TABLE);
}

void Parser::arrayIndexLeft(IndexConv *pIndexConv, const UnivType *pArray, int nDim)
{
	// �R���e�L�X�g�̐؂�ւ�
	IndexContext context(m_pIndexContext);
	m_pIndexContext->m_isInsideArrayIndex = true;
	m_pIndexContext->m_pIndexConv = pIndexConv;
	m_pIndexContext->m_pTargetArray = pArray;
	m_pIndexContext->m_arrayDimensionId = nDim;
	assert(m_pSentenceContext->m_isInsideRight == false);

	enum TokenKind eKind = getNextToken();
	if(eKind == TK_COLON){
		pIndexConv->setConvTypeMatrix(nDim, ICT_NONE);
		return;
	}else{
		putBackToken();
	}

	UnivType& begin = pIndexConv->getIndexSeriesBeginMatrix(nDim);
	UnivType& delta = pIndexConv->getIndexSeriesDeltaMatrix(nDim);
	UnivType& end = pIndexConv->getIndexSeriesEndMatrix(nDim);
	m_pIndexContext->saveCurrentTokenPos(m_token);
	eKind = getNextToken();
	// ���ӎ��ɂ����ẮAbegin, end�����p�����ꍇ�ɂ́A�����ɒP�̂ő��݂��Ă��Ȃ��Ă͂Ȃ�Ȃ����߁A
	// ���̊֐����ŉ�͂��s���Aprimitive()���ő��݂��Ă��Ă��G���[�Ƃ���B (end - 1)�Ȃǂ̋L�q�͂ł��Ȃ��B
	switch(eKind){
	case TK_BEGIN:
		begin = INDEX_BEGIN;
		break;
	case TK_END:
		begin = INDEX_END;
		break;
	default:
		putBackToken();
		primitive(begin);
	}
	eKind = getNextToken();
	if(eKind == TK_COLON){	// �J�n�C���f�b�N�X�ƏI���C���f�b�N�X�ɂ���Ԏw��
		eKind = getNextToken();
		switch(eKind){
		case TK_BEGIN:
			end = INDEX_BEGIN;
			break;
		case TK_END:
			end = INDEX_END;
			break;
		default:
			putBackToken();
			primitive(end);
		}
		eKind = getNextToken();
		if(eKind == TK_COLON){	// ���Ԋu�C���f�b�N�X�w��
			delta.move(end);
			eKind = getNextToken();
			switch(eKind){
			case TK_BEGIN:
				end = INDEX_BEGIN;
				break;
			case TK_END:
				end = INDEX_END;
				break;
			default:
				putBackToken();
				primitive(end);
			}
		}
		pIndexConv->setConvTypeMatrix(nDim, ICT_SERIES);
		return;
	}

	m_pIndexContext->loadTokenPos(m_token);
	expression(pIndexConv->getIndexTableMatrix(nDim));
	pIndexConv->setConvTypeMatrix(nDim, ICT_TABLE);
}
