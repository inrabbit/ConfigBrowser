#ifndef DEF_REQUIRE_UPDATE_OBJ
#define DEF_REQUIRE_UPDATE_OBJ

#define UK_FLAG_VALUE       0x01	// �l�̍X�V
#define UK_FLAG_ARRAY_SIZE  0x02	// �z��̃T�C�Y�̍X�V

enum UpdateKind{
	UK_ONLY_VALUE       = UK_FLAG_VALUE,	// �l�݂̂̍X�V
	UK_ONLY_ARRAY_SIZE  = UK_FLAG_ARRAY_SIZE,	// �z��̃T�C�Y�݂̂̍X�V
	UK_ALL              = UK_FLAG_VALUE | UK_FLAG_ARRAY_SIZE	// ���҂𔺂�
};

class UnivType;
class BaseDB;

class RequireUpdateObj
{
private:
	bool m_updated;
	static BaseDB *m_pDB;

protected:
	// �I�u�W�F�N�g�̍X�V��Ԃ�ݒ�
	void invalidate(){ m_updated = false; }
	void validate(){ m_updated = true; }
	// �I�u�W�F�N�g�ɍX�V���K�v�����肷��
	bool requireUpdate(){ return !m_updated; }

public:
	RequireUpdateObj();
	~RequireUpdateObj();

	static void setDB(BaseDB *pDB){ m_pDB = pDB; }
	static BaseDB *getDB(){ return m_pDB; }

	virtual bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	virtual void update();

};

#endif
