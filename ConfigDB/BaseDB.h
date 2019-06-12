#ifndef DEF_BASE_DB
#define DEF_BASE_DB

#include "CommonUtils.h"
#include "WrappedMap.h"
#include "WrappedVector.h"
#include "UnivType.h"
#include "CdbException.h"

class RequireUpdateObj;

class BaseDB
{
private:
	// �ڑ���̃��[�gDB
	BaseDB *m_pRoot;
	// ���g���T�u�Z�b�g�ł���ꍇ�A���L�҂�DB
	BaseDB *m_pSubsetOwner;
	// ����DB���X�g
	WrappedVector<BaseDB *> m_searchList;
	// �X�V���X�g(�y���Ӂz�T�u�Z�b�g�͍X�V���X�g���������A�T�u�Z�b�g���L�҂̍X�V���X�g�𗘗p������̂Ƃ���)
	WrappedVector<RequireUpdateObj *> m_updateList;

	// DB�̐ڑ��Ɋւ������
	void connectNamedObjects(BaseDB *pDB);
	void disconnectNamedObjects(BaseDB *pDB);
	bool isConnected(BaseDB *pDB) const;
	static void createLink(BaseDB *pFrom, BaseDB *pTo);
	static void deleteLink(BaseDB *pFrom, BaseDB *pTo);

	// ���g�ɑ΂��錟���v���̏���
	UnivType *search(const char *pName, bool bSubsetOwnerSearch = true);
	// ���g�Ǝ��g������DB�̌���
	UnivType *searchSelf(const char *pName, BaseDB *pSearchExcept = NULL);
	// ���g��DB�Ɏw�肵�����O�ŗv�f���ǉ��ł��邩�̊m�F
	bool confirmAddEnable(const char *pName);

protected:
	// �Q�ƕԂ����G���[�̍ۂɕԂ��_�~�[
	static UnivType m_dummy;

	WrappedMap m_map;
	WrappedVector<UnivType *> m_nonameList;

public:
	// ���O�t���V�K�I�u�W�F�N�g�̓o�^
	UnivType& addNew(const char *pName, CDB_ERR& error);
	// ���O�t���V�K�I�u�W�F�N�g�̓o�^�i�G���[���O�ŕԂ��j
	UnivType& addNew(const char *pName);
	// ���O�Ȃ��V�K�I�u�W�F�N�g�̓o�^�i�G���[���O�ŕԂ��j
	UnivType& addNew();

	// �T�u�Z�b�g�̐V�K�o�^�i�G���[���O�ŕԂ��j
	BaseDB *createSubset(UnivType *pObj);

public:
	BaseDB();
	~BaseDB();

	// DB�̐ڑ��i�����͈͂̒ǉ��j
	CDB_ERR connectDB(BaseDB *pDB);
	// DB�̐ڑ�����
	CDB_ERR disconnectDB(BaseDB *pDB = NULL);

	// �R���e�i�N���X���擾����i�f�o�b�O�p�ꗗ�\���̂��߁j
	WrappedMap *getContainer(){ return &m_map; }

	// �S�v�f�̏���
	void clear();
	// �L�[�̑��݊m�F
	bool isExistKey(const char *pName, bool internal = false) const;
	// �v�f�̑��f�[�^�x�[�X����̃R�s�[
	CDB_ERR copy(const char *pName, const BaseDB*  pSource);
	// �S�v�f�̑��f�[�^�x�[�X����̃R�s�[
	CDB_ERR copy(const BaseDB*  pSource);
	CDB_ERR evalCopy(const BaseDB*  pSource);
	// �v�f�̑��f�[�^�x�[�X����̈ړ�
	CDB_ERR move(const char *pName, BaseDB*  pSource);
	// �v�f�̍폜
	CDB_ERR remove(const char *pName);
	// �v�f�̖��O�̕ύX
	CDB_ERR rename(const char *pOldName, const char *pNewName);
	// ���O�Ȃ��I�u�W�F�N�g�ɖ��O������i���͂��̋t�j
	CDB_ERR rename(UnivType *pTarget, const char *pName);

	// UnivType�^�i�G���[���O�ŕԂ��j
	UnivType& getUnivType(const char *pName);	// �萔�����ɂ̓A�N�Z�X�ł��Ȃ�
	const UnivType& getConstUnivType(const char *pName) const;
	// UnivType�^�i�G���[�R�[�h�ŕԂ��j
	UnivType& getUnivType(const char *pName, CDB_ERR& error);	// �萔�����ɂ̓A�N�Z�X�ł��Ȃ�
	const UnivType& getConstUnivType(const char *pName, CDB_ERR& error) const;
	// UnivType�^�i�G���[���O�ŕԂ��j
	// �萔�����ł��擾�ł���(�������A���e�����������悤�Ƃ������_�ŗ�O)
	UnivType& getUnsafeUnivType(const char *pName, bool bSubsetOwnerSearch = true);

	// �����l
	CDB_ERR getInt(const char *pName, int& x);
	CDB_ERR setInt(const char *pName, int x);
	// �����Ȃ������l
	CDB_ERR getUInt(const char *pName, unsigned int& x);
	CDB_ERR setUInt(const char *pName, unsigned int x);
	// �����l
	CDB_ERR getReal(const char *pName, UnivType::Real& x);
	CDB_ERR setReal(const char *pName, UnivType::Real x);
	// �u�[���l
	CDB_ERR getBool(const char *pName, bool& x);
	CDB_ERR setBool(const char *pName, bool x);
	// �|�C���^�l
	CDB_ERR getPtr(const char *pName, void*& p);
	CDB_ERR setPtr(const char *pName, void* p);
	// ������
	CDB_ERR getString(const char *pName, const char*& p);
	CDB_ERR setString(const char *pName, const char* p);
	// �o�C�i���f�[�^
	CDB_ERR getBinary(const char *pName, void*& p, size_t& size);
	CDB_ERR setBinary(const char *pName, const void* p, size_t size);
	// �z��I�u�W�F�N�g�i�ߋ��o�[�W�����Ƃ̌݊����̂��߁B���p�͐�������Ȃ��B�j
	CDB_ERR getArrayObj(const char *pName, void*& p, size_t& size);
	CDB_ERR setArrayObj(const char *pName, const void* p, size_t size);

	// �X�V�ʒm�̔��s
	void issueNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	// �X�V�ʒm�̎󂯎��
	bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	void update();
	// �Q�Ɛ�̒u������
	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);

#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel) const;
#endif

private:
	// �X�V�ʒm�̎󂯎��i���g�̏��L�c���[�ȉ��̂݁j
	bool receiveNotifyUpdatedChildren(const UnivType *pUnivType, enum UpdateKind kind);
	void updateChildren();

	friend RequireUpdateObj;
	// �I�u�W�F�N�g���X�V���X�g�ɒǉ��E�폜
	void addToUpdateList(RequireUpdateObj *pObj){ m_updateList.push_back(pObj); }
	void removeFromUpdateList(RequireUpdateObj *pObj){ m_updateList.erase(pObj); }

};

#endif
