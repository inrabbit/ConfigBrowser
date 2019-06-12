#include "UnivType.h"
#include "NamedUnivType.h"
#include "UnivMatrix.h"
#include "IndexConv.h"
#include "Equation.h"
#include "FormatToken.h"
#include "BaseDB.h"

#include <cstring>
#include <cassert>

struct UnivType::TypeInfo UnivType::m_initial_type = {FALSE, FALSE, FALSE, TRUE, TYPE_EMPTY}; // 081102 デフォルトで定数属性はなしとした

void UnivType::Initialize()
{
	m_type = m_initial_type;

#ifdef UNION_AS_STRUCT
	memset(&m_data, 0, sizeof(m_data));
#endif
}

// フル参照に影響されない、自身の強制クリア
void UnivType::clearAll()
{
	m_type.isConstant = FALSE;	// const属性だとclearできないため
	releaseReference();
	clear();
}

void UnivType::clear()
{
	if(isConstant()) throwException(CDB_WRITE_TO_CONST);

	switch(m_type.contentKind){
	case TYPE_MATRIX:
		delete m_data.pMatrix;
		break;
	case TYPE_REF:	// 【注意】参照先をクリアすることになる。フル参照はclear()では解除されない。
		getScalarReference().clear();
		break;
	case TYPE_REF_CONST:
		// 参照元 m_data.refer.pRefer は消さない
		delete m_data.refer.pIndexConv;
		break;
	case TYPE_EQUATION:
		delete m_data.pEquation;
		break;
	case TYPE_STRING:
		delete[] m_data.string.pString;
		break;
	case TYPE_BINARY:
		delete[] m_data.binData.pData;
		break;
	case TYPE_SUBSET:
		if(m_type.isObjectOwner){
			delete m_data.subset.pDB;
		}
		break;
	default:
		;
	}
	m_type.contentKind = TYPE_EMPTY;

#ifdef UNION_AS_STRUCT
	memset(&m_data, 0, sizeof(m_data));
#endif
}

// TODO : フル参照の場合は、参照先をinvalidにすべき
void UnivType::invalidate()
{
	if(isConstant()) throwException(CDB_WRITE_TO_CONST);

	clearAll();
	m_type.contentKind = TYPE_INVALID;
}

