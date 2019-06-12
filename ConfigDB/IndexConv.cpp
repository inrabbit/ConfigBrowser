#include "IndexConv.h"
#include "CommonUtils.h"
#include <cassert>
#include <cstring>

IndexConv::IndexConv(bool isInverseTrans/* = false*/) : m_isInverseTrans(isInverseTrans)
{
	m_pReferenced = NULL;
	m_pReferer = NULL;

	m_IsTranspose = false;
	m_IsVectorAccess = false;
}

IndexConv::~IndexConv()
{
}

void IndexConv::update()
{
	// 更新が必要あるかどうか
	if(!requireUpdate()) return;

	// パラメータの更新
	updateParameters();

	int nRow, nCol;
	getReferencedSize(nRow, nCol);
	if(m_IsVectorAccess){
		// ベクトルアクセスの場合（vec演算子）
		m_RowMap.setNumElementsRange(nRow * nCol);
		m_ColMap.setNumElementsRange(1);
	}else{
		// 行列アクセスの場合
		m_RowMap.setNumElementsRange(nRow);
		m_ColMap.setNumElementsRange(nCol);
	}

	if(!m_isInverseTrans){
		// 順変換
		m_RowMap.update();
		m_ColMap.update();
	}else{
		// 逆変換
		m_RowMap.updateInverse();
		m_ColMap.updateInverse();
	}

	// 更新完了
	validate();
}

void IndexConv::convert(int& row, int& col) const
{
	if(!m_isInverseTrans){
		// 順変換
		m_RowMap.convert(row);
		m_ColMap.convert(col);
	}else{
		// 逆変換
		m_RowMap.convertInverse(row);
		m_ColMap.convertInverse(col);
	}

	// 転置
	if(m_IsTranspose){
		int temp = row;
		row = col;
		col = temp;
	}
}

void IndexConv::convert(int& n) const
{
	// 行列インデックスに戻す
	int nRefRow;
	if(!m_IsTranspose){
		nRefRow = m_RowMap.getNumElementsDomain();
	}else{
		nRefRow = m_ColMap.getNumElementsDomain();
	}
	int col = n / nRefRow;
	int row = n % nRefRow;

	// 行列インデックス変換
	convert(row, col);

	// ベクトル化（vec演算子）
	nRefRow = m_RowMap.getNumElementsRange();
	n = col * nRefRow + row;
}

bool IndexConv::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	bool updated = false;
	bool temp;

	if(kind & UK_FLAG_ARRAY_SIZE){
		// m_pReferencedとm_pRefererについては、自身の所有ではないため、サイズ更新のみチェックする
		updated = ((m_pReferenced == pUnivType) || (m_pReferer == pUnivType));
	}

	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		temp = m_RowData.IndexTable.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		break;
	case ICT_SERIES:
		temp = m_RowData.Begin.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		temp = m_RowData.Delta.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		temp = m_RowData.End.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		temp = m_ColData.IndexTable.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		break;
	case ICT_SERIES:
		temp = m_ColData.Begin.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		temp = m_ColData.Delta.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		temp = m_ColData.End.receiveNotifyUpdated(pUnivType, kind);
		updated = (updated || temp);
		break;
	default:
		;
	}

	if(updated) invalidate();

	return updated;
}

bool IndexConv::replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith)
{
	bool updated = false;
	bool temp;

	if(m_pReferenced == pFindWhat){
		m_pReferenced = pReplaceWith;
		updated = true;
	}
	if(m_pReferer == pFindWhat){
		m_pReferer = pReplaceWith;
		updated = true;
	}

	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		temp = m_RowData.IndexTable.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		break;
	case ICT_SERIES:
		temp = m_RowData.Begin.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		temp = m_RowData.Delta.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		temp = m_RowData.End.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		temp = m_ColData.IndexTable.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		break;
	case ICT_SERIES:
		temp = m_ColData.Begin.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		temp = m_ColData.Delta.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		temp = m_ColData.End.replaceReference(pFindWhat, pReplaceWith);
		updated = (updated || temp);
		break;
	default:
		;
	}

	if(updated) invalidate();

	return updated;
}

