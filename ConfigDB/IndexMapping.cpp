#include "IndexMapping.h"
#include "CdbException.h"
#include "CommonUtils.h"
#include <cassert>

IndexMapping::IndexMapping()
{
	m_eType = ICT_NONE;

	m_nElementsDomain = 0;
	m_nElementsRange = 0;
}

IndexMapping::~IndexMapping()
{
	clear();
}

void IndexMapping::clear()
{
	if(m_eType == ICT_TABLE){
		delete[] m_MapInfo.table.pIndexTable;
		delete[] m_MapInfo.table.pFixedIndexTable;
	}
	m_MapInfo.table.nIndexTable = 0;
	m_MapInfo.table.pIndexTable = NULL;
	m_MapInfo.table.nFixedIndexTable = 0;
	m_MapInfo.table.pFixedIndexTable = NULL;
	m_eType = ICT_NONE;
}

void IndexMapping::update()
{
	switch(m_eType){
	case ICT_NONE:
	case ICT_REVERSE:
		m_nElementsDomain = m_nElementsRange;
		break;
	case ICT_TABLE:
		{
			m_nElementsDomain = m_MapInfo.table.nIndexTable;
			int *pFixedIndexTable = reallocFixedIndexTable(m_nElementsDomain);
			bool exceed = false;
			for(int i = 0; i < m_nElementsDomain; i++){
				pFixedIndexTable[i] = fixIndexVal(m_MapInfo.table.pIndexTable[i]);
				if(pFixedIndexTable[i] >= m_nElementsRange) exceed = true;
			}
			if(exceed) throw CdbException(CDB_INDEX_EXCEED);
		}
		break;
	case ICT_TABLE_SINGLE:
		m_nElementsDomain = 1;
		m_MapInfo.table_single.nFixedIndex = fixIndexVal(m_MapInfo.table_single.nIndex);
		if(m_MapInfo.table_single.nFixedIndex >= m_nElementsRange) throw CdbException(CDB_INDEX_EXCEED);
		break;
	case ICT_SERIES:
		{
			int begin = fixIndexVal(m_MapInfo.series.nBegin);
			int delta = m_MapInfo.series.nDelta;
			int end = fixIndexVal(m_MapInfo.series.nEnd);
			int large = (begin > end) ? begin : end;
			int small = (begin < end) ? begin : end;
			int range = large - small;
			int abs_delta = (delta > 0) ? delta : -delta;
			int max_index = range / abs_delta;
			if(begin < 0) throw CdbException(CDB_INDEX_NEGATIVE);
			if(begin + delta * max_index >= m_nElementsRange) throw CdbException(CDB_INDEX_EXCEED);
			m_nElementsDomain = max_index + 1;
			m_MapInfo.series.nFixedBegin = begin;
			m_MapInfo.series.nFixedDelta = delta;
		}
		break;
	default:
		assert(false);
	}
}

void IndexMapping::convert(int& n) const
{
	switch(m_eType){
	case ICT_NONE:
		break;
	case ICT_REVERSE:
		n = m_nElementsRange - n - 1;
		assert((0 <= n) && (n < m_nElementsRange));
		break;
	case ICT_TABLE:
		n = m_MapInfo.table.pFixedIndexTable[n];
		break;
	case ICT_TABLE_SINGLE:
		n = m_MapInfo.table_single.nIndex;
		break;
	case ICT_SERIES:
		n = m_MapInfo.series.nFixedBegin + n * m_MapInfo.series.nFixedDelta;
		assert((0 <= n) && (n < m_nElementsRange));
		break;
	case ICT_INVALID:
	default:
		assert(false);
	}
}

void IndexMapping::updateInverse()
{
	// m_nElementsRangeは上位から指定されるため、ここでは決定しない
	switch(m_eType){
	case ICT_NONE:
	case ICT_REVERSE:
		assert(m_nElementsRange == m_nElementsDomain);
		break;
	case ICT_TABLE:
		{
			int i;
			int *pFixedIndexTable = reallocFixedIndexTable(m_nElementsRange);
			for(i = 0; i < m_nElementsRange; i++) pFixedIndexTable[i] = INDEX_INVALID;
			for(i = 0; i < m_nElementsDomain; i++){
				int x = m_MapInfo.table.pIndexTable[i];
				if(pFixedIndexTable[x] != INDEX_INVALID){
					throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
				}
				pFixedIndexTable[x] = i;
			}
		}
		break;
	case ICT_TABLE_SINGLE:
		// 特に準備は必要としない
		break;
	case ICT_SERIES:
		{
			int begin = fixIndexVal(m_MapInfo.series.nBegin);
			int delta = m_MapInfo.series.nDelta;
			int end = fixIndexVal(m_MapInfo.series.nEnd);
			int large = (begin > end) ? begin : end;
			int small = (begin < end) ? begin : end;
			int range = large - small + 1;
			int abs_delta = (delta > 0) ? delta : -delta;
			int max_index = range / abs_delta;
			if(begin < 0) throw CdbException(CDB_INDEX_NEGATIVE);
			if(begin + delta * max_index >= m_nElementsRange) throw CdbException(CDB_INDEX_EXCEED);
			m_nElementsDomain = max_index + 1;
			m_MapInfo.series.nFixedBegin = begin;
			m_MapInfo.series.nFixedDelta = delta;

		}
		break;
	default:
		assert(false);
	}
}

