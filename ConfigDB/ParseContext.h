#ifndef DEF_PARSE_CONTEXT
#define DEF_PARSE_CONTEXT

class UnivType;
class IndexConv;
class BaseDB;
class TokenAnaly;
enum TokenKind;

#include "CdbException.h"

#define INVALID  (-1)

// �R���e�L�X�g�ۑ��\����
template<class T>
class ParseContext
{
public:
	typedef T *T_PTR;

	ParseContext(T_PTR& pContextContainer) : m_pContextContainer(pContextContainer){}
	~ParseContext(){
		m_pContextContainer = m_pPrevContext;	// �ȑO�̃R���e�L�X�g�ɖ߂�
	}

protected:
	void init(){
#if 0	// 090723 �����p�����[�^���p���͔p�~
		if(m_pContextContainer != NULL){
			memcpy(this, m_pContextContainer, sizeof(T));	// �ȑO�̓��e���R�s�[����(m_pContextContainer�ւ̎Q�ƃ|�C���^�́A��ɓ����l�ł��邽�߁A���̂悤�ȏ㏑���R�s�[���\�ł���)
			//*static_cast<T_PTR>(this) = *m_pContextContainer;
		}else{
			setDefault();	// �f�t�H���g�l�̐ݒ�
		}
#else
		if(m_pContextContainer == NULL){
			setDefault();	// �f�t�H���g�l�̐ݒ�
		}
#endif
		m_pPrevContext = m_pContextContainer;				// �ȑO�̃R���e�L�X�g��ۑ�
		m_pContextContainer = static_cast<T_PTR>(this);		// �V�����R���e�L�X�g�ɐ؂�ւ���
	}
	virtual void setDefault() = 0;
	T_PTR GetPrevContext() const { return m_pPrevContext; }

private:
	T_PTR& m_pContextContainer;
	T_PTR m_pPrevContext;	// �ȑO�̃R���e�L�X�g��ۑ����Ă���
};