void IndexConv::updateParameters()
{
	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		m_RowData.IndexTable.update();
		if(m_RowData.IndexTable.isScalar()){
			m_RowMap.setIndexTableSingle((int)m_RowData.IndexTable);
		}else{
			int n = m_RowData.IndexTable.getNumElements();
			if(n == 1){
				m_RowMap.setIndexTableSingle((int)m_RowData.IndexTable[0]);
			}else{
				int *pTable = m_RowMap.setIndexTable(n);
				assert(pTable != NULL);
				for(int i = 0; i < n; i++){
					pTable[i] = (int)m_RowData.IndexTable[i];
				}
			}
		}
		break;
	case ICT_SERIES:
		m_RowData.Begin.update();
		m_RowData.Delta.update();
		m_RowData.End.update();
		if(m_RowData.Delta.isEmpty()){
#if 1
			m_RowData.Delta = 1;
#else
			if((int)m_RowData.Begin <= (int)m_RowData.End){
				m_RowData.Delta = 1;
			}else{
				m_RowData.Delta = -1;
			}
#endif
		}
		m_RowMap.setSeriesInfo(m_RowData.Begin, m_RowData.Delta, m_RowData.End);
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
	case ICT_TABLE_SINGLE:
		m_ColData.IndexTable.update();
		if(m_ColData.IndexTable.isScalar()){
			m_ColMap.setIndexTableSingle((int)m_ColData.IndexTable);
		}else{
			int n = m_ColData.IndexTable.getNumElements();
			if(n == 1){
				m_ColMap.setIndexTableSingle((int)m_ColData.IndexTable[0]);
			}else{
				int *pTable = m_ColMap.setIndexTable(n);
				assert(pTable != NULL);
				for(int i = 0; i < n; i++){
					pTable[i] = (int)m_ColData.IndexTable[i];
				}
			}
		}
		break;
	case ICT_SERIES:
		m_ColData.Begin.update();
		m_ColData.Delta.update();
		m_ColData.End.update();
		if(m_ColData.Delta.isEmpty()){
#if 1
			m_ColData.Delta = 1;
#else
			if((int)m_ColData.Begin <= (int)m_ColData.End){
				m_ColData.Delta = 1;
			}else{
				m_ColData.Delta = -1;
			}
#endif
		}
		m_ColMap.setSeriesInfo(m_ColData.Begin, m_ColData.Delta, m_ColData.End);
		break;
	default:
		;
	}

}

// 【注意】この関数の用途は特にないものと思われる 
bool IndexConv::isConstant() const
{
	bool isConst = m_pReferenced->isConstant();
	if(m_isInverseTrans){
		assert(m_pReferer != NULL);
		isConst = (isConst && m_pReferer->isConstant());
	}

	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
		isConst = (isConst && m_RowData.IndexTable.isConstant());
		break;
	case ICT_SERIES:
		isConst = (isConst && m_RowData.Begin.isConstant());
		isConst = (isConst && m_RowData.Delta.isConstant());
		isConst = (isConst && m_RowData.End.isConstant());
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
		isConst = (isConst && m_ColData.IndexTable.isConstant());
		break;
	case ICT_SERIES:
		isConst = (isConst && m_ColData.Begin.isConstant());
		isConst = (isConst && m_ColData.Delta.isConstant());
		isConst = (isConst && m_ColData.End.isConstant());
		break;
	default:
		;
	}

	return isConst;
}

bool IndexConv::isFixed() const
{
	bool isFixed = m_pReferenced->isFixed();
	if(m_isInverseTrans){
		assert(m_pReferer != NULL);
		isFixed = (isFixed && m_pReferer->isFixed());
	}

	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
		isFixed = (isFixed && m_RowData.IndexTable.isFixed());
		break;
	case ICT_SERIES:
		isFixed = (isFixed && m_RowData.Begin.isFixed());
		isFixed = (isFixed && m_RowData.Delta.isFixed());
		isFixed = (isFixed && m_RowData.End.isFixed());
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
		isFixed = (isFixed && m_ColData.IndexTable.isFixed());
		break;
	case ICT_SERIES:
		isFixed = (isFixed && m_ColData.Begin.isFixed());
		isFixed = (isFixed && m_ColData.Delta.isFixed());
		isFixed = (isFixed && m_ColData.End.isFixed());
		break;
	default:
		;
	}

	return isFixed;
}

void IndexConv::simplify()
{
	m_pReferenced->simplify();
	if(m_isInverseTrans){
		assert(m_pReferer != NULL);
		m_pReferer->simplify();
	}

	switch(m_RowMap.getMappingType()){
	case ICT_TABLE:
		m_RowData.IndexTable.simplify();
		break;
	case ICT_SERIES:
		m_RowData.Begin.simplify();
		m_RowData.Delta.simplify();
		m_RowData.End.simplify();
		break;
	default:
		;
	}
	switch(m_ColMap.getMappingType()){
	case ICT_TABLE:
		m_ColData.IndexTable.simplify();
		break;
	case ICT_SERIES:
		m_ColData.Begin.simplify();
		m_ColData.Delta.simplify();
		m_ColData.End.simplify();
		break;
	default:
		;
	}
}

void IndexConv::copy(const IndexConv& src)
{
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

void IndexConv::getRefererSize(int& row, int& col) const
{
	// 逆変換の場合しか利用されない
	if(m_isInverseTrans){
		assert(m_pReferer != NULL);
		row = m_pReferer->getNumRow();
		col = m_pReferer->getNumColumn();
	}
}

void IndexConv::getReferencedSize(int& row, int& col) const
{
	assert(m_pReferenced != NULL);
	row = m_pReferenced->getNumRow();
	col = m_pReferenced->getNumColumn();
}

void IndexConv::getReferencedArea(int& begin_row, int& begin_col, int& size_row, int& size_col) const
{
	// このインデックス変換によってRefererから参照されるサイズ（起点からマイナス方向の場合は負の値）
	getRefererSize(size_row, size_col);
	size_row *= (int)m_RowData.Delta;
	size_col *= (int)m_ColData.Delta;

	// 参照される起点
	begin_row = m_RowMap.getRangeBeginIndex();
	begin_col = m_ColMap.getRangeBeginIndex();
}
