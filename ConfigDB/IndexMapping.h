#ifndef DEF_INDEX_MAPPING
#define DEF_INDEX_MAPPING

#define INDEX_BEGIN    (-9999)	// �\���ubegin�v�����������Ƃ�����
#define INDEX_END      (-8888)	// �\���uend�v�����������Ƃ�����
#define INDEX_INVALID  (-7777)	// �����ȃC���f�b�N�X�ԍ�

enum IndexConvType{
	ICT_NONE = 0,
	ICT_REVERSE,
	ICT_SERIES,
	ICT_TABLE,
	ICT_TABLE_SINGLE,
	ICT_INVALID
};

class IndexMapping
{
private:
	enum IndexConvType m_eType;
	int m_nElementsDomain;	// ��`��i�ϊ������W�n�j�v�f��
	int m_nElementsRange;	// �l��i�ϊ�����W�n�j�v�f��

	union tagMapInfo{
		struct tagSeries{
			int nBegin;
			int nDelta;
			int nEnd;
			int nFixedBegin;
			int nFixedDelta;
		} series;
		struct tagTable{
			int nIndexTable;
			int *pIndexTable;
			int nFixedIndexTable;
			int *pFixedIndexTable;
		} table;
		struct tagTableSingle{
			int nIndex;
			int nFixedIndex;
		} table_single;
	} m_MapInfo;

private:
	void clear();
	inline int fixIndexVal(int index);
	int *reallocIndexTable(int nElements);
	int *reallocFixedIndexTable(int nElements);

public:
	IndexMapping();
	~IndexMapping();

	// ���ϊ�
	void update();	
	void convert(int& n) const;

	// �t�ϊ�
	void updateInverse();
	void convertInverse(int& n) const;

	void setMappingType(enum IndexConvType type){ clear(); m_eType = type; }
	enum IndexConvType getMappingType() const { return m_eType; }

	int getNumElementsDomain() const { return m_nElementsDomain; }
	int getNumElementsRange() const { return m_nElementsRange; }
	void setNumElementsRange(int n){ m_nElementsRange = n; }

	void setSeriesInfo(int begin, int delta, int end);
	int *setIndexTable(int nElements);
	void setIndexTableSingle(int nIndex);

	int getRangeBeginIndex() const;
};

#endif