inline UnivType& UnivType::evalEquation() const
{
	try{
		return m_data.pEquation->eval();
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::throwException(CDB_ERR eErrorCode) const
{
	CdbException e(eErrorCode);
	if(m_type.hasName){
		const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
		e.addVariableName(pNamed->getName());
	}
	throw e;
}

inline UnivType& UnivType::getScalarReference()
{
	if(isIndexConvAvailable()){
		if(m_data.refer.pIndexConv->getNumElements() == 1){
			return getAt(0);
		}else{
			// 参照要素がスカラ型でないのでエラー
			throwException(CDB_NOT_SCALAR);
			return *this;
		}
	}else{
		return *m_data.refer.pRefer;
	}
}

inline const UnivType& UnivType::getScalarReference() const
{
	if(isIndexConvAvailable()){
		if(m_data.refer.pIndexConv->getNumElements() == 1){
			return getAtConst(0);
		}else{
			// 参照要素がスカラ型でないのでエラー
			throwException(CDB_NOT_SCALAR);
			return *this;
		}
	}else{
		return *m_data.refer.pRefer;
	}
}

inline const UnivType& UnivType::getReferenceForTypeInspection() const
{
	if(isIndexConvAvailable()){
		if(m_data.refer.pIndexConv->getNumElements() == 1){
			return getAtConst(0);
		}
	}
	return *m_data.refer.pRefer;
}

inline bool UnivType::isIndexConvAvailable() const
{
	if(m_data.refer.pIndexConv != NULL){
		m_data.refer.pIndexConv->update();	// 110127 配列参照先の更新タイミングが存在しなかったので追加 
		return true;
	}else{
		return false;
	}
}

inline bool UnivType::isIndexConvExists() const
{
	return m_data.refer.pIndexConv != NULL;
}

void UnivType::simplify() const
{
	switch(m_type.contentKind){
	case TYPE_REF:
	case TYPE_REF_CONST:
		m_data.refer.pRefer->simplify();
		if(isIndexConvAvailable()){
			m_data.refer.pIndexConv->simplify();
		}
		// 固定された参照先を自身にコピーする
		// （メモリの新規確保を伴わない場合にのみ行う）
		if(isFixed()){
			int type_id = getContentTypeID();
			if(
				(type_id == TYPE_EMPTY) ||
				(type_id == TYPE_INT) ||
				(type_id == TYPE_UINT) ||
				(type_id == TYPE_REAL) ||
				(type_id == TYPE_DOUBLE) ||
				(type_id == TYPE_POINTER) ||
				(type_id == TYPE_BOOL) ||
				(type_id == TYPE_INVALID)
				){
			}
			const UnivType& ref = getScalarReference();
			UnivType *pThis = const_cast<UnivType *>(this);
			pThis->copy(*ref.getReferenceRoot(true));
		}
		break;
	case TYPE_EQUATION:
		m_data.pEquation->simplify();
		if(isFixed()){
			// 算出結果を自オブジェクトに代入する
			UnivType temp;
			temp.move(evalEquation());
			UnivType *pThis = const_cast<UnivType *>(this);
			pThis->move(temp);	// 数式を削除して評価結果で上書きする
		}
		break;
	default:
		break;
	}
}

bool UnivType::isConstant() const
{
	if(m_type.contentKind == TYPE_REF){
		// フル参照であった場合には、参照先の設定に従う
		return getReferenceForTypeInspection().isConstant();
	}else{
		// フル参照以外は自身の設定に従う
		return m_type.isConstant == TRUE;
	}
}

void UnivType::setConstant(bool isConstant)
{
	// TODO : 全てへの変更通知
	m_type.isConstant = isConstant ? TRUE : FALSE;
}

bool UnivType::isFixed() const
{
	// 他変数への依存先がfixedかどうかチェックする
	switch(m_type.contentKind){
	case TYPE_MATRIX:
		if(!m_data.pMatrix->isFixed()){
			return false;
		}
		break;
	case TYPE_REF:
	case TYPE_REF_CONST:
		if(!getReferenceForTypeInspection().isFixed()){
			return false;
		}
		if(isIndexConvAvailable()){
			if(!m_data.refer.pIndexConv->isFixed()){
				return false;
			}
		}
		break;
	case TYPE_EQUATION:
		if(!m_data.pEquation->isFixed()){
			return false;
		}
		break;
	case TYPE_SUBSET:
		return true;	// TODO:現状subsetは常にfixedとしておくが、本来はBaseDBがisFixed()を持つべきである 
	default:
		break;
	}

	// 自身がconstであること
	return isConstant();
}

void UnivType::copy(const UnivType& src)
{
	if(isConstant()) throwException(CDB_WRITE_TO_CONST);

	// 自身がフル参照の場合は参照先に対してコピーする
	if(isFullReference()){
		try{
			getScalarReference().copy(src);
		}
		catch(CdbException& e){
			if(m_type.hasName){
				const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
				e.addVariableName(pNamed->getName());
			}
			throw e;
		}
		return;
	}

	// コピー前になるべくオブジェクト数を減らしておく
	src.simplify();

	switch(src.m_type.contentKind){
	case TYPE_MATRIX:
		{
			int nRow = src.getNumRow();
			int nCol = src.getNumColumn();
			setMatrix(nRow, nCol);
			int i, j;
			for(i = 0; i < nRow; i++){
				for(j = 0; j < nCol; j++){
					getAt(i, j).copy(src.getAtConst(i, j));
				}
			}
		}
		break;
	case TYPE_REF:	// TODO:フル参照の場合の挙動を定義すること
	case TYPE_REF_CONST:
		if(src.isIndexConvExists()){
			// IndexConvオブジェクトのコピー
			IndexConv *pIndexConv = new IndexConv;
			pIndexConv->copy(*src.m_data.refer.pIndexConv);

			if(src.m_data.refer.pRefer->m_type.hasName){	// コピー対称の参照先が名前付きならば、その参照先オブジェクトへの参照とする
				m_data.refer.pRefer = src.m_data.refer.pRefer;
				m_data.refer.pIndexConv = pIndexConv;
				m_type.contentKind = src.m_type.contentKind;
			}else{
				m_data.refer.pRefer = new UnivType;
				m_data.refer.pIndexConv = pIndexConv;
				m_type.contentKind = src.m_type.contentKind;
				m_data.refer.pRefer->copy(*src.m_data.refer.pRefer);
				pIndexConv->setConvertReferenced(m_data.refer.pRefer);
			}
		}else{
			if(src.m_data.refer.pRefer->m_type.hasName){	// コピー対称の参照先が名前付きならば、その参照先オブジェクトへの参照とする
				m_data.refer.pRefer = src.m_data.refer.pRefer;
				m_data.refer.pIndexConv = src.m_data.refer.pIndexConv;
				m_type.contentKind = src.m_type.contentKind;
			}else{
				copy(*src.m_data.refer.pRefer);
			}
		}
		break;
	case TYPE_EQUATION:
		{
			Equation *pEquation = new Equation;
			pEquation->copy(*src.m_data.pEquation);
			setEquation(pEquation);
		}
		break;
	case TYPE_SUBSET:
		{
			BaseDB *pDB = new BaseDB;
			pDB->copy(src.getSubsetDB());
			setSubset(pDB);
		}
		break;
	case TYPE_STRING:
		setString(src.getString());
		break;
	case TYPE_BINARY: 
		setBinary(src.getBinary(), src.getBinarySize());
		break;
	default:
		clear();
		m_data = src.m_data;
		m_type.contentKind = src.m_type.contentKind;
		break;
	}
}

void UnivType::evalCopy(const UnivType& src)
{
	if(isConstant()) throwException(CDB_WRITE_TO_CONST);

	if(src.isEmpty()){
		clear();
	}else if(src.isMatrix()){
		int nRow = src.getNumRow();
		int nCol = src.getNumColumn();
		setMatrix(nRow, nCol);
		int i, j;
		for(i = 0; i < nRow; i++){
			for(j = 0; j < nCol; j++){
				getAt(i, j).evalCopy(src.getAtConst(i, j));
			}
		}
	}else if(src.isInteger()){
		setInt(src.getInt());
	}else if(src.isUInteger()){
		setUInt(src.getUInt());
	}else if(src.isReal()){
		setReal(src.getReal());
	}else if(src.isDouble()){
		setDouble(src.getDouble());
	}else if(src.isPointer()){
		setPointer(src.getPointer());
	}else if(src.isBoolean()){
		setBool(src.getBool());
	}else if(src.isString()){
		setString(src.getString());
	}else if(src.isBinary()){
		setBinary(src.getBinary(), src.getBinarySize());
	}else if(src.isSubset()){
#if 1
		BaseDB *pDB = new BaseDB;
		pDB->evalCopy(src.getSubsetDB());
		setSubset(pDB);
#else
		setSubset(src.getSubsetDB(), false);	// オブジェクトの所有は引き継がない(勝手に開放させない)
#endif
	}else{
		throwException(CDB_UNKNOWN_TYPE);
	}
}

void UnivType::move(UnivType& src)
{
	switch(src.m_type.contentKind){
	case TYPE_REF:
		getScalarReference().move(src);
		break;
	default:
		if(isConstant()) throwException(CDB_WRITE_TO_CONST);
		clear();
		m_data = src.m_data;
		{	// 120713 hasName属性は上書きしないようにした
			int hasName = m_type.hasName;
			m_type = src.m_type;
			m_type.hasName = hasName;
		}
		src.m_type.contentKind = TYPE_EMPTY;
	}
}

void UnivType::setMatrix(int row, int col)
{
	if(m_type.contentKind == TYPE_REF){
		//100714 m_data.refer.pRefer->setMatrix(row, col);
		getScalarReference().setMatrix(row, col);
		return;
	}

	if(isConstant()) throwException(CDB_WRITE_TO_CONST);

	if(m_type.contentKind == TYPE_MATRIX){
		// 既に行列である場合には、オブジェクトを再利用する
		m_data.pMatrix->allocate(row, col);
	}else{
		// 行列オブジェクトの新規作成
		clear();
		m_data.pMatrix = new UnivMatrix(row, col);
		m_type.contentKind = TYPE_MATRIX;
	}
}

UnivType& UnivType::getAt(int n)
{
	if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);

	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getAt(n);
#if 1	// 100714
		case TYPE_REF:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int m = 0;
					m_data.refer.pIndexConv->convert(m);
					return m_data.refer.pRefer->getAt(m).getAt(n);
				}else{
					m_data.refer.pIndexConv->convert(n);
					return m_data.refer.pRefer->getAt(n);
				}
			}else{
				return m_data.refer.pRefer->getAt(n);
			}