void IndexMapping::convertInverse(int& n) const
{
	switch(m_eType){
	case ICT_NONE:
		break;
	case ICT_REVERSE:
		n = m_nElementsDomain - n - 1;
		assert((0 <= n) && (n < m_nElementsDomain));
		break;
	case ICT_TABLE:
		n = m_MapInfo.table.pFixedIndexTable[n];
		break;
	case ICT_TABLE_SINGLE:
		n = 0;
		break;
	case ICT_SERIES:
		if(((n - m_MapInfo.series.nFixedBegin) % m_MapInfo.series.nFixedDelta) == 0){
			n = (n - m_MapInfo.series.nFixedBegin) / m_MapInfo.series.nFixedDelta;
			assert((0 <= n) && (n < m_nElementsDomain));
		}else{
			n = INDEX_INVALID;
		}
		break;
	case ICT_INVALID:
	default:
		assert(false);
	}
}

inline int IndexMapping::fixIndexVal(int index)
{
	if(index == INDEX_BEGIN) return 0;
	if(index == INDEX_END) return m_nElementsRange - 1;
	return index;
}

int *IndexMapping::reallocIndexTable(int nElements)
{
	if(m_eType != ICT_TABLE) return NULL;

	if(m_MapInfo.table.nIndexTable != nElements){
		delete[] m_MapInfo.table.pIndexTable;
		if(nElements > 0){
			m_MapInfo.table.nIndexTable = nElements;
			m_MapInfo.table.pIndexTable = new int[nElements];
		}else{
			m_MapInfo.table.nIndexTable = 0;
			m_MapInfo.table.pIndexTable = NULL;
		}
	}

	return m_MapInfo.table.pIndexTable;
}

int *IndexMapping::reallocFixedIndexTable(int nElements)
{
	if(m_eType != ICT_TABLE) return NULL;

	if(m_MapInfo.table.nFixedIndexTable != nElements){
		delete[] m_MapInfo.table.pFixedIndexTable;
		if(nElements > 0){
			m_MapInfo.table.nFixedIndexTable = nElements;
			m_MapInfo.table.pFixedIndexTable = new int[nElements];
		}else{
			m_MapInfo.table.nFixedIndexTable = 0;
			m_MapInfo.table.pFixedIndexTable = NULL;
		}
	}

	return m_MapInfo.table.pFixedIndexTable;
}

void IndexMapping::setSeriesInfo(int begin, int delta, int end)
{
	setMappingType(ICT_SERIES);
	m_MapInfo.series.nBegin = begin;
	m_MapInfo.series.nDelta = delta;
	m_MapInfo.series.nEnd = end;
}

int *IndexMapping::setIndexTable(int nElements)
{
	setMappingType(ICT_TABLE);
	reallocFixedIndexTable(0);
	return reallocIndexTable(nElements);
}

void IndexMapping::setIndexTableSingle(int nIndex)
{
	setMappingType(ICT_TABLE_SINGLE);
	m_MapInfo.table_single.nIndex = nIndex;
}

int IndexMapping::getRangeBeginIndex() const
{
	// 参照される起点
	switch(m_eType){
	case ICT_NONE:
		return INDEX_BEGIN;
	case ICT_REVERSE:
		return INDEX_END;
	case ICT_TABLE:
		assert(m_MapInfo.table.nIndexTable > 0);
		return m_MapInfo.table.pIndexTable[0];
	case ICT_TABLE_SINGLE:
		return m_MapInfo.table_single.nIndex;
	case ICT_SERIES:
		return m_MapInfo.series.nBegin;
	default:
		assert(false);
		return INDEX_INVALID;
	}
}
