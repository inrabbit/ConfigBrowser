#ifndef DEF_INDEX_CONV
#define DEF_INDEX_CONV

#include "UnivType.h"
#include "IndexMapping.h"
#include "RequireUpdateObj.h"

// 配列インデックス識別子
#define DIM_ROW 0	// 行指定
#define DIM_COL 1	// 列指定
#define DIM_VEC 0	// ベクトル指定（DIM_ROWと同じ値にしておくこと）

class IndexConv : public RequireUpdateObj
{
private:
	bool m_IsTranspose;				// 転置を行うかどうか
	bool m_IsVectorAccess;			// vec演算子アクセスを行うかどうか
	const bool m_isInverseTrans;	// 逆変換モードかどうか
	const UnivType *m_pReferer;		// 参照元（変換前座標系におけるオブジェクト）
	const UnivType *m_pReferenced;	// 被参照（変換後座標系におけるオブジェクト）

	IndexMapping m_RowMap;		// 行方向のインデックス写像オブジェクト
	IndexMapping m_ColMap;		// 列方向のインデックス写像オブジェクト

	struct IndexConvData{
		UnivType IndexTable;
		UnivType Begin;
		UnivType End;
		UnivType Delta;
	};

	struct IndexConvData m_RowData;	// 行方向のパラメータ実体
	struct IndexConvData m_ColData;	// 列方向のパラメータ実体

	void updateParameters();

public:
	IndexConv(bool isInverseTrans = false);
	~IndexConv();

	void convert(int& row, int& col) const;
	void convert(int& n) const;
	int getNumRow() const { return m_IsTranspose ? m_ColMap.getNumElementsDomain() : m_RowMap.getNumElementsDomain(); }
	int getNumColumn() const { return m_IsTranspose ? m_RowMap.getNumElementsDomain() : m_ColMap.getNumElementsDomain(); }
	int getNumElements() const { return m_RowMap.getNumElementsDomain() * m_ColMap.getNumElementsDomain(); }

	void setConvertReferer(const UnivType *pObj){ m_pReferer = pObj; }	// 逆変換モードを利用しない場合は、登録の必要なし
	void setConvertReferenced(const UnivType *pObj){ m_pReferenced = pObj; }
	void getRefererSize(int& row, int& col) const;
	void getReferencedSize(int& row, int& col) const;
	void getReferencedArea(int& begin_row, int& begin_col, int& size_row, int& size_col) const;

	void setTranspose(bool x){ m_IsTranspose = x; }
	void setVectorAccess(bool x){ m_IsVectorAccess = x; }
	void setConvTypeMatrix(int nDim, enum IndexConvType type){ m_IsVectorAccess = false; (nDim == DIM_ROW) ? m_RowMap.setMappingType(type) : m_ColMap.setMappingType(type); }
	void setConvTypeVector(enum IndexConvType type){ m_IsVectorAccess = true; m_RowMap.setMappingType(type); }

	bool getVectorAccess() const { return m_IsVectorAccess; }

	UnivType& getIndexTableMatrix(int nDim){ return (nDim == DIM_ROW) ? m_RowData.IndexTable : m_ColData.IndexTable; }
	UnivType& getIndexTableVector(){ return m_RowData.IndexTable; }
	UnivType& getIndexSeriesBeginMatrix(int nDim){ return (nDim == DIM_ROW) ? m_RowData.Begin : m_ColData.Begin; }
	UnivType& getIndexSeriesBeginVector(){ return m_RowData.Begin; }
	UnivType& getIndexSeriesDeltaMatrix(int nDim){ return (nDim == DIM_ROW) ? m_RowData.Delta : m_ColData.Delta; }
	UnivType& getIndexSeriesDeltaVector(){ return m_RowData.Delta; }
	UnivType& getIndexSeriesEndMatrix(int nDim){ return (nDim == DIM_ROW) ? m_RowData.End : m_ColData.End; }
	UnivType& getIndexSeriesEndVector(){ return m_RowData.End; }

	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);
	virtual bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	virtual void update();
	bool isConstant() const;
	bool isFixed() const;
	void simplify();
	void copy(const IndexConv& src);

};

#endif