#else
		case TYPE_REF:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(n);
			}
			return m_data.refer.pRefer->getAt(n);
#endif
		case TYPE_REF_CONST:
			throw CdbException(CDB_WRITE_TO_CONST);
			return *this;
		case TYPE_EQUATION:	// 【注意】数式の結果としての配列への書き込みが起こった時点で、全行列要素の値は固定される(Equation::OP_THROUGH_TILL_WRITEによる) 
			return evalEquation().getAt(n);
		default:
			return *this;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName(), n);
		}
		throw e;
	}
}

UnivType& UnivType::getAt(int row, int col)
{
	if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);

	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getAt(row, col);
#if 1	// 100714
		case TYPE_REF:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int x = 0, y = 0;
					m_data.refer.pIndexConv->convert(x, y);
					return m_data.refer.pRefer->getAt(x, y).getAt(row, col);
				}else{
					m_data.refer.pIndexConv->convert(row, col);
					return m_data.refer.pRefer->getAt(row, col);
				}
			}else{
				return m_data.refer.pRefer->getAt(row, col);
			}
#else
		case TYPE_REF:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(row, col);
			}
			return m_data.refer.pRefer->getAt(row, col);
#endif
		case TYPE_REF_CONST:
			throw CdbException(CDB_WRITE_TO_CONST);
			return *this;
		case TYPE_EQUATION:	// 【注意】数式の結果としての配列への書き込みが起こった時点で、全行列要素の値は固定される(Equation::OP_THROUGH_TILL_WRITEによる) 
			return evalEquation().getAt(row, col);
		default:
			return *this;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName(), row, col);
		}
		throw e;
	}
}

const UnivType& UnivType::getAtConst(int n) const
{
	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getAt(n);
#if 1	// 100714
		case TYPE_REF:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int m = 0;
					m_data.refer.pIndexConv->convert(m);
					return m_data.refer.pRefer->getAtConst(m).getAtConst(n);
				}else{
					m_data.refer.pIndexConv->convert(n);
					return m_data.refer.pRefer->getAtConst(n);
				}
			}else{
				return m_data.refer.pRefer->getAtConst(n);
			}
		case TYPE_REF_CONST:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int m = 0;
					m_data.refer.pIndexConv->convert(m);
					return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(m).getAtConst(n);
				}else{
					m_data.refer.pIndexConv->convert(n);
					return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(n);
				}
			}else{
				return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(n);
			}
#else
		case TYPE_REF:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(n);
			}
			return m_data.refer.pRefer->getAtConst(n);
		case TYPE_REF_CONST:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(n);
			}
			return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(n);
