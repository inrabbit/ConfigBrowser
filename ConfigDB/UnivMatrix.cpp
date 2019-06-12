#include "UnivMatrix.h"
#include "OwnedUnivType.h"

UnivMatrix::UnivMatrix(int row, int col) : m_nRow(0), m_nColumn(0), m_nElements(0), m_pElements(NULL)
{
	allocate(row, col);
}

void UnivMatrix::allocate(int row, int col)
{
	int n = row * col;
	if(m_nElements != n){
		clear();
		m_nElements = n;
		m_pElements = new OwnedUnivType[m_nElements];	// 数式による所有情報を付加する可能性があるため、全てOwnedUnivType型とする
	}
	m_nRow = row;
	m_nColumn = col;
}

void UnivMatrix::clear()
{
	delete[] m_pElements;
	m_nRow = m_nColumn = m_nElements = 0;
	m_pElements = NULL;
}

bool UnivMatrix::isFixed() const
{
	bool isFixed = true;
	for(int i = 0; i < m_nElements; i++){
		if(!m_pElements[i].isFixed()) isFixed = false;
	}
	return isFixed;
}

#ifdef DEBUG_PRINT
#define TAB_INDENT(nIndentLevel) { for(int n = 0; n < nIndentLevel; n++) System::Console::Write("\t"); }
void UnivMatrix::debugPrintStructure(int nIndentLevel) const
{
	System::Console::WriteLine("MATRIX("+m_nRow+","+m_nColumn+"){");
	for(int i = 0; i < m_nElements; i++){
		TAB_INDENT(nIndentLevel);
		System::Console::Write("("+(i%m_nRow)+","+(i/m_nRow)+")=");
		m_pElements[i].debugPrintStructure(nIndentLevel + 1);
		if(i !=(m_nElements - 1)){
			System::Console::WriteLine(",");
		}
	}
	System::Console::Write("}");
}
#endif
