#ifndef DEF_INDEX_CONV
#define DEF_INDEX_CONV

#include "UnivType.h"
#include "IndexMapping.h"
#include "RequireUpdateObj.h"

// �z��C���f�b�N�X���ʎq
#define DIM_ROW 0	// �s�w��
#define DIM_COL 1	// ��w��
#define DIM_VEC 0	// �x�N�g���w��iDIM_ROW�Ɠ����l�ɂ��Ă������Ɓj

class IndexConv : public RequireUpdateObj
{
private:
	bool m_IsTranspose;				// �]�u���s�����ǂ���
	bool m_IsVectorAccess;			// vec���Z�q�A�N�Z�X���s�����ǂ���
	const bool m_isInverseTrans;	// �t�ϊ����[�h���ǂ���
	const UnivType *m_pReferer;		// �Q�ƌ��i�ϊ��O���W�n�ɂ�����I�u�W�F�N�g�j
	const UnivType *m_pReferenced;	// ��Q�Ɓi�ϊ�����W�n�ɂ�����I�u�W�F�N�g�j

	IndexMapping m_RowMap;		// �s�����̃C���f�b�N�X�ʑ��I�u�W�F�N�g
	IndexMapping m_ColMap;		// ������̃C���f�b�N�X�ʑ��I�u�W�F�N�g

	struct IndexConvData{
		UnivType IndexTable;
		UnivType Begin;
		UnivType End;
		UnivType Delta;
	};

	struct IndexConvData m_RowData;	// �s�����̃p�����[�^����
	struct IndexConvData m_ColData;	// ������̃p�����[�^����

	void updateParameters();

public:
	IndexConv(bool isInverseTrans = false);
	~IndexConv();

	void convert(int& row, int& col) const;
	void convert(int& n) const;
	int getNumRow() const { return m_IsTranspose ? m_ColMap.getNumElementsDomain() : m_RowMap.getNumElementsDomain(); }
	int getNumColumn() const { return m_IsTranspose ? m_RowMap.getNumElementsDomain() : m_ColMap.getNumElementsDomain(); }
	int getNumElements() const { return m_RowMap.getNumElementsDomain() * m_ColMap.getNumElementsDomain(); }

	void setConvertReferer(const UnivType *pObj){ m_pReferer = pObj; }	// �t�ϊ����[�h�𗘗p���Ȃ��ꍇ�́A�o�^�̕K�v�Ȃ�
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