#endif
		case TYPE_EQUATION:
			return m_data.pEquation->eval().getAtConst(n);
		default:
			return *this;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName(), n);
		}
		throw e;
	}
}

const UnivType& UnivType::getAtConst(int row, int col) const
{
	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getAt(row, col);
#if 1	// 100714
		case TYPE_REF:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int x = 0, y = 0;
					m_data.refer.pIndexConv->convert(x, y);
					return m_data.refer.pRefer->getAtConst(x, y).getAt(row, col);
				}else{
					m_data.refer.pIndexConv->convert(row, col);
					return m_data.refer.pRefer->getAtConst(row, col);
				}
			}else{
				return m_data.refer.pRefer->getAtConst(row, col);
			}
		case TYPE_REF_CONST:
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int x = 0, y = 0;
					m_data.refer.pIndexConv->convert(x, y);
					return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(x, y).getAtConst(row, col);
				}else{
					m_data.refer.pIndexConv->convert(row, col);
					return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(row, col);
				}
			}else{
				return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(row, col);
			}
#else
		case TYPE_REF:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(row, col);
			}
			return m_data.refer.pRefer->getAtConst(row, col);
		case TYPE_REF_CONST:
			if(m_data.refer.pIndexConv){
				m_data.refer.pIndexConv->convert(row, col);
			}
			return static_cast<const UnivType *>(m_data.refer.pRefer)->getAtConst(row, col);
#endif
		case TYPE_EQUATION:
			return m_data.pEquation->eval().getAtConst(row, col);
		default:
			return *this;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName(), row, col);
		}
		throw e;
	}
}

int UnivType::getNumRow() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getNumRow();
		case TYPE_REF:
		case TYPE_REF_CONST:
#if 1	// 100714
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int n = 0;
					m_data.refer.pIndexConv->convert(n);
					return m_data.refer.pRefer->getAtConst(n).getNumRow();
				}else{
					return m_data.refer.pIndexConv->getNumRow();
				}
			}else{
				return m_data.refer.pRefer->getNumRow();
			}
#else
			if(m_data.refer.pIndexConv){
				return m_data.refer.pIndexConv->getNumRow();
			}else{
				return m_data.refer.pRefer->getNumRow();
			}
#endif
		case TYPE_EQUATION:
			return m_data.pEquation->eval().getNumRow();
		default:
			return isScalar() ? 1 : 0;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

int UnivType::getNumColumn() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getNumColumn();
		case TYPE_REF:
		case TYPE_REF_CONST:
#if 1	// 100714
			if(isIndexConvAvailable()){
				if(m_data.refer.pIndexConv->getNumElements() == 1){
					int n = 0;
					m_data.refer.pIndexConv->convert(n);
					return m_data.refer.pRefer->getAtConst(n).getNumColumn();
				}else{
					return m_data.refer.pIndexConv->getNumColumn();
				}
			}else{
				return m_data.refer.pRefer->getNumColumn();
			}
#else
			if(m_data.refer.pIndexConv){
				return m_data.refer.pIndexConv->getNumColumn();
			}else{
				return m_data.refer.pRefer->getNumColumn();
			}
#endif
		case TYPE_EQUATION:
			return m_data.pEquation->eval().getNumColumn();
		default:
			return isScalar() ? 1 : 0;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

int UnivType::getNumElements() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_MATRIX:
			return m_data.pMatrix->getNumElements();
		case TYPE_REF:
		case TYPE_REF_CONST:
#if 1	// 100714
			if(isIndexConvAvailable()){
				int n = m_data.refer.pIndexConv->getNumElements();
				if(n == 1){
					int m = 0;
					m_data.refer.pIndexConv->convert(m);
					return m_data.refer.pRefer->getAtConst(m).getNumElements();
				}else{
					return n;
				}
			}else{
				return m_data.refer.pRefer->getNumElements();
			}
#else
			if(m_data.refer.pIndexConv){
				return m_data.refer.pIndexConv->getNumElements();
			}else{
				return m_data.refer.pRefer->getNumElements();
			}
#endif
		case TYPE_EQUATION:
			return m_data.pEquation->eval().getNumElements();
		default:
			return isScalar() ? 1 : 0;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

int UnivType::getInt() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_INT:
			return m_data.integer;
		case TYPE_UINT:
			return (int)m_data.uinteger;
		case TYPE_REAL:
			return (int)m_data.real;
		case TYPE_DOUBLE:
			return (int)m_data.dreal;
		case TYPE_BOOL:
			return m_data.boolean ? 1 : 0;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getInt();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getInt();
		case TYPE_EQUATION:
			return evalEquation().getInt();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

unsigned int UnivType::getUInt() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_UINT:
		case TYPE_POINTER:
			return m_data.uinteger;
		case TYPE_INT:
			return (unsigned int)m_data.integer;
		case TYPE_REAL:
			return (unsigned int)m_data.real;
		case TYPE_DOUBLE:
			return (unsigned int)m_data.dreal;
		case TYPE_BOOL:
			return m_data.boolean ? 1 : 0;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getUInt();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getUInt();
		case TYPE_EQUATION:
			return evalEquation().getUInt();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void *UnivType::getPointer() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_UINT:
			return (void *)m_data.uinteger;
		case TYPE_POINTER:
			return m_data.ptr;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getPointer();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getPointer();
		case TYPE_EQUATION:
			return evalEquation().getPointer();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