class SubsetContext : public ParseContext<SubsetContext>
{
public:
	SubsetContext(SubsetContext*& p) : ParseContext<SubsetContext>(p){
		if(p != NULL){
			m_pDB = p->m_pDB;
			m_pSubset = p->m_pSubset;
			m_bSubsetOwnerSearch = p->m_bSubsetOwnerSearch;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_pDB = NULL;
		m_bSubsetOwnerSearch = true;
	}

private:
	BaseDB *m_pDB;				// ���̃R���e�L�X�g�ɂ����鑀��Ώ�Subset��DB(����Ώ�Subset��������i���s���Ɍ���j�̏ꍇ��NULL)
	UnivType *m_pSubset;		// ���̃R���e�L�X�g�ɂ����鑀��Ώ�Subset
	bool m_bSubsetOwnerSearch;	// ���L�҂�DB�܂Ō������s�����ǂ���

	UnivType& runtimeGetFromSubset(const char *pName);

public:
	// DB�ւ̃A�N�Z�X
	void setRootDB(BaseDB *pDB);			// ���[�gDB�̐ݒ�i�ŏ��ɕK���s���Ă����j
	void createSubset(const char *pName);	// �w�肳�ꂽ���O�ŃT�u�Z�b�g���쐬���A����𑀍�ΏۂƂ���
	void createSubset(UnivType *pObj);		// �w�肳�ꂽUnivType�I�u�W�F�N�g�ɃT�u�Z�b�g���쐬���A����𑀍�ΏۂƂ���
	void setSubset(UnivType *pSubset);		// �T�u�Z�b�g���w�肳�ꂽ���̂ɐ؂�ւ���
	UnivType& getSubset();					// ���݂̑���Ώ�subset�̎擾
	void disableOwnerSearch(){ m_bSubsetOwnerSearch = false; }	// �ȍ~�̃T�u�Z�b�g���L�҂ւ̌����𖳌�������
	// �ȉ��ABaseDB�̃��b�p�[
	UnivType& addNew(const char *pName, CDB_ERR& error);
	UnivType& addNew(const char *pName);
	UnivType& addNew();
	CDB_ERR rename(UnivType *pTarget, const char *pName);
	UnivType& getUnsafeUnivType(const char *pName);
};

class BraceContext : public ParseContext<BraceContext>
{
public:
	BraceContext(BraceContext*& p) : ParseContext<BraceContext>(p) {
		if(p != NULL){
			m_currentBrace = p->m_currentBrace;
			m_countTerm = p->m_countTerm;
			m_countPrimitive = p->m_countPrimitive;
			m_braceLevel = p->m_braceLevel;
		}
		init();
	}

protected:
	virtual void setDefault();

public:
	enum TokenKind m_currentBrace;		// ���݂̎��������Ă��銇��
	short int m_countTerm;				// �����̉��Ԗڂ̍��̒��ł��邩(�ŏ��̗v�f��0��)
	short int m_countPrimitive;			// �����̉��Ԗڂ̊�{�v�f�ł��邩(�ŏ��̗v�f��0��)
	short int m_braceLevel;				// �\����͂̃��x��(�ŏ�ʂ��牽�Ԗڂ̊��ʂ̒���)
};

class SentenceContext : public ParseContext<SentenceContext>
{
public:
	SentenceContext(SentenceContext*& p) : ParseContext<SentenceContext>(p) {
		if(p != NULL){
			m_isReference = p->m_isReference;
			m_isConstant = p->m_isConstant;
			m_isInsideRight = p->m_isInsideRight;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_isReference = false;
		m_isConstant = true;
		m_isInsideRight = true;
	}

public:
	bool m_isReference;					// �Q�ƂȂ�true(true�̂Ƃ��Am_isConstant�̈Ӗ��͂Ȃ��Ȃ�)
	bool m_isConstant;					// �萔�Ȃ�true
	bool m_isInsideRight;				// �E�ӎ����Ȃ�true
};

class IndexContext : public ParseContext<IndexContext>
{
public:
	IndexContext(IndexContext*& p) : ParseContext<IndexContext>(p) {
		if(p != NULL){
			m_saveTokenPos = p->m_saveTokenPos;
			m_isInsideArrayIndex = p->m_isInsideArrayIndex;
			m_pTargetArray = p->m_pTargetArray;
			m_pIndexConv = p->m_pIndexConv;
			m_arrayDimensionId = p->m_arrayDimensionId;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_isInsideArrayIndex = false;
		m_pTargetArray = NULL;
	}

private:
	int m_saveTokenPos;					// �g�[�N���ʒu�ۑ��p�ϐ�

public:
	bool m_isInsideArrayIndex;			// �z��C���f�b�N�X���ł���Ȃ�true
	const UnivType *m_pTargetArray;		// �z��C���f�b�N�X���ł���ꍇ�A�Ώۂ̔z��ւ̃|�C���^
	const IndexConv *m_pIndexConv;		// �z��C���f�b�N�X���ł���ꍇ�A�C���f�b�N�X�ϊ��I�u�W�F�N�g�ւ̃|�C���^
	short int m_arrayDimensionId;		// �z��C���f�b�N�X���ʎq

	void saveCurrentTokenPos(TokenAnaly& token);	// ���݂̃g�[�N���ʒu��ۑ�
	void loadTokenPos(TokenAnaly& token);			// �ۑ�����Ă����g�[�N���ʒu�ɖ߂�
};

class CatContext : public ParseContext<CatContext>
{
public:
	CatContext(CatContext*& p) : ParseContext<CatContext>(p){
		init();
		// �����p�����[�^���p���͍s��Ȃ� 
		// (�����context�����ɂă��Z�b�g�����)
		setDefault();
	}

protected:
	virtual void setDefault(){
		m_bNestedMatrix = false;
	}

private:
	bool m_bNestedMatrix;			// ���̃R���e�L�X�g�ɂ�����v�f���Acat�ɂ�����1�v�f�Ƃ��ăl�X�g����

public:
	// ���Z�b�g(�f�t�H���g��Ԃɖ߂�)
	void reset(){ setDefault(); }
#if 0
	// �A�N�Z�X(1�O�̃R���e�L�X�g�ɃA�N�Z�X����)
	void setNested(bool x){
		if(GetPrevContext() != NULL){
			GetPrevContext()->m_bNestedMatrix = x;
		}
	}
#endif
	// �A�N�Z�X(���݂̃R���e�L�X�g�ɃA�N�Z�X����)
	void setNested(bool x){ m_bNestedMatrix = x; }
	bool getNested(){ return m_bNestedMatrix; }

};

#endif
