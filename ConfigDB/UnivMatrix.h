#ifndef DEF_UNIV_MATRIX
#define DEF_UNIV_MATRIX

#include "UnivType.h"
#include "OwnedUnivType.h"
#include "CdbException.h"
#include "CommonUtils.h"
#include <cassert>

class UnivType;

class UnivMatrix
{
private:
	int m_nRow;
	int m_nColumn;
	int m_nElements;
	OwnedUnivType *m_pElements;
protected:
	friend UnivType;	// UnivMatrixインスタンスはUnivTypeしか作成できないことを保障する 
	UnivMatrix() : m_nRow(0), m_nColumn(0), m_nElements(0), m_pElements(NULL) {}
	UnivMatrix(int row, int col);
	~UnivMatrix(){ clear(); }
public:
	UnivType& getAt(int n){
		assert(n < m_nElements);
		return m_pElements[n];
	}
	UnivType& getAt(int row, int col){
		int n = m_nRow * col + row;
		if(n >= m_nElements) throw CdbException(CDB_INDEX_EXCEED);
		return m_pElements[n];
	}
	const UnivType& getAt(int n) const {
		if(n >= m_nElements) throw CdbException(CDB_INDEX_EXCEED);
		return *(const UnivType *)&m_pElements[n];
	}
	const UnivType& getAt(int row, int col) const {
		int n = m_nRow * col + row;
		if(n >= m_nElements) throw CdbException(CDB_INDEX_EXCEED);
		return *(const UnivType *)&m_pElements[n];
	}
	int getNumRow() const { return m_nRow; }
	int getNumColumn() const { return m_nColumn; }
	int getNumElements() const { return m_nRow * m_nColumn; }
	void allocate(int row, int col);
	void clear();
	bool isFixed() const;

#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel) const;
#endif
};

#endif