bool UnivType::getBool() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_BOOL:
			return m_data.boolean;
		case TYPE_INT:
			return m_data.integer != 0;
		case TYPE_UINT:
			return m_data.uinteger != 0;
		case TYPE_REAL:
			return m_data.real != 0;
		case TYPE_DOUBLE:
			return m_data.dreal != 0;
		case TYPE_POINTER:
			return m_data.ptr != NULL;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getBool();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getBool();
		case TYPE_EQUATION:
			return evalEquation().getBool();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

UnivType::Real UnivType::getReal() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_REAL:
			return m_data.real;
		case TYPE_DOUBLE:
			return (Real)m_data.dreal;
		case TYPE_INT:
			return (Real)m_data.integer;
		case TYPE_UINT:
			return (Real)m_data.uinteger;
		case TYPE_BOOL:
			return m_data.boolean ? (Real)1 : (Real)0;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getReal();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getReal();
		case TYPE_EQUATION:
			return evalEquation().getReal();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

UnivType::Double UnivType::getDouble() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_DOUBLE:
			return m_data.dreal;
		case TYPE_REAL:
			return (Double)m_data.real;
		case TYPE_INT:
			return (Double)m_data.integer;
		case TYPE_UINT:
			return (Double)m_data.uinteger;
		case TYPE_BOOL:
			return m_data.boolean ? (Double)1 : (Double)0;
		case TYPE_STRING:
		{
			FormatToken format;
			return format.ToUnivTypeWithoutNewLine(m_data.string.pString).getDouble();
		}
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getDouble();
		case TYPE_EQUATION:
			return evalEquation().getDouble();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

const char *UnivType::getString() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_STRING:
			return m_data.string.pString;
		case TYPE_BOOL:
			return m_data.boolean ? "TRUE" : "FALSE";
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getString();
		case TYPE_EQUATION:
			return evalEquation().getString();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

size_t UnivType::getStringLength() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_STRING:
			return m_data.string.length;
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getStringLength();
		case TYPE_EQUATION:
			return evalEquation().getStringLength();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void *UnivType::getBinary() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_BINARY:
			return m_data.binData.pData;
		case TYPE_REF:
		case TYPE_REF_CONST:	// const参照なのにvoid*を返していいのか？
			return getScalarReference().getBinary();
		case TYPE_EQUATION:
			return evalEquation().getBinary();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

size_t UnivType::getBinarySize() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_BINARY:
			return m_data.binData.size;
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getBinarySize();
		case TYPE_EQUATION:
			return evalEquation().getBinarySize();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

BaseDB *UnivType::getSubsetDB() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_SUBSET:
			return m_data.subset.pDB;
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getSubsetDB();
		case TYPE_EQUATION:
			return evalEquation().getSubsetDB();
		case TYPE_MATRIX:
			throw CdbException(CDB_NOT_SCALAR);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::toString(UnivType& obj) const
{
	if(isString()){
		obj.setConstReference(this);
	}else{
		FormatToken format;
		obj.setString(format.ToStringWithoutSuffix(*this));
	}
}

void UnivType::toBinary(UnivType& obj) const
{
	if(isBinary()){
		obj.setConstReference(this);
	}else{
		throw CdbException(CDB_NOT_IMPLEMENTED);
	}
}

