#ifndef DEF_INDEX_MAPPING
#define DEF_INDEX_MAPPING

#define INDEX_BEGIN    (-9999)	// 予約語「begin」が入ったことを示す
#define INDEX_END      (-8888)	// 予約語「end」が入ったことを示す
#define INDEX_INVALID  (-7777)	// 無効なインデックス番号

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
	int m_nElementsDomain;	// 定義域（変換元座標系）要素数
	int m_nElementsRange;	// 値域（変換後座標系）要素数

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

	// 順変換
	void update();	
	void convert(int& n) const;

	// 逆変換
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
