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
	m_pSubsetContext->setRootDB(&m_RootDB);	// 検索ルートDB（グローバル名前空間）の設定

	try{
		// トークンが空になるまで解析を行う
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
	m_pSubsetContext->setRootDB(&m_RootDB);	// 検索ルートDB（グローバル名前空間）の設定

	try{
		topRight(obj);
	}
	catch(CdbException& e){
		e.addFileLine(m_token.getCurrentLine());
		throw e;
	}

	// TODO : 120117 以前の評価結果が上書きされるときに、そこでしか利用されていなかった名前なしオブジェクトがゴミとして残る問題がある
	// どこからも参照されてないものは削除する機能が必要
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

// スクリプト解析トップ
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

// サブセット定義
void Parser::subsetDefine()
{
	// この時点でsubsetキーワードの次のトークンの状態になっていること

	// 次はサブセット名称を示す文字列でなければならない
	enum TokenKind eKind = getNextToken();
	if(eKind != TK_SYMBOL){
		throw CdbException(CDB_SYNTAX_ERROR);
	}
	UnivType SubsetName;
	getCurrentTokenContent(SubsetName);

	// サブセットを作成し、それに切り替える
	SubsetContext context(m_pSubsetContext);
	m_pSubsetContext->createSubset(SubsetName);

	// 続いて . が来た場合は、次の階層のサブセットを定義する
	if(checkNextToken(TK_DOT)){
		subsetDefine();
	}else{
		// 引き続いて { が来た場合のみ、式の読み込みを行う
		if(checkNextToken(TK_BRACE_OPEN)){

			// } が来るまで解析を行う
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

// 式解析トップ
// 【注意】
// reference属性は、左辺・右辺の構築において参照される
// const属性は、左辺式側の構築においてのみ参照される（右辺では参照しない）
void Parser::topEquation()
{
	enum TokenKind eKind = getNextToken();
	switch(eKind){
	case TK_VARIABLE:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = false;
			m_pSentenceContext->m_isConstant = false;

			// 左辺式より参照オブジェクトを構築
			UnivType& obj = topLeft();
			// = が存在していること
			assertNextToken(TK_SUBST);
			// 右辺式オブジェクトを構築
			topRight(obj);
		}
		break;
	case TK_REFERENCE:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = true;
			m_pSentenceContext->m_isConstant = false;	// 参照において定数属性は関係ないが、とりあえずfalseとしておく

			// 左辺式より参照オブジェクトを構築
			UnivType& obj = topLeft();
			// = が存在していること
			assertNextToken(TK_SUBST);
			// 右辺式オブジェクトを構築
			topRight(obj);

#if 0	// 090805 実際にどう実装するのかは未定であるが、結果算出型の数式に対してのreferenceではエラーを出したい
			// 実際に変数へのリファレンスになっていなかったらエラー
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
			m_pSentenceContext->m_isConstant = true;	// TODO : const属性の影響範囲が明確でない

			// 引数を入れるためのコンテナを作成
			assert(m_pCustomFuncArguments == NULL);
			m_pCustomFuncArguments = new BaseDB;
			// 左辺式よりカスタム関数オブジェクトを構築
			UnivType& eqn = topFunc();
			// = が存在していること
			assertNextToken(TK_SUBST);
			// 右辺式オブジェクトを構築
			topRight(eqn);
			// 引数用コンテナを削除
			delete m_pCustomFuncArguments;
			m_pCustomFuncArguments = NULL;
		}
		break;
	default:	// 変数はデフォルトで定数扱いとする（実質的にconst修飾子に意味はない）
		putBackToken();
	case TK_CONST:
		{
			SentenceContext context(m_pSentenceContext);
			m_pSentenceContext->m_isReference = false;
			m_pSentenceContext->m_isConstant = true;

			// 左辺式より参照オブジェクトを構築
			UnivType& obj = topLeft();
			// = が存在していること
			assertNextToken(TK_SUBST);
			// 右辺式オブジェクトを構築
			topRight(obj);
		}
		break;
	}

	// 文末マーク（セミコロン）のチェック
	if(!checkNextToken(TK_SEMICOLON)){
		// 090806 文末のセミコロンは、なくてもエラーを表示しない
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
	
	// カスタム関数オブジェクトを作成
	UnivType& FuncObj = m_pSubsetContext->addNew((const char *)FuncName);
	Equation *pEquation = new Equation;
	pEquation->setOperation(Equation::OP_CUSTOM_FUNC);
	FuncObj.setEquation(pEquation);
	// 定数属性を付加
	FuncObj.setConstant(m_pSentenceContext->m_isConstant);
	
	assert(m_pCustomFuncArguments != NULL);
	
	// 仮引数をargObjsに登録する
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
	pEquation->setNumArguments(1 + nArgs);	// 最初の１引数は数式が入る

	// 引数変数ＤＢにカスタム関数オブジェクトの引数への参照を設定する
	for(int i = 0; i < nArgs; i++){
		const char *pArgName = argObjs[i]->getString();
		//m_pCustomFuncArguments->addNew(pArgName).setReference(&pEquation->getArgumentAt(1 + i));
		m_pCustomFuncArguments->addNew(pArgName).setPointer(&pEquation->getArgumentAt(1 + i));
		delete argObjs[i];
	}

	return pEquation->getArgumentAt(0);	// 数式を返す
}

void Parser::topRight(UnivType& obj)
{
	SentenceContext context(m_pSentenceContext);
	m_pSentenceContext->m_isInsideRight = true;

	CatContext context2(m_pCatContext);	// CatContextは、１回のトップレベル呼び出しにつき、1度しかcontext切り替えしない

	// objに定義内容を設定するために、一旦定数属性を解除する
	bool isConstant = obj.isConstant();
	obj.setConstant(false);

	branch(obj);

	// 定数属性を元のように設定しなおす
	obj.setConstant(isConstant);
}

// スクリプト行の右辺式トップは、文末のセミコロンをrowCatと間違えないために、vectorから始める
// 2010/04/04 文末のセミコロンと混同しないために、トップレベルからセミコロンを利用できなくする
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
#if 0	// 1の方が記述量が少ないがメモリ効率が悪い
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
#if 0	// 1の方が記述量が少ないがメモリ効率が悪い
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

		// vectorにてNestedを実行した場合には、matrixにおいては実行しない
		m_pCatContext->reset();
	}
}

void Parser::expression(UnivType& obj)
{
	// 最初の項の読み込み
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
	int nRestArgs = -1;	// 残っている引数の数（-1は無制限）
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
		// 2番目以降の項の読み込み
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
	// 最初の基本要素の読み込み
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
	int nRestArgs = -1;	// 残っている引数の数（-1は無制限）
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
		// 2番目以降の基本要素の読み込み
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
		if(eKind == TK_MOD) throw CdbException(CDB_VAGUE_PRIORITY, "%");	// %の後に続きtermの演算があってはならない
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
		// 行列の転置
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
		// 行列の共役
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
		// 等差１の等差数列
		if(m_pSentenceContext->m_isReference){
			throw CdbException(CDB_REFERENCE_TO_AROP);	// TODO : 本当はreferenceできるようにしたいが現状無理である（関数と同じため）
		}
		UnivType arg1, arg2;
		arg1.move(obj);
		{
			// 符号付きの数を括弧なしでそのまま書けるように
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_countTerm = 0;
			m_pBraceContext->m_countPrimitive = 0;

			subsetAccess(arg2);
		}

		Equation *pEquation = new Equation;
		pEquation->setOperation(Equation::OP_SERIES);
		obj.setEquation(pEquation);

		if(getNextToken() == TK_COLON){
			// 等差数列
			UnivType arg3;
			{
				// 符号付きの数を括弧なしでそのまま書けるように
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

	// サブセットのアクセスを使わない場合
	if(!checkNextToken(TK_DOT)){
		return;
	}

	const UnivType *pRefer = NULL;
	// primitive()によって変数に無駄な参照がなされている場合は、解除して直接その参照先への参照を張る
	if(isRefSymbol){
		assert(obj.isReference());
		// 参照先のオブジェクトを取得する
		IndexConv *pIndexConv;
		obj.getReference(pRefer, pIndexConv);

		if(pIndexConv == NULL){
			// objの参照を解除する
			obj.releaseReference();
		}else{
			// 意味のある参照であるため、そこに再度参照を作成するようにする
			pRefer = NULL;
		}

		isRefSymbol = false;	// 関数呼び出し又はインデックス参照となるため、変数への単純参照とならない
	}
	// obj自体を参照先とするために、objの内容を移動して保存しておく
	if(pRefer == NULL){
		// 参照される側となるオブジェクトを新規作成したコンテナに移動する
		UnivType& ReferredObj = m_pSubsetContext->addNew();
		ReferredObj.move(obj);
		pRefer = &ReferredObj;
	}

	// サブセットを切り替えて再帰呼び出し
	{
		SubsetContext context(m_pSubsetContext);
		m_pSubsetContext->disableOwnerSearch();	// 「.」によるアクセスでは親検索は行わない
		m_pSubsetContext->setSubset(const_cast<UnivType *>(pRefer));	// 【TODO】setSubset()内部でconstをはずさなくてはいけないのか要検討
		subsetAccess(obj);
	}
}

bool Parser::subsetDeclare(UnivType& obj)
{
	// subsetキーワードでない場合
	if(!checkNextToken(TK_SUBSET)){
		return func(obj);
	}

	// subsetブロックの開始 { が来ていること
	assertNextToken(TK_BRACE_OPEN);

	// サブセットを作成し、それに切り替える
	SubsetContext context(m_pSubsetContext);
	m_pSubsetContext->createSubset(&obj);

	// } が来るまで解析を行う
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

// 戻り値：objが名前付き変数への単純参照であるかどうか
bool Parser::func(UnivType& obj)
{
	// 組み込み関数の判定
	// 次のトークンがTK_SYMBOLかつ、その次が ( である場合に、シンボルが組み込み関数名称であるかを確かめる

	if(getNextToken() == TK_SYMBOL){
		UnivType name;
		getCurrentTokenContent(name);
		const char *pName = (const char *)name;
		if(checkNextToken(TK_BRACKET_RND_OPEN)){
			Equation::EqnOperation eOp = Equation::findEmbeddedFuncByName(pName);
			if(eOp != Equation::OP_INVALID){
				// 組み込み関数である場合
				if(m_pSentenceContext->m_isReference){
					throw CdbException(CDB_REFERENCE_TO_FUNC);
				}

				// 組み込み関数呼び出しオブジェクトを作成
				Equation *pEquation = new Equation;
				obj.setEquation(pEquation);
				pEquation->setOperation(eOp);

				// 引数の追加
				if(!checkNextToken(TK_BRACKET_RND_CLOSE)){
					do{
						// 引数を解析して追加
						UnivType& arg = pEquation->addSingleArgument();
						condition(arg);
					}while(checkNextToken(TK_COMMA));
					assertNextToken(TK_BRACKET_RND_CLOSE);	// 括弧が閉じていること
				}

				// 自動的に引数を追加する必要がある特別な関数である場合
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

	// 組み込み関数でなかった場合
	bool isRefSymbol = primitive(obj);

	enum TokenKind eKind;
	while((eKind = getNextToken()) != TK_EMPTY){

		// ユーザー定義関数でも配列インデックスでもない場合
		if((eKind != TK_BRACKET_RND_OPEN) && (eKind != TK_BRACKET_SQR_OPEN)){
			putBackToken();
			break;
		}

		const UnivType *pRefer = NULL;
		// primitive()によって変数に無駄な参照がなされている場合は、解除して直接その参照先への参照を張る
		if(isRefSymbol){
			assert(obj.isReference());
			// 参照先のオブジェクトを取得する
			IndexConv *pIndexConv;
			obj.getReference(pRefer, pIndexConv);

			if(pIndexConv == NULL){
				// objの参照を解除する
				obj.releaseReference();
			}else{
				// 意味のある参照であるため、そこに再度参照を作成するようにする
				pRefer = NULL;
			}

			isRefSymbol = false;	// 関数呼び出し又はインデックス参照となるため、変数への単純参照とならない
		}
		// obj自体を参照先とするために、objの内容を移動して保存しておく
		if(pRefer == NULL){
			// 参照される側となるオブジェクトを新規作成したコンテナに移動する
			UnivType& ReferredObj = m_pSubsetContext->addNew();
			ReferredObj.move(obj);
			pRefer = &ReferredObj;
		}

		// ユーザー定義関数の呼び出しである場合
		if(eKind == TK_BRACKET_RND_OPEN){
			if(m_pSentenceContext->m_isReference){
				throw CdbException(CDB_REFERENCE_TO_FUNC);
			}

			// カスタム関数呼び出しオブジェクトを作成
			Equation *pEquation = new Equation;
			obj.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_CUSTOM_FUNC_CALL);

			// 最初の引数は、カスタム関数オブジェクトである
			UnivType& custom_func = pEquation->addSingleArgument();
			custom_func.setConstReference(pRefer);

			// 引数の追加
			if(!checkNextToken(TK_BRACKET_RND_CLOSE)){
				do{
					// 引数を解析して追加
					UnivType& arg = pEquation->addSingleArgument();
					condition(arg);
				}while(checkNextToken(TK_COMMA));
				assertNextToken(TK_BRACKET_RND_CLOSE);	// 括弧が閉じていること
			}
		}

		// 範囲指定インデックス付き参照である場合
		if(eKind == TK_BRACKET_SQR_OPEN){

			// IndexConvオブジェクトを作成
			IndexConv *pIndexConv = new IndexConv;

			// 最初の引数（行インデックス指定、又はベクトル指定）
			arrayIndexRight(pIndexConv, pRefer, DIM_VEC);

			// コンマがあれば、列インデックス指定を読み込む
			if(checkNextToken(TK_COMMA)){
				arrayIndexRight(pIndexConv, pRefer, DIM_COL);
				pIndexConv->setVectorAccess(false);
			}else{
				// コンマがなければベクトル指定
				pIndexConv->setVectorAccess(true);
			}

			// インデックス付きの変数参照
			if(m_pSentenceContext->m_isReference){
				obj.setFullReference(const_cast<UnivType *>(pRefer), pIndexConv);
			}else{
				obj.setConstReference(pRefer, pIndexConv);
			}

			assertNextToken(TK_BRACKET_SQR_CLOSE);	// 括弧が閉じていること	
		}
	}

	return isRefSymbol;
}

// 戻り値：objが名前付き変数への単純参照であるかどうか
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
		// TK_BEGIN, TK_ENDは配列インデックス内のみで有効
		if(m_pIndexContext && m_pIndexContext->m_isInsideArrayIndex && m_pSentenceContext->m_isInsideRight){	// この関数では、右辺式中のみを取り扱う
			// 要素の開始インデックスは必ずゼロである
			obj = 0;
		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_END:
		// TK_BEGIN, TK_ENDは配列インデックス内のみで有効
		if(m_pIndexContext && m_pIndexContext->m_isInsideArrayIndex && m_pSentenceContext->m_isInsideRight){	// この関数では、右辺式中のみを取り扱う
			// 要素数を取得する数式オブジェクトを生成

			// 数をデクリメントするための数式を作成
			Equation *pEquation = new Equation;
			obj.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_MINUS);
			pEquation->setNumArguments(2);
			pEquation->getArgumentAt(1) = 1;
			UnivType& NumElements = pEquation->getArgumentAt(0);

			// 要素数を取得する数式を作成
			pEquation = new Equation;
			NumElements.setEquation(pEquation);
			pEquation->setOperation(Equation::OP_ARRAY_SIZE);
			pEquation->setNumArguments(3);
			pEquation->getArgumentAt(0).setConstReference(m_pIndexContext->m_pTargetArray);	// サイズ取得対象の変数
			pEquation->getArgumentAt(1) = m_pIndexContext->m_arrayDimensionId;
			pEquation->getArgumentAt(2) = m_pIndexContext->m_pIndexConv->getVectorAccess();	// ベクトルアクセスかどうかを渡す

		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_BRACKET_RND_OPEN:
		if((m_pBraceContext->m_currentBrace != TK_INVALID) && (m_pBraceContext->m_currentBrace != TK_BRACKET_RND_OPEN)){
			// 異種の括弧で囲まれていた場合
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACKET_RND_CLOSE)){
			// 空リスト
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
			// 異種の括弧で囲まれていた場合
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACKET_SQR_CLOSE)){
			// 空リスト
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
			// 異種の括弧で囲まれていた場合
			if(m_pCatContext != NULL){
				m_pCatContext->setNested(true);
			}
		}
		if(checkNextToken(TK_BRACE_CLOSE)){
			// 空リスト
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
		// スカラー評価式では、スカラー値しか評価できない
		expression(obj);
		assertNextToken(TK_SCALAR_EQN);
		break;
	case TK_PLUS:
		// 単項の＋ (最初の項の最初の基本要素でのみ有効)
		if((m_pBraceContext->m_countTerm == 0) && (m_pBraceContext->m_countPrimitive == 0)){
			m_pBraceContext->m_countTerm = INVALID;	// 単項＋が連続して使えないようにする

			// 基本的に、何もしない
			term(obj);

			m_pBraceContext->m_countTerm = 0;
		}else{
			throw CdbException(CDB_SYNTAX_ERROR);
		}
		break;
	case TK_MINUS:
		// 単項の－ (最初の項の最初の基本要素でのみ有効)
		if((m_pBraceContext->m_countTerm == 0) && (m_pBraceContext->m_countPrimitive == 0)){
			m_pBraceContext->m_countTerm = INVALID;	// 単項＋が連続して使えないようにする

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

			// 通常の変数参照
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

	return eKind == TK_SYMBOL;	// トークンがシンボルであれば、objは名前付き変数への単純参照となる
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
			// 要素が最終的に１つしかなかった場合、列に関する要素指定を無効とする
			if(m_pSentenceContext->m_isReference){
				pLastObj->releaseReference();	// 参照解除、IndexConvオブジェクト削除
				pLastObj->setFullReference(&obj);
			}else{
				// constが設定されていたらいったん解除する
				bool isConstant = pLastObj->isConstant();
				pLastObj->setConstant(false);

				pLastObj->setConstReference(&obj);

				// constを設定しなおす
				pLastObj->setConstant(isConstant);
			}
		}else{
			// 最後の要素を最終要素までとする
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
			// 要素が最終的に１つしかなかった場合、行に関する要素指定を無効とする
			if(m_pSentenceContext->m_isReference){
				pLastObj->releaseReference();	// 参照解除、IndexConvオブジェクト削除
				pLastObj->setFullReference(&obj);
			}else{
				// constが設定されていたらいったん解除する
				bool isConstant = pLastObj->isConstant();
				pLastObj->setConstant(false);

				pLastObj->setConstReference(&obj);

				// constを設定しなおす
				pLastObj->setConstant(isConstant);
			}
		}else{
			// 最後の要素を最終要素までとする
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
		// 現状、同種の括弧でもネスト参照扱いとする
		{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_currentBrace = TK_BRACKET_RND_OPEN;
			matrixLeft(obj);
		}
		assertNextToken(TK_BRACKET_RND_CLOSE);
		break;
	case TK_BRACKET_SQR_OPEN:
		// 現状、同種の括弧でもネスト参照扱いとする
		{
			BraceContext context(m_pBraceContext);
			m_pBraceContext->m_currentBrace = TK_BRACKET_SQR_OPEN;
			matrixLeft(obj);
		}
		assertNextToken(TK_BRACKET_SQR_CLOSE);
		break;
	case TK_BRACE_OPEN:
		// 現状、同種の括弧でもネスト参照扱いとする
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
				// 範囲指定インデックス付き
				UnivType *pNamedObj;
				Equation *pEquation = NULL;

				try{
					UnivType& NamedObj = m_pSubsetContext->getUnsafeUnivType(pName);
					if(!NamedObj.isEquation()) throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
					pEquation = NamedObj.getEquation();
					pNamedObj = &NamedObj;
				}
				catch(CdbException& e){
					// 登録が見つからない以外のエラーは投げなおし
					if(e.getErrorCode() != CDB_NOTFOUND) throw e;

					// 数式を新規作成
					CDB_ERR error;
					UnivType& NewNamedObj = m_pSubsetContext->addNew(pName, error);
					if(error != CDB_OK) throw CdbException(error);
					pEquation = new Equation;
					NewNamedObj.setEquation(pEquation);
					pEquation->setOperation(Equation::OP_CAT_PARTIAL_REF);
					pNamedObj = &NewNamedObj;
				}

				// IndexConvオブジェクトを作成
				IndexConv *pIndexConv = new IndexConv;

				// 最初の引数（行インデックス指定、又はベクトル指定）
				arrayIndexLeft(pIndexConv, pNamedObj, DIM_VEC);

				// コンマがあれば、列インデックス指定を読み込む
				if(checkNextToken(TK_COMMA)){
					arrayIndexLeft(pIndexConv, pNamedObj, DIM_COL);
					pIndexConv->setVectorAccess(false);
				}else{
					// コンマがなければベクトル指定
					pIndexConv->setVectorAccess(true);
				}

				assertNextToken(TK_BRACKET_RND_CLOSE);

				// 定義式を追加
				// 【動作】ここにおいて数式の引数に対してconst参照又はfull参照で追加しておくと、
				// Equation::OP_CAT_PARTIAL_REFが自動的に参照タイプを判定して、要素間の参照を作成する
				// 【注意】逆に、Equation::OP_CAT_PARTIAL_REFの引数に対しては、必ず参照を設定しなければならない
				UnivType& newDef = pEquation->addSingleArgument();
				if(m_pSentenceContext->m_isReference){
					newDef.setFullReference(&obj, pIndexConv);
				}else{
					newDef.setConstReference(&obj, pIndexConv);
				}
			}else{
				// 範囲指定インデックスなし
				CDB_ERR error = m_pSubsetContext->rename(&obj, pName);
				if(error != CDB_OK){
					if(error == CDB_KEY_COLLISION){
						// 既に登録されている変数が未定義だったら
						UnivType& temp = m_pSubsetContext->getUnsafeUnivType(pName);
						if(!temp.isValid()){
							temp.setFullReference(&obj);	// TODO : invalidな名前つきを消して、objをリネームしてもいい
							// 定数属性を付加
							temp.setConstant(m_pSentenceContext->m_isConstant);
							obj.setConstant(m_pSentenceContext->m_isConstant);	// 20110123 フル参照では定数属性は参照先に従うため 
						}else{
							// 既に同名称で定義されていたら
							throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
						}
					}else{
						assert(false);
					}
				}else{
					// 定数属性を付加
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
	// コンテキストの切り替え
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
	if(eKind == TK_COLON){	// 開始インデックスと終了インデックスによる区間指定
		primitive(end);
		eKind = getNextToken();
		if(eKind == TK_COLON){	// 等間隔インデックス指定
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
	// コンテキストの切り替え
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
	// 左辺式においては、begin, endが利用される場合には、引数に単体で存在していなくてはならないため、
	// この関数中で解析を行い、primitive()中で存在していてもエラーとする。 (end - 1)などの記述はできない。
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
	if(eKind == TK_COLON){	// 開始インデックスと終了インデックスによる区間指定
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
		if(eKind == TK_COLON){	// 等間隔インデックス指定
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