void UnivType::toSubsetDB(UnivType& obj) const
{
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

void UnivType::getReference(UnivType*& pRefer, IndexConv*& pIndexConv) const
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			pRefer = m_data.refer.pRefer;
			pIndexConv = m_data.refer.pIndexConv;
			break;
		case TYPE_REF_CONST:
			throw CdbException(CDB_WRITE_TO_CONST);
		default:
			throw CdbException(CDB_NOT_REFERENCE);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::getReference(const UnivType*& pRefer, IndexConv*& pIndexConv) const
{
	assert((m_type.contentKind == TYPE_REF) || (m_type.contentKind == TYPE_REF_CONST));
	pRefer = m_data.refer.pRefer;
	pIndexConv = m_data.refer.pIndexConv;
}

Equation *UnivType::getEquation()
{
	try{
		switch(m_type.contentKind){
		case TYPE_EQUATION:
			return m_data.pEquation;
		case TYPE_REF:
			return getScalarReference().getEquation();
		case TYPE_REF_CONST:
			throw CdbException(CDB_WRITE_TO_CONST);
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

const Equation *UnivType::getConstEquation() const
{
	try{
		switch(m_type.contentKind){
		case TYPE_EQUATION:
			return m_data.pEquation;
		case TYPE_REF:
		case TYPE_REF_CONST:
			return getScalarReference().getConstEquation();
		default:
			throw CdbException(CDB_CAST_ERROR);
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setInt(int x)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setInt(x);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.integer = x;
			m_type.contentKind = TYPE_INT;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setUInt(unsigned int x)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setUInt(x);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.uinteger = x;
			m_type.contentKind = TYPE_UINT;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setPointer(void *p)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setPointer(p);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.ptr = p;
			m_type.contentKind = TYPE_POINTER;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setBool(bool x)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setBool(x);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.boolean = x;
			m_type.contentKind = TYPE_BOOL;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setReal(Real x)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setReal(x);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.real = x;
			m_type.contentKind = TYPE_REAL;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setDouble(Double x)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setDouble(x);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.dreal = x;
			m_type.contentKind = TYPE_DOUBLE;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setString(const char *p)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setString(p);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			assert(p != NULL);
			clear();
			m_data.string.length = strlen(p);
			m_data.string.pString = new char[m_data.string.length + 1];
			strcpy(m_data.string.pString, p);
			m_type.contentKind = TYPE_STRING;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setBinary(const void *p, size_t size)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setBinary(p, size);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			clear();
			m_data.binData.size = size;
			m_data.binData.pData = new unsigned char[size];
			if(p != NULL) memcpy(m_data.binData.pData, p, size);
			m_type.contentKind = TYPE_BINARY;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setSubset(BaseDB *pDB, bool delegated/* = true*/)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setSubset(pDB, delegated);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			assert(pDB != NULL);
			clear();
			m_data.subset.pDB = pDB;
			m_type.contentKind = TYPE_SUBSET;
			m_type.isObjectOwner = delegated ? TRUE : FALSE;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setEquation(Equation *pEquation)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setEquation(pEquation);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			assert(pEquation != NULL);
			clear();
			m_data.pEquation = pEquation;
			m_type.contentKind = TYPE_EQUATION;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::releaseReference()
{
	if((m_type.contentKind == TYPE_REF) || (m_type.contentKind == TYPE_REF_CONST)){
		// フル参照の解除
		if(isIndexConvExists()){
			delete m_data.refer.pIndexConv;
		}
		m_type.contentKind = TYPE_EMPTY;
	}
#if 0	// 090805 参照でないものに対して参照解除してもエラーを出さないようにした
	else{
		throw CdbException(CDB_NOT_REFERENCE);
	}
#endif
}

void UnivType::setFullReference(UnivType *pRefer, IndexConv *pIndexConv/* = NULL*/)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setFullReference(pRefer, pIndexConv);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			if(pRefer == NULL) throw CdbException(CDB_NULL_REFERENCE);
			if(pRefer == this) throw CdbException(CDB_SELF_REFERENCE);
			clear();
			m_data.refer.pRefer = pRefer;
			m_data.refer.pIndexConv = pIndexConv;
			if(pIndexConv != NULL){
				pIndexConv->setConvertReferenced(m_data.refer.pRefer);
			}
			m_type.contentKind = TYPE_REF;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

void UnivType::setConstReference(const UnivType *pRefer, IndexConv *pIndexConv/* = NULL*/)
{
	try{
		switch(m_type.contentKind){
		case TYPE_REF:
			getScalarReference().setConstReference(pRefer, pIndexConv);
			break;
		default:
			if(isConstant()) throw CdbException(CDB_WRITE_TO_CONST);
			if(pRefer == NULL) throw CdbException(CDB_NULL_REFERENCE);
			if(pRefer == this) throw CdbException(CDB_SELF_REFERENCE);
			clear();
			m_data.refer.pRefer = const_cast<UnivType *>(pRefer);
			m_data.refer.pIndexConv = pIndexConv;
			if(pIndexConv != NULL){
				pIndexConv->setConvertReferenced(m_data.refer.pRefer);
			}
			m_type.contentKind = TYPE_REF_CONST;
		}
	}
	catch(CdbException& e){
		if(m_type.hasName){
			const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
			e.addVariableName(pNamed->getName());
		}
		throw e;
	}
}

bool UnivType::isValid() const
{
	switch(m_type.contentKind){
	case TYPE_INVALID:
		return false;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isValid();
	case TYPE_EQUATION:
		return m_data.pEquation->isValid();
		//return evalEquation().isValid();
	default:
		return true;
	}
}

bool UnivType::isEmpty() const
{
	switch(m_type.contentKind){
	case TYPE_EMPTY:
		return true;
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isEmpty();
	case TYPE_EQUATION:
		return evalEquation().isEmpty();
	default:
		return false;
	}
}

bool UnivType::isEquation() const
{
	switch(m_type.contentKind){
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isEquation();
	case TYPE_EQUATION:
		return true;
	default:
		return false;
	}
}

bool UnivType::isScalar() const
{
	switch(m_type.contentKind){
	case TYPE_MATRIX:
		return false;
	case TYPE_REF:
	case TYPE_REF_CONST:
		if(isIndexConvAvailable()){
			if(m_data.refer.pIndexConv->getNumElements() == 1){
				int n = 0;
				m_data.refer.pIndexConv->convert(n);
				return m_data.refer.pRefer->getAtConst(n).isScalar();
			}else{
				return false;
			}
		}else{
			return m_data.refer.pRefer->isScalar();
		}
	case TYPE_EQUATION:
		return evalEquation().isScalar();
	default:
		return true;
	}
}

bool UnivType::isMatrix() const
{
	switch(m_type.contentKind){
	case TYPE_MATRIX:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		if(isIndexConvAvailable()){
			if(m_data.refer.pIndexConv->getNumElements() == 1){
				int n = 0;
				m_data.refer.pIndexConv->convert(n);
				return m_data.refer.pRefer->getAtConst(n).isMatrix();
			}else{
				return true;
			}
		}else{
			return m_data.refer.pRefer->isMatrix();
		}
	case TYPE_EQUATION:
		return evalEquation().isMatrix();
	default:
		return false;
	}
}

bool UnivType::isInteger() const
{
	switch(m_type.contentKind){
	case TYPE_INT:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isInteger();
	case TYPE_EQUATION:
		return evalEquation().isInteger();
	default:
		return false;
	}
}
	
bool UnivType::isUInteger() const
{
	switch(m_type.contentKind){
	case TYPE_UINT:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isUInteger();
	case TYPE_EQUATION:
		return evalEquation().isUInteger();
	default:
		return false;
	}
}
	
bool UnivType::isReal() const
{
	switch(m_type.contentKind){
	case TYPE_REAL:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isReal();
	case TYPE_EQUATION:
		return evalEquation().isReal();
	default:
		return false;
	}
}

bool UnivType::isDouble() const
{
	switch(m_type.contentKind){
	case TYPE_DOUBLE:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isDouble();
	case TYPE_EQUATION:
		return evalEquation().isDouble();
	default:
		return false;
	}
}

bool UnivType::isPointer() const
{
	switch(m_type.contentKind){
	case TYPE_POINTER:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isPointer();
	case TYPE_EQUATION:
		return evalEquation().isPointer();
	default:
		return false;
	}
}

bool UnivType::isBoolean() const
{
	switch(m_type.contentKind){
	case TYPE_BOOL:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isBoolean();
	case TYPE_EQUATION:
		return evalEquation().isBoolean();
	default:
		return false;
	}
}

bool UnivType::isString() const
{
	switch(m_type.contentKind){
	case TYPE_STRING:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isString();
	case TYPE_EQUATION:
		return evalEquation().isString();
	default:
		return false;
	}
}

bool UnivType::isBinary() const
{
	switch(m_type.contentKind){
	case TYPE_BINARY:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isBinary();
	case TYPE_EQUATION:
		return evalEquation().isBinary();
	default:
		return false;
	}
}

bool UnivType::isSubset() const
{
	switch(m_type.contentKind){
	case TYPE_SUBSET:
		return true;
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().isSubset();
	case TYPE_EQUATION:
		return evalEquation().isSubset();
	default:
		return false;
	}
}

bool UnivType::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	if(pUnivType == this){
		return true;
	}else{
		// 自分の依存先を調べる
		switch(m_type.contentKind){
		case TYPE_REF:
		case TYPE_REF_CONST:
			{
				bool changed1 = m_data.refer.pRefer->receiveNotifyUpdated(pUnivType, kind);
				bool changed2 = false;
				if(isIndexConvExists()){
					changed2 = m_data.refer.pIndexConv->receiveNotifyUpdated(pUnivType, kind);
				}
				return changed1 || changed2;
			}
		case TYPE_EQUATION:
			return m_data.pEquation->receiveNotifyUpdated(pUnivType, kind);
		case TYPE_SUBSET:
			if(RequireUpdateObj::getDB() == m_data.subset.pDB){
				return false;
				// root DBのupdateにおいて無限ループしないようにする
				// 他のsubsetの場合には、UnivType型に入っているため、pUnivType == thisの判定で弾かれる
			}
			return m_data.subset.pDB->receiveNotifyUpdated(pUnivType, kind);
		default:
			return false;
		}
	}
}

void UnivType::update()
{
#if 1
	// 110127 現時点で、この関数の呼び出しタイミングにて何もしていない
	// 利用する際のgetterの最初にて更新作業を行っている為
	return;
#else
	switch(m_type.contentKind){
	case TYPE_REF:
	case TYPE_REF_CONST:
		m_data.refer.pRefer->update();
		if(isIndexConvExists()){
			m_data.refer.pIndexConv->update();
		}
		break;
	case TYPE_EQUATION:
		m_data.pEquation->update();
		break;
	case TYPE_SUBSET:
		m_data.subset.pDB->update();
		break;
	default:
		;
	}
#endif
}

bool UnivType::replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith)
{
	bool replaced = false;
	// 自分の依存先を調べる
	switch(m_type.contentKind){
	case TYPE_REF:
	case TYPE_REF_CONST:
		if(m_data.refer.pRefer == pFindWhat){
			m_data.refer.pRefer = pReplaceWith;
			replaced = true;
		}else{
			replaced = m_data.refer.pRefer->replaceReference(pFindWhat, pReplaceWith);
		}
		if(isIndexConvExists()){
			replaced = replaced || m_data.refer.pIndexConv->replaceReference(pFindWhat, pReplaceWith);
		}
		break;
	case TYPE_EQUATION:
		replaced = m_data.pEquation->replaceReference(pFindWhat, pReplaceWith);
		break;
	case TYPE_SUBSET:
		replaced = m_data.subset.pDB->replaceReference(pFindWhat, pReplaceWith);
		break;
	default:
		break;
	}
	return replaced;
}

int UnivType::getContentTypeID() const
{
	switch(m_type.contentKind){
	case TYPE_REF:
	case TYPE_REF_CONST:
		return getReferenceForTypeInspection().getContentTypeID();
	case TYPE_EQUATION:
		return evalEquation().getContentTypeID();
	default:
		return (int)m_type.contentKind;
	}
}

char *UnivType::getFormatString(char *pBuff, size_t sizeBuff) const
{
	switch(m_type.contentKind){
	case TYPE_STRING:
		strncpy(pBuff, m_data.string.pString, sizeBuff);
		break;
	case TYPE_BOOL:
		strncpy(pBuff, (m_data.boolean ? "TRUE" : "FALSE"), sizeBuff);
		break;
	case TYPE_REF:
	case TYPE_REF_CONST:
		getScalarReference().getFormatString(pBuff, sizeBuff);
		break;
	case TYPE_EQUATION:
		evalEquation().getFormatString(pBuff, sizeBuff);
		break;
	case TYPE_INT:
	case TYPE_UINT:
	case TYPE_REAL: 
	case TYPE_DOUBLE: 
	case TYPE_POINTER:
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			strncpy(pBuff, p, sizeBuff);
		}
		break;
	case TYPE_MATRIX:
		throwException(CDB_NOT_SCALAR);
	default:
		throwException(CDB_CAST_ERROR);
	}

	return pBuff;
}

#ifdef DEBUG_PRINT
#include <cstdio>
void UnivType::debugPrintStructure(int nIndentLevel, bool collapsed/* = true*/) const
{
	if(collapsed && m_type.hasName){
		const NamedUnivType *pNamed = static_cast<const NamedUnivType *>(this);
		System::String ^pName = gcnew System::String(pNamed->getName());
#if 0
		System::Console::Write("[" + pName + "]");
#else
		char szTemp[128];
		sprintf(szTemp, "[%s(0x%x)]", pName, reinterpret_cast<int>(pNamed));
		System::Console::Write(gcnew System::String(szTemp));
#endif
		return;
	}

	switch(m_type.contentKind){
	case TYPE_EMPTY:
		System::Console::Write("EMPTY");
		break;
	case TYPE_REF:
		System::Console::Write("REF");
		if(isIndexConvAvailable()){
			// IndexConvの表示は保留
			System::Console::Write(":INDEXED");
		}
		System::Console::Write("->");
		m_data.refer.pRefer->debugPrintStructure(nIndentLevel);
		break;
	case TYPE_REF_CONST:
		System::Console::Write("REF_CONST");
		if(isIndexConvAvailable()){
			// IndexConvの表示は保留
			System::Console::Write(":INDEXED");
		}
		System::Console::Write("->");
		m_data.refer.pRefer->debugPrintStructure(nIndentLevel);
		break;
	case TYPE_MATRIX:
		m_data.pMatrix->debugPrintStructure(nIndentLevel);
		break;
	case TYPE_EQUATION:
		m_data.pEquation->debugPrintStructure(nIndentLevel);
		break;
	case TYPE_INT:
		System::Console::Write("INT:");
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			System::String ^pValue = gcnew System::String(p);
			System::Console::Write(pValue);
		}
		break;
	case TYPE_UINT:
		System::Console::Write("UINT:");
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			System::String ^pValue = gcnew System::String(p);
			System::Console::Write(pValue);
		}
		break;
	case TYPE_REAL: 
		System::Console::Write("REAL:");
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			System::String ^pValue = gcnew System::String(p);
			System::Console::Write(pValue);
		}
		break;
	case TYPE_DOUBLE: 
		System::Console::Write("DBL:");
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			System::String ^pValue = gcnew System::String(p);
			System::Console::Write(pValue);
		}
		break;
	case TYPE_POINTER:
		System::Console::Write("POINTER:");
		{
			FormatToken format;
			const char *p = format.ToString(*this);
			System::String ^pValue = gcnew System::String(p);
			System::Console::Write(pValue);
		}
		break;
	case TYPE_BOOL:
		System::Console::Write("BOOL:"+(m_data.boolean ? "TRUE" : "FALSE"));
		break;
	case TYPE_STRING:
		System::Console::Write("STRING:\""+gcnew System::String((const char *)m_data.string.pString)+"\"");
		break;
	case TYPE_BINARY:
		System::Console::Write("BINARY");
		break;
	case TYPE_SUBSET:
		System::Console::Write("SUBSET");
		m_data.subset.pDB->debugPrintStructure(nIndentLevel);
		break;
	case TYPE_INVALID:
		System::Console::Write("INVALID");
		break;
	default:
		throwException(CDB_CAST_ERROR);
	}
}
#endif

const UnivType *UnivType::getReferenceRoot(bool isGoBackOnlyThroughRef) const
{
	const UnivType *pUnivType = this;

	// 現在のインスタンスが参照であった場合、参照元を特定する
	while(pUnivType->isReference()){
		IndexConv *pIndexConv;
		pUnivType->getReference(pUnivType, pIndexConv);
		if(isGoBackOnlyThroughRef){
			if(pIndexConv != NULL) break;	// 090731 内容が変化しない範囲でのルートを返す
		}
	}

	return pUnivType;
}
